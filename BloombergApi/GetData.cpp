#include "stdafx.h"

#define BB_USING_DLL
#include <bbapi.h>
#include <bbunix.h>

#include "bbutils.h"

#include "BloombergApi.h"

namespace Bloomberg
{
	// Request the data
	void CBloombergApi::CBBDataRequest::request(
		const set_secdesc_t& secs,
		const set_fieldid_t& fields,
		const set_override_t&overrides,
		CClientCallback* callback)
	{
		if (secs.empty())
			throw std::runtime_error("Unable to make a data request. This list of securities is empty");

		if (fields.empty())
			throw std::runtime_error("Unable to make a data request. This list of fields is empty");

		if (overrides.size() > BB_MAX_OVRRDS)
			throw std::runtime_error("Unable to make a data request. Too many overrides");

		// No more than fifty fields can be requested.
		// Split the fields into pages to handle this.
		size_t field_page_count = (fields.size() / BB_MAX_FIELDS) + (fields.size() % BB_MAX_FIELDS > 0 ? 1 : 0);
		set_fieldid_t::const_iterator i_fields(fields.begin());
		while (i_fields != fields.end())
		{
			// Make the page of fields
			vec_fieldid_t page_fields;
			while (page_fields.size() < BB_MAX_FIELDS && i_fields != fields.end())
			{
				page_fields.push_back(*i_fields);
				++i_fields;
			}

			// No more than ten security can be requested.
			// Split the requets into pages to handle this.
			set_secdesc_t::const_iterator i_sec(secs.begin());
			while (i_sec != secs.end())
			{
				// Make the page of securities
				vec_secdesc_t page_secs;
				while (page_secs.size() < BB_MAX_SECS && i_sec != secs.end())
				{
					page_secs.push_back(*i_sec);
					++i_sec;
				}

				// Push the page of securities and fields onto the request queue.
				m_RequestQueue.push_back( group_request_t(m_GroupRequestId, request_t(page_secs, page_fields, overrides) ) );

				// Remember the number of requests made.
				m_GroupRequestCountMap[m_GroupRequestId] += page_secs.size();
			}
		}

		// Register the callback against the group id.
		m_CallbackMap[m_GroupRequestId] = callback;

		// Request the data in a controlled fasion.
		processRequestQueue();

		// Get the next id
		++m_GroupRequestId;
	}

	void
	CBloombergApi::CBBDataRequest::processRequestQueue()
	{
		// While there are outstanding requests and the number of pending requests is below a minimum
		while (!m_RequestQueue.empty() && m_PagedRequestMap.size() < m_QueueLength)
		{
			// Fetch the request from the front of the queue
			const group_request_t& request = m_RequestQueue.front();
			group_request_id_t group_request_id = request.first;
			const vec_secdesc_t& secs = request.second.first;
			const vec_fieldid_t& fields = request.second.second;
			const set_override_t& overrides = request.second.third;

			// Setup the overrides
			const size_t OVERRIDE_VALUE_LEN = 32;
			fieldid_t raw_override_fields[BB_MAX_OVRRDS];
			char raw_override_values[BB_MAX_OVRRDS * OVERRIDE_VALUE_LEN];
			memset((void*) raw_override_fields, 0, sizeof(raw_override_fields));
			memset((void*) raw_override_values, 0, sizeof(raw_override_values));
			set_override_t::const_iterator overrides_i(overrides.begin());
			size_t overrides_j = 0;
			while (overrides_i != overrides.end())
			{
				raw_override_fields[overrides_j] = overrides_i->first;
				strncpy(raw_override_values + overrides_j * OVERRIDE_VALUE_LEN, overrides_i->second.c_str(), OVERRIDE_VALUE_LEN);
				++overrides_i;
				++overrides_j;
			}

			// Setup the fields
			size_t nfields = fields.size();
			fieldid_t raw_fields[BB_MAX_FIELDS];
			for (size_t i = 0; i < nfields; ++i)
				raw_fields[i] = fields[i];

			// Setup the securities
			char raw_secs[BB_MAX_SECS * BB_MAX_SEC_DESCRIPTION];
			fieldid_t raw_types[BB_MAX_SECS];
			memset((void*) raw_secs, 0, sizeof(raw_secs));
			memset((void*) raw_types, 0, sizeof(raw_types));
			for (size_t i = 0; i < secs.size(); ++i)
			{
				strcpy(raw_secs + i * BB_MAX_SEC_DESCRIPTION, secs[i].first.c_str());
				raw_types[i] = secs[i].second;
			}

			// Request
			request_id_t request_id = bb_getdatax(
				m_Connection,
				static_cast<long>(secs.size()),
				raw_types,
				raw_secs,
				static_cast<long>(fields.size()),
				raw_fields,
				static_cast<long>(overrides.size()),
				raw_override_fields,
				raw_override_values);

			if (request_id > 0)
			{
				// Remember what we asked for so we can unpack the data when it arrives.
				m_PagedRequestMap[request_id] = page_request_t(secs, fields);

				// Remember the handle we got back so we can reassemble the data to match the original request.
				m_GroupRequestIdReverseMap[request_id] = group_request_id;
			}

			// Remove the request from the queue.
			m_RequestQueue.pop_front();
		}
	}

