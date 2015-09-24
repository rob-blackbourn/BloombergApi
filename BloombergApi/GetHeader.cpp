#include "stdafx.h"

#define BB_USING_DLL
#include <bbapi.h>
#include <bbunix.h>

#include "bbutils.h"

#include "BloombergApi.h"

namespace Bloomberg
{

	void CBloombergApi::CBBHeaderRequest::request(const set_secdesc_t& secs, CClientCallback* callback)
	{
		// The maxiumum number of securities that can be requested in one go is ten.
		// Split the request up into pages of ten or less.
		set_secdesc_t::const_iterator i_sec(secs.begin());
		while (i_sec != secs.end())
		{
			// Make the page of securities.
			vec_secdesc_t page_secs;
			while (page_secs.size() < BB_MAX_SECS && i_sec != secs.end())
			{
				page_secs.push_back(*i_sec);
				++i_sec;
			}

			// Push this request page onto the queue.
			m_RequestQueue.push_back( group_request_t(m_GroupRequestId, page_secs) );

			// Keep count of the number of securities in the group.
			m_GroupRequestCountMap[m_GroupRequestId] += page_secs.size();
		}

		// Register the callback against the internal group request id.
		m_CallbackMap[m_GroupRequestId] = callback;

		// Request the data.
		processRequestQueue();

		// Get the next id
		++m_GroupRequestId;
	}

