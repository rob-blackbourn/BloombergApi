#include "stdafx.h"

#define BB_USING_DLL
#include <bbapi.h>
#include <bbunix.h>

#include "bbutils.h"

#include "BloombergApi.h"

namespace Bloomberg
{
	const size_t BB_MAX_SECS_HIST = 8;
	const size_t BB_MAX_FIELDS_HIST = 5;

	// Request the data
	void CBloombergApi::CBBHistDataRequest::request(
		const set_secdesc_t& secs,
		const set_fieldid_t& fields,
		const datespec_t& datespec,
		CClientCallback* callback)
	{
		// No more than five fields can be specified in one request.
		// Split the fields into pages to handle this.
		size_t field_page_count = (fields.size() / BB_MAX_FIELDS_HIST) + (fields.size() % BB_MAX_FIELDS_HIST > 0 ? 1 : 0);
		set_fieldid_t::const_iterator i_fields(fields.begin());
		while (i_fields != fields.end())
		{
			// Make the page of fields
			vec_fieldid_t page_fields;
			while (page_fields.size() < BB_MAX_FIELDS_HIST && i_fields != fields.end())
			{
				page_fields.push_back(*i_fields);
				++i_fields;
			}

			// No more than eight securities can be specified in one request.
			// Split the requests into pages to handle this.
			set_secdesc_t::const_iterator i_sec(secs.begin());
			while (i_sec != secs.end())
			{
				// Make the page of securities
				vec_secdesc_t page_secs;
				while (page_secs.size() < BB_MAX_SECS_HIST && i_sec != secs.end())
				{
					page_secs.push_back(*i_sec);
					++i_sec;
				}

				// Push the request page on the queue.
				m_RequestQueue.push_back( group_request_t(m_GroupRequestId, request_t(page_secs, page_fields, datespec) ) );

				// Keep count of the number of securities per field page we have requested in this group.
				m_GroupRequestCountMap[m_GroupRequestId] += page_secs.size();
			}
		}

		// Register the callback against the internal request id.
		m_CallbackMap[m_GroupRequestId] = callback;

		// Request the data in a controlled manner.
		processRequestQueue();

		// Get the next id
		++m_GroupRequestId;
	}

	void
	CBloombergApi::CBBHistDataRequest::processRequestQueue()
	{
		// While there are outstanding requests and the number of pending requests is below a minimum
		while (!m_RequestQueue.empty() && m_PagedRequestMap.size() < m_QueueLength)
		{
			// Fetch the request from the front of the queue
			const group_request_t& request = m_RequestQueue.front();
			group_request_id_t group_request_id = request.first;
			const vec_secdesc_t& secs = request.second.first;
			const vec_fieldid_t& fields = request.second.second;
			const datespec_t& datespec = request.second.third;

			// Setup the fields
			size_t nfields = fields.size();
			fieldid_t raw_fields[BB_MAX_FIELDS_HIST];
			for (size_t i = 0; i < nfields; ++i)
				raw_fields[i] = fields[i];

			// Setup the securities
			char raw_secs[BB_MAX_SECS_HIST * BB_MAX_SEC_DESCRIPTION];
			fieldid_t raw_types[BB_MAX_SECS_HIST];
			memset((void*) raw_secs, 0, sizeof(raw_secs));
			memset((void*) raw_types, 0, sizeof(raw_types));
			for (size_t i = 0; i < secs.size(); ++i)
			{
				strcpy(raw_secs + i * BB_MAX_SEC_DESCRIPTION, secs[i].first.c_str());
				raw_types[i] = secs[i].second;
			}

			int4 raw_field_count = static_cast<long>(fields.size());
			int4 raw_sec_count = static_cast<long>(secs.size());

			// Request
			request_id_t request_id = bb_mgethistoryx(
				m_Connection,
				raw_sec_count,
				raw_secs,
				raw_types,
				datespec.first,
				datespec.second,
				raw_field_count,
				raw_fields,
				datespec.third);

			if (request_id > 0)
			{
				// Remember what we aked for so we can unpack it when the reponse arrives
				m_PagedRequestMap[request_id] = page_request_t(secs, fields);

				// Remember the handle we got back so we can reassemble the data to match the original request.
				m_GroupRequestIdReverseMap[request_id] = group_request_id;
			}
			else
			{
				// Need an error callback
			}

			// Remove the request from the queue.
			m_RequestQueue.pop_front();
		}
	}

	void CBloombergApi::CBBHistDataRequest::handleMessage(const bb_msg_mhistory_t* bb_msg_mhistory)
	{
		request_id_t request_id = bb_msg_mhistory->comm_header.request_id;

		// Find the request
		page_request_map_t::iterator request_map_iter(m_PagedRequestMap.find(request_id));
		if (request_map_iter == m_PagedRequestMap.end())
			return;	// We got something we didn't ask for

		page_request_t request = request_map_iter->second;
		vec_secdesc_t request_sec_list = request.first;
		vec_fieldid_t request_field_list = request.second;

		// Find the group request id given the bloomberg request id.
		group_request_id_t group_request_id = m_GroupRequestIdReverseMap[request_id];
		
		// We will only receive this request once so we can remove it from the reverse lookup map.
		m_GroupRequestIdReverseMap.erase(request_id);

		// Find the data map associated with this group
		sec_retval_map_t& request_data_map = m_GroupResponseMap[group_request_id];

		// Did we get any data?
		// We should have got back the number of securities we requested
		// We should have got the number of fields we requested
		if (bb_msg_mhistory->comm_header.num_items > 0 && bb_msg_mhistory->num_of_securities == request_sec_list.size() && bb_msg_mhistory->num_of_fields == request_field_list.size())
		{
			/* get pointer to start of Security Error Codes */
			const long* sec_err = &(bb_msg_mhistory->mhistory_data[0]);
			/* pointer to start of number of points per field */
			const long* num_points_array = &(bb_msg_mhistory->mhistory_data[bb_msg_mhistory->num_of_securities]);

			/* ptr to a ptr to the start of the security data */
			bb_decode_mhistory_t** raw_data_array = (bb_decode_mhistory_t**) &num_points_array[bb_msg_mhistory->num_of_securities * bb_msg_mhistory->num_of_fields];

			for (long i_security = 0; i_security < bb_msg_mhistory->num_of_securities; i_security++)
			{
				// Get the security
				const secdesc_t& sec = request_sec_list[i_security];

				// Find the data map associated with this security
				std::map< fieldid_t, vec_retval_t>& sec_data_map = request_data_map[sec];

				if (sec_err[i_security] != 0)
				{
					// Error for this security. Skip to next.
					continue;
				}

				bb_decode_mhistory_t* data = raw_data_array[i_security];

				if (data == NULL)
				{
					// No data for this security. Skip to next.
					continue;
				}

				for (long i_field = 0; i_field < bb_msg_mhistory->num_of_fields;  i_field++)
				{
					fieldid_t fieldid = request_field_list[i_field];

					long num_points = num_points_array[i_security * bb_msg_mhistory->num_of_fields + i_field]; 

					for (int i = 0; i < num_points; i++)
					{ 
						longdate_t date = data->mhist_data[i].date;
						double value = data->mhist_data[i].value;

						vec_retval_t& vec_retval = sec_data_map[fieldid];
						vec_retval.push_back(retval_t(date, value));
					}

					data = data + num_points;
				}
			}

			m_GroupRequestCountMap[group_request_id] -= bb_msg_mhistory->num_of_securities;
		}

		// Remove the request from the request map.
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

		// Now we have handled a message we can request some more data.
		processRequestQueue();
	}

	void CBloombergApi::CBBHistDataRequest::handleReceived(group_request_id_t group_request_id)
	{
		// Find the data and callback for this group request.
		sec_retval_map_t responses;
		CClientCallback* callback;

		try
		{
			group_response_map_t::iterator i_group_response(m_GroupResponseMap.find(group_request_id));
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