	void CBloombergApi::CBBDataRequest::handleMessage(const bb_msg_fieldsx_t* bb_msg_fieldsx)
	{
		request_id_t request_id = bb_msg_fieldsx->comm_header.request_id;

		// Find the request
		page_request_map_t::iterator request_map_iter(m_PagedRequestMap.find(request_id));
		if (request_map_iter == m_PagedRequestMap.end())
			return;	// We got something we didn't ask for.

		// Break out the request into variables to make th code more readable.
		const page_request_t& request = request_map_iter->second;
		const vec_secdesc_t& request_sec_list = request.first;
		const vec_fieldid_t& request_field_list = request.second;

		// Find the group request id given the bloomberg request id.
		group_request_id_t group_request_id = m_GroupRequestIdReverseMap[request_id];
		
		// We will only receive this request once so we can remove it from the reverse lookup map.
		m_GroupRequestIdReverseMap.erase(request_id);
		
		// Fetch the data map associated with this group.
		sec_retval_map_t& request_data_map = m_GroupResponseMap[group_request_id];

		// We should have got back the number of securities we requested.
		if (bb_msg_fieldsx->comm_header.num_items == request_sec_list.size())
		{
			// Go through the data packing it in the group data map.
			for (size_t i_security = 0; i_security < request_sec_list.size(); ++i_security)
			{
				// Get the security
				const secdesc_t& sec = request_sec_list[i_security];

				// Fetch the data map associated with this security
				retval_map_t& sec_data_map = request_data_map[sec];

				// Process the field data
				const char* field_data = bb_msg_fieldsx->field_ptr[i_security];

				if (strncmp("SECURITY UNKNOWN", field_data, 16) == 0)
				{
					// Create dummy entries for unknown securities.
					for (size_t i_field = 0; i_field < request_field_list.size(); ++i_field)
					{
						fieldid_t fieldid = request_field_list[i_field];
						sec_data_map[fieldid] = "";
					}
				}
				else
				{
					// Creates entries for valid securities.
					for (size_t i_field = 0; i_field < request_field_list.size(); ++i_field)
					{
						fieldid_t fieldid = request_field_list[i_field];
						sec_data_map[fieldid] = field_data;

						field_data += strlen(field_data) + 1;
					}
				}

				// We've dealt with this request.
				--m_GroupRequestCountMap[group_request_id];
			}
		}
		else
		{
			// Need some kind of error callback.
		}

		// Remove the handled request from the request map.
		m_PagedRequestMap.erase(request_map_iter);

		// Are we there yet?
		bool ready_to_call = false;
		if (m_GroupRequestCountMap[group_request_id] == 0)
		{
			m_GroupRequestCountMap.erase(group_request_id);
			ready_to_call = true;
		}

		if (ready_to_call)
			handleReceived(group_request_id);

		processRequestQueue();
	}

	void CBloombergApi::CBBDataRequest::handleReceived(group_request_id_t group_request_id)
	{
		// Find the data and callback for this group request.
		sec_retval_map_t responses;
		CClientCallback* callback;
		try
		{
			response_map_t::iterator i_group_response(m_GroupResponseMap.find(group_request_id));
			if (i_group_response == m_GroupResponseMap.end())
				throw "unable to find response";
			responses = i_group_response->second;

			callback_map_t::iterator i_callback(m_CallbackMap.find(group_request_id));
			if (i_callback == m_CallbackMap.end())
				throw "unable to find callback";

			callback = i_callback->second;
		}
		catch (const char*)
		{
			return;
		}

		// Execute the callbacks
		callback->handleData(responses);

		// Remove the data and the callback
		m_GroupResponseMap.erase(group_request_id);
		m_CallbackMap.erase(group_request_id);
	}

}