	void
	CBloombergApi::CBBHeaderRequest::processRequestQueue()
	{
		// If the request queue is empty there is nothing to do
		// If the reqeusted queue is at maximum wait for it to drain before adding more.
		while (!m_RequestQueue.empty() && m_PagedRequestMap.size() < m_QueueLength)
		{
			// Fetch the request from the front of the queue
			const group_request_t& request = m_RequestQueue.front();
			group_request_id_t group_request_id = request.first;
			const vec_secdesc_t& secs = request.second;

			// Pack subject names into appropriate bloomberg array.
			char raw_secs[BB_MAX_SEC_DESCRIPTION * BB_MAX_SECS];
			fieldid_t raw_types[BB_MAX_SECS];
			memset((void*) raw_secs, 0, sizeof(raw_secs));
			memset((void*) raw_types, 0, sizeof(raw_types));
			for (size_t i = 0; i < secs.size(); ++i)
			{
				strcpy(raw_secs + i * BB_MAX_SEC_DESCRIPTION, secs[i].first.c_str());
				raw_types[i] = secs[i].second;
			}

			// Request the data.
			request_id_t request_id = bb_getheader_typedx(
				m_Connection,
				BHeaderENHANCEDSEARCH | EXTENDED_HEADER,
				0,
				static_cast<long>(secs.size()),
				raw_types,
				raw_secs);

			if (request_id > 0)
			{
				// Remember what we asked for so we can unpack the data when it arrives.
				m_PagedRequestMap[request_id] = secs;

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

	void CBloombergApi::CBBHeaderRequest::handleMessage(const bb_msg_headerx_t* msg_headerx)
	{
		request_id_t request_id = msg_headerx->comm_header.request_id;

		// Find the request
		page_request_map_t::iterator request_map_iter(m_PagedRequestMap.find(request_id));
		if (request_map_iter == m_PagedRequestMap.end())
			return;	// We got something we didn't ask for

		const vec_secdesc_t& request_sec_list = request_map_iter->second;

		// Find the group request id given the bloomberg request id.
		group_request_id_t group_request_id = m_GroupRequestIdReverseMap[request_id];
		
		// We will only receive this request once so we can remove it from the reverse lookup map.
		m_GroupRequestIdReverseMap.erase(request_id);
		
		// Find the data map associated with this group
		sec_retval_map_t& request_data_map = m_GroupResponseMap[group_request_id];

		// We should have got back the number of securities we requested
		if (msg_headerx->comm_header.num_items != request_sec_list.size())
		{
			// Something went wrong
			for (size_t i_security = 0; i_security < request_sec_list.size(); ++i_security)
			{
				// Get the security
				const secdesc_t& sec = request_sec_list[i_security];

				// Find the data map associated with this security
				retval_map_t& sec_data_map = request_data_map[sec];

				// We don't know what the error was
				sec_data_map["EXT_STATUS_MSG"] = "unknown error";
			}
		}
		else
		{
			// Unpack and store the header data for each security.
			for (size_t i_security = 0; i_security < request_sec_list.size(); ++i_security)
			{
				// Get the security
				const secdesc_t& sec = request_sec_list[i_security];

				// Find the data map associated with this security
				retval_map_t& sec_data_map = request_data_map[sec];

				// Process the header data
				const bb_decode_headerx_t *decode_headerx = &msg_headerx->sec_header[i_security];

				char buf[33], buf_itoa[34];
				memcpy(buf, &decode_headerx->sec_key, 32);
				buf[32] = 0;

				// There are no field id's for the header so we use strings.
				sec_data_map["EXT_SEC_KEY"] = buf;
				sec_data_map["EXT_STATUS_CODE"] = _itoa(decode_headerx->status, buf_itoa, 10);

				if (decode_headerx->status == 0)
				{
					sec_data_map["EXT_STATUS_MSG"] = "OK";

					std::string NA = "N.A.";

					sec_data_map["PRICE_OPEN"]		= (decode_headerx->price_open		== BB_VAL_MISSING ? NA : bb_todouble(decode_headerx->price_open));
					sec_data_map["PRICE_HIGH"]		= (decode_headerx->price_high		== BB_VAL_MISSING ? NA : bb_todouble(decode_headerx->price_high));
					sec_data_map["PRICE_LOW"]		= (decode_headerx->price_low		== BB_VAL_MISSING ? NA : bb_todouble(decode_headerx->price_low));
					sec_data_map["PRICE_LAST"]		= (decode_headerx->price_last		== BB_VAL_MISSING ? NA : bb_todouble(decode_headerx->price_last));
					sec_data_map["PRICE_SETTLE"]	= (decode_headerx->price_settle		== BB_VAL_MISSING ? NA : bb_todouble(decode_headerx->price_settle));
					sec_data_map["PRICE_BID"]		= (decode_headerx->price_bid		== BB_VAL_MISSING ? NA : bb_todouble(decode_headerx->price_bid));
					sec_data_map["PRICE_ASK"]		= (decode_headerx->price_ask		== BB_VAL_MISSING ? NA : bb_todouble(decode_headerx->price_ask));
					sec_data_map["YIELD_BID"]		= (decode_headerx->yield_bid		== BB_VAL_MISSING ? NA : bb_todouble(decode_headerx->yield_bid));
					sec_data_map["YIELD_ASK"]		= (decode_headerx->yield_ask		== BB_VAL_MISSING ? NA : bb_todouble(decode_headerx->yield_ask));
					sec_data_map["LIMIT_UP"]		= (decode_headerx->limit_up			== BB_VAL_MISSING ? NA : bb_todouble(decode_headerx->limit_up));
					sec_data_map["LIMIT_DOWN"]		= (decode_headerx->limit_down		== BB_VAL_MISSING ? NA : bb_todouble(decode_headerx->limit_down));
					sec_data_map["OPEN_INTEREST"]	= (decode_headerx->open_interest	== BB_VAL_MISSING ? NA : bb_todouble(decode_headerx->open_interest));
					sec_data_map["YEST_PRICE_LAST"]	= (decode_headerx->yest_price_last	== BB_VAL_MISSING ? NA : bb_todouble(decode_headerx->yest_price_last));
					sec_data_map["SCALE"]			= (decode_headerx->scale			== BB_VAL_MISSING ? NA : bb_todouble(decode_headerx->scale));
					sec_data_map["TIME_LAST"]		= (decode_headerx->time_last		== BB_VAL_MISSING ? NA : Bloomberg::bb_time_t(decode_headerx->time_last));
					sec_data_map["EXCHANGE_LAST"]	= (decode_headerx->exchange_last	== BB_VAL_MISSING ? NA : bb_toint(decode_headerx->exchange_last));
					sec_data_map["TICK_DIRECTION"]	= (decode_headerx->tick_direction	== BB_VAL_MISSING ? NA : bb_toint(decode_headerx->tick_direction));
					sec_data_map["SIZE_BID"]		= (decode_headerx->size_bid			== BB_VAL_MISSING ? NA : bb_toint(decode_headerx->size_bid));
					sec_data_map["SIZE_ASK"]		= (decode_headerx->size_ask			== BB_VAL_MISSING ? NA : bb_toint(decode_headerx->size_ask));
					sec_data_map["CONDITION_BID"]	= (decode_headerx->condition_bid	== BB_VAL_MISSING ? NA : bb_toint(decode_headerx->condition_bid));
					sec_data_map["CONDITION_ASK"]	= (decode_headerx->condition_ask	== BB_VAL_MISSING ? NA : bb_toint(decode_headerx->condition_ask));
					sec_data_map["CONDITION_LAST"]	= (decode_headerx->condition_last	== BB_VAL_MISSING ? NA : bb_toint(decode_headerx->condition_last));
					sec_data_map["CONDITION_MARKET"]= (decode_headerx->condition_market	== BB_VAL_MISSING ? NA : bb_toint(decode_headerx->condition_market));
					sec_data_map["MONITORABLE"]		= (decode_headerx->monitorable		== BB_VAL_MISSING ? NA : (decode_headerx->monitorable == 1 ? "Y" : "N"));
					sec_data_map["VOLUME_TOTAL"]	= (decode_headerx->volume_total		== BB_VAL_MISSING ? NA : bb_toint(decode_headerx->volume_total));
					sec_data_map["TICKS_TOTAL"]		= (decode_headerx->ticks_total		== BB_VAL_MISSING ? NA : bb_toint(decode_headerx->ticks_total));
					sec_data_map["TIME_START"]		= (decode_headerx->time_start		== BB_VAL_MISSING ? NA : Bloomberg::bb_time_t(decode_headerx->time_start));
					sec_data_map["TIME_END"]		= (decode_headerx->time_end			== BB_VAL_MISSING ? NA : Bloomberg::bb_time_t(decode_headerx->time_end));
					sec_data_map["RT_PCS"]			= (decode_headerx->rt_pcs			== BB_VAL_MISSING ? NA : bb_topcs(decode_headerx->rt_pcs));
					sec_data_map["FORMAT"]			= (decode_headerx->format			== BB_VAL_MISSING ? NA : bb_toint(decode_headerx->format));
					sec_data_map["DATE"]			= (decode_headerx->date				== BB_VAL_MISSING ? NA : Bloomberg::bb_date_t(decode_headerx->date));

					if (decode_headerx->extra && decode_headerx->extra_size >= sizeof(bb_decode_xheaderx_v5_t))
					{
						sec_data_map["YIELD_LAST"]			= (decode_headerx->extra->yield_last			== BB_VAL_MISSING ? NA : bb_todouble(decode_headerx->extra->yield_last));
						sec_data_map["YIELD_LAST_TDY"]		= (decode_headerx->extra->yield_last_tdy		== BB_VAL_MISSING ? NA : bb_todouble(decode_headerx->extra->yield_last_tdy));
						sec_data_map["YIELD_OPEN"]			= (decode_headerx->extra->yield_open			== BB_VAL_MISSING ? NA : bb_todouble(decode_headerx->extra->yield_open));
						sec_data_map["YIELD_OPEN_TDY"]		= (decode_headerx->extra->yield_open_tdy		== BB_VAL_MISSING ? NA : bb_todouble(decode_headerx->extra->yield_open_tdy));
						sec_data_map["YIELD_HIGH"]			= (decode_headerx->extra->yield_high			== BB_VAL_MISSING ? NA : bb_todouble(decode_headerx->extra->yield_high));
						sec_data_map["YIELD_HIGH_TDY"]		= (decode_headerx->extra->yield_high_tdy		== BB_VAL_MISSING ? NA : bb_todouble(decode_headerx->extra->yield_high_tdy));
						sec_data_map["YIELD_LOW"]			= (decode_headerx->extra->yield_low				== BB_VAL_MISSING ? NA : bb_todouble(decode_headerx->extra->yield_low));
						sec_data_map["YIELD_LOW_TDY"]		= (decode_headerx->extra->yield_low_tdy			== BB_VAL_MISSING ? NA : bb_todouble(decode_headerx->extra->yield_low_tdy));
						sec_data_map["YEST_YIELD_LAST"]		= (decode_headerx->extra->yest_yield_last		== BB_VAL_MISSING ? NA : bb_todouble(decode_headerx->extra->yest_yield_last));
						sec_data_map["PRICE_AT_TRADE"]		= (decode_headerx->extra->price_AT_trade		== BB_VAL_MISSING ? NA : bb_todouble(decode_headerx->extra->price_AT_trade));
						sec_data_map["PRICE_AT_TRADE_TDY"]	= (decode_headerx->extra->price_AT_trade_tdy	== BB_VAL_MISSING ? NA : bb_todouble(decode_headerx->extra->price_AT_trade_tdy));
						sec_data_map["PRICE_MID"]			= (decode_headerx->extra->price_mid				== BB_VAL_MISSING ? NA : bb_todouble(decode_headerx->extra->price_mid));
						sec_data_map["PRICE_MID_TDY"]		= (decode_headerx->extra->price_mid_tdy			== BB_VAL_MISSING ? NA : bb_todouble(decode_headerx->extra->price_mid_tdy));

						if (decode_headerx->extra_size >= sizeof(bb_decode_xheaderx_v7_t))
						{
							sec_data_map["VWAP"]	= (decode_headerx->extra->vwap == BB_VAL_MISSING ? NA : bb_todouble(decode_headerx->extra->vwap));
						}
					}
				}
				else
				{
					// Try to find a good error message to return
					const char* msg = "unknown error";
					static struct {int status; const char* msg;} msgs[] = {{-1, "can't identify security"}, {-2, "can't get header"}, {-3, "invalid security"}, {-4, "security not in database"}, {-5, "security exists, has not traded in 30 days"}, {-6, "no realtime available for this security"}};
					for (size_t i = 0; i < (sizeof(msgs) / sizeof(msgs[0])); ++i)
					{
						if (decode_headerx->status == msgs[i].status)
						{
							msg = msgs[i].msg;
							break;
						}
					}
					sec_data_map["EXT_STATUS_MSG"] = msg;
				}

				// We've dealt with this request.
				--m_GroupRequestCountMap[group_request_id];
			}
		}

		// Remove this request from the request map.
		m_PagedRequestMap.erase(request_map_iter);

		// Are we there yet?
		bool ready_to_call = false;
		if (m_GroupRequestCountMap[group_request_id] <= 0)
		{
			m_GroupRequestCountMap.erase(group_request_id);
			ready_to_call = true;
		}

		if (ready_to_call)
			handleReceived(group_request_id);

		// Now we've handled a message we can process more requests.
		processRequestQueue();
	}

	void CBloombergApi::CBBHeaderRequest::handleReceived(group_request_id_t group_request_id)
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