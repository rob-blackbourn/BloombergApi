#include "stdafx.h"

#define BB_USING_DLL
#include <bbapi.h>
#include <bbunix.h>

#include "bbutils.h"

#include "BloombergApi.h"

namespace Bloomberg
{
	static const bb_decode_tick_data_t *getNextDecodeTickData(const bb_decode_tick_data_t *pDecodeTickData)
	{
		if (pDecodeTickData == 0)
			return 0;

		return reinterpret_cast<const bb_decode_tick_data_t *>(pDecodeTickData->data + pDecodeTickData->size);
	}
	
	static const bb_decode_tick_extra_t *getNextTickExtra(const bb_decode_tick_extra_t *pTickExtra)
	{
		/*
		**  To get to the next bb_decode_tick_extra_t, go past all the bb_decode_tick_data_t's
		**  that this pTickExtra contains.
		*/

		if (pTickExtra == 0 || pTickExtra->count < 0 || pTickExtra->count > 0xFF)
			return 0;

		const bb_decode_tick_data_t *pDecodeTickData = pTickExtra->data;

		if (pTickExtra->count == 0)
		{
			/*
			**	A pTickExtra that a count of 0 bb_decode_tick_data_t, contains
			**	an empty bb_decode_tick_data_t.
			*/
			pDecodeTickData = getNextDecodeTickData(pDecodeTickData);
		}
		else
		{
			/*
			**	Go past all the bb_decode_tick_data_t's that this pTickExtra contains.
			*/
			for(int i = 0; i < pTickExtra->count; ++i)
				pDecodeTickData = getNextDecodeTickData(pDecodeTickData);
		}

		const bb_decode_tick_extra_t *data = reinterpret_cast<const bb_decode_tick_extra_t *>(pDecodeTickData);
		if (data == 0 || data->count > 0xFF)
			return 0;
		else
			return data;
	}

	static const bb_decode_tick_extra_t *getTickExtra(const bb_msg_tickx_t *pMsgTickx, int4 tickIndex)
	{
		if (pMsgTickx == 0 || tickIndex < 0 || tickIndex >= pMsgTickx->comm_header.num_items)
			return 0;

		const bb_decode_tick_extra_t *pTickExtra = (const bb_decode_tick_extra_t *) (pMsgTickx->tick_data + pMsgTickx->comm_header.num_items);

		for(int4 i = 0; i < tickIndex; ++i)
			pTickExtra = getNextTickExtra(pTickExtra);

		return pTickExtra;
	}

	static std::string operator << (std::string& lhs, const bb_decode_tick_data_t& rhs)
	{
		switch(rhs.data_type)
		{
		case bTypeSTRING:
			lhs = (char*) rhs.data;
			//printf("SMART FIELD %ld(0x%04x): STRING : %s\n", pDecodeTickData->field_id, pDecodeTickData->field_id, pDecodeTickData->data);
			break;

		case bTypeNUMERIC:
			lhs = bb_toint(*((long *)rhs.data));
			//printf("SMART FIELD %ld(0x%04x): NUMERIC: %ld\n", pDecodeTickData->field_id, pDecodeTickData->field_id, *((long *)pDecodeTickData->data));
			break;

		case bTypePRICE:
			lhs = bb_todouble(*((double *)rhs.data));
			//printf("SMART FIELD %ld(0x%04x): PRICE  : %f\n", pDecodeTickData->field_id, pDecodeTickData->field_id, *((double *)pDecodeTickData->data));
			break;

		case bTypeSECURITY:
		case bTypeDATE:
		case bTypeTIME:
		case bTypeDATETIME:
		case bTypeBULK:
		case bTypeMONTHYEAR:
		case bTypeBOOLEAN:
		case bTypeISOCODE:
		default:
			throw;
			//printf("SMART::UNKNOWN TYPE %ld: ", pDecodeTickData->data_type);
			//hexDump(pDecodeTickData->data, pDecodeTickData->size);
		}

		return lhs;
	}

	void CBloombergApi::CBBTickMonitor::request(const set_secdesc_t& secs, const set_moncat_t& cats, CClientCallback* callback)
	{
		if (secs.empty())
			throw std::runtime_error("Unable to make a tick monitor request. This list of securities is empty");

		if (cats.size() > bCategoryNUM_TICKMNTR_CATEGORIES)
			throw std::runtime_error("Unable to make a tick monitor request. The number of categories is greater than the maximum number of categories available");

		// Make a new set of secs not yet monitored
		set_secdesc_t unmonitored_secs;
		for (set_secdesc_t::const_iterator i_sec(secs.begin()); i_sec != secs.end(); ++i_sec)
		{
			tickmntr_rev_map_t::const_iterator i_monids(m_TickMonitorReverseIdMap.find(*i_sec));
			if (i_monids == m_TickMonitorReverseIdMap.end())
				unmonitored_secs.insert(*i_sec);
		}

		if (!unmonitored_secs.empty())
		{
			// Split the request up into pages.
			set_secdesc_t::const_iterator i_unmonitored_sec(unmonitored_secs.begin());
			while (i_unmonitored_sec != unmonitored_secs.end())
			{
				vec_secdesc_t page_secs;
				while (page_secs.size() < BB_MAX_SECS && i_unmonitored_sec != unmonitored_secs.end())
				{
					page_secs.push_back(*i_unmonitored_sec);
					++i_unmonitored_sec;
				}

				m_TickMonitorQueue.push_back( group_request_tickmntr_t(m_GroupRequestId, request_tickmntr_t(page_secs, cats) ) );
				m_GroupRequestCountMap[m_GroupRequestId] += page_secs.size();
			}

			// Make the request.
			processTickMonitorQueue();
		}

		// Add the callback if it does not yet exist and if there is anything to do.
		if (m_CallbackMap.find(callback) == m_CallbackMap.end() && !unmonitored_secs.empty())
			m_CallbackMap[callback].insert(secs.begin(), secs.end());

		// Get the next id
		++m_GroupRequestId;
	}

	void CBloombergApi::CBBTickMonitor::processTickMonitorQueue()
	{
		while (!m_TickMonitorQueue.empty() && m_TickMonitorRequestMap.size() < m_QueueLength)
		{
			const group_request_tickmntr_t& request = m_TickMonitorQueue.front();
			group_request_id_t group_request_id = request.first;
			const request_tickmntr_t& group_request = request.second;
			const vec_secdesc_t& secs = group_request.first;
			const set_moncat_t& cats = group_request.second;

			// Pack subject names into appropriate bloomberg array
			char raw_secs[32 * BB_MAX_SECS];
			fieldid_t raw_types[BB_MAX_SECS];
			memset((void*) raw_secs, 0, sizeof(raw_secs));
			memset((void*) raw_types, 0, sizeof(raw_types));
			for (size_t i = 0; i < secs.size(); ++i)
			{
				strcpy(raw_secs + i * 32, secs[i].first.c_str());
				raw_types[i] = secs[i].second;
			}

			moncat_t raw_cats[static_cast<size_t>(bCategoryNUM_TICKMNTR_CATEGORIES)];
			memset((void*) raw_cats, 0, sizeof(raw_cats));
			std::copy(cats.begin(), cats.end(), raw_cats);

			// Request
			//monid_t mon_id = bb_tickmntr_typed(m_Connection, static_cast<long>(secs.size()), raw_types, raw_secs);
			monid_t mon_id = bb_tickmntr_enhanced(
				m_Connection,
				static_cast<long>(secs.size()),
				raw_types,
				raw_secs,
				static_cast<long>(cats.size()),
				raw_cats,
				0);

			if (mon_id > 0)
			{
				m_TickMonitorRequestMap[mon_id] = secs;
			}
			else
			{
				// Need some kind of error callback
			}

			m_TickMonitorQueue.pop_front();
		}
	}

	void CBloombergApi::CBBTickMonitor::derequest(CClientCallback* callback)
	{
		// Find the callback
		callback_map_t::const_iterator i_target_callback(m_CallbackMap.find(callback));
		if (i_target_callback == m_CallbackMap.end())
			throw "unable to find callback";

		// Remove any security monitored by another callback
		set_secdesc_t target_secset = i_target_callback->second;
		for (callback_map_t::const_iterator i_callback(m_CallbackMap.begin()); i_callback!= m_CallbackMap.end(); ++i_callback)
		{
			if (i_callback == i_target_callback)
				continue;

			const set_secdesc_t& secset = i_target_callback->second;
			for (set_secdesc_t::const_iterator i_secset(secset.begin()); i_secset != secset.end(); ++i_secset)
			{
				const secdesc_t& sec = *i_secset;
				set_secdesc_t::iterator i_target_secset(target_secset.find(sec));
				if (i_target_secset != target_secset.end())
					target_secset.erase(i_target_secset);
			}
		}

		// Remove the callback
		m_CallbackMap.erase(callback);

		// Find the monids for the sec's to be removed
		set_monid_t monids;
		for (set_secdesc_t::const_iterator i_target_secset(target_secset.begin()); i_target_secset != target_secset.end(); ++i_target_secset)
		{
			const secdesc_t& sec = *i_target_secset;
			tickmntr_rev_map_t::iterator i_monid = m_TickMonitorReverseIdMap.find(sec);
			if (i_monid != m_TickMonitorReverseIdMap.end())
				monids.insert(i_monid->second);
		}

		// The maxiumum number of securities that can be requested in one go is ten.
		// Split the request up into pages of ten or less.
		set_monid_t::const_iterator i_monids(monids.begin());
		while (i_monids != monids.end())
		{
			vec_monid_t page_monids;
			while (page_monids.size() < BB_MAX_MONIDS && i_monids != monids.end())
			{
				page_monids.push_back(*i_monids);
				++i_monids;
			}

			m_StopMonitorQueue.push_back( group_request_stopmntr_t(m_GroupRequestId, page_monids) );
		}

		// Make the request.
		processStopMonitorQueue();

		// Get the next id
		++m_GroupRequestId;
	}

	void CBloombergApi::CBBTickMonitor::derequest(const set_secdesc_t& secs)
	{
		typedef std::set<CClientCallback*> callback_set_t;
		typedef std::set<monid_t> monid_set_t;

		// Remove these securities from the callbacks, and produce a list of callbacks which are now empty
		callback_set_t empty_callbacks;
		for (callback_map_t::iterator i_target_callback(m_CallbackMap.begin()); i_target_callback != m_CallbackMap.end(); ++i_target_callback)
		{
			CClientCallback* callback = i_target_callback->first;
			set_secdesc_t& callback_secs = i_target_callback->second;

			// For each target security remove from the list
			for (set_secdesc_t::const_iterator i_secs(secs.begin()); i_secs != secs.end(); ++i_secs)
			{
				set_secdesc_t::iterator i_callback_secs(callback_secs.find(*i_secs));
				if (i_callback_secs != callback_secs.end())
					callback_secs.erase(i_callback_secs);
			}

			if (callback_secs.empty())
				empty_callbacks.insert(callback);
		}

		// Remove the callbacks with no securities to monitor
		for (callback_set_t::const_iterator i_empty_callbacks(empty_callbacks.begin()); i_empty_callbacks != empty_callbacks.end(); ++i_empty_callbacks)
			m_CallbackMap.erase(*i_empty_callbacks);

		// Find the monids for the sec's to be removed
		monid_set_t monids;
		for (set_secdesc_t::const_iterator i_secs(secs.begin()); i_secs != secs.end(); ++i_secs)
		{
			const secdesc_t& sec = *i_secs;
			tickmntr_rev_map_t::iterator i_monid = m_TickMonitorReverseIdMap.find(sec);
			if (i_monid != m_TickMonitorReverseIdMap.end())
				monids.insert(i_monid->second);
		}

		// The maxiumum number of securities that can be requested in one go is ten.
		// Split the request up into pages of ten or less.
		set_monid_t::const_iterator i_monids(monids.begin());
		while (i_monids != monids.end())
		{
			vec_monid_t page_monids;
			while (page_monids.size() < BB_MAX_MONIDS && i_monids != monids.end())
			{
				page_monids.push_back(*i_monids);
				++i_monids;
			}

			m_StopMonitorQueue.push_back( group_request_stopmntr_t(m_GroupRequestId, page_monids) );
		}

		// Make the request.
		processStopMonitorQueue();

		// Get the next id
		++m_GroupRequestId;
	}

	void CBloombergApi::CBBTickMonitor::processStopMonitorQueue()
	{
		while (!m_StopMonitorQueue.empty() && m_StopMonitorRequestMap.size() < 10)
		{
			const group_request_stopmntr_t& request = m_StopMonitorQueue.front();
			group_request_id_t group_request_id = request.first;
			const vec_monid_t& monids = request.second;

			// Pack subject names into appropriate bloomberg array
			monid_t raw_monids[BB_MAX_MONIDS];
			memset((void*) raw_monids, 0, sizeof(raw_monids));
			for (size_t i = 0; i < monids.size(); ++i)
				raw_monids[i]= monids[i];

			// Stop the request
			request_id_t request_id = bb_stopmntr(m_Connection, static_cast<long>(monids.size()), raw_monids);

			if (request_id > 0)
			{
				m_StopMonitorRequestMap[request_id] = monids;
			}
			else
			{
				// Need some kind of error callback.
			}

			m_StopMonitorQueue.pop_front();
		}
	}

	void CBloombergApi::CBBTickMonitor::handleStopMonitor(const bb_comm_header_t* msg)
	{
		// There is no return data on a stop monitor call. We must assume that
		// everything we asked for has been stopped.

		// Go through all stop requests.
		for (stopmntr_req_map_t::iterator i(m_StopMonitorRequestMap.begin()); i != m_StopMonitorRequestMap.end(); ++i)
		{
			request_id_t request_id = i->first;
			vec_monid_t& monids = i->second;
			for (vec_monid_t::iterator i_monid(monids.begin()); i_monid != monids.end(); ++i_monid)
			{
				// Try to find this monid
				tickmntr_map_t::iterator id_map_iter(m_TickMonitorIdMap.find(*i_monid));
				if (id_map_iter != m_TickMonitorIdMap.end())
				{
					// Get the security relevent to the given monid
					const Bloomberg::secdesc_t& sec = id_map_iter->second;

					// Find the reverse lookup entry and remove it.
					tickmntr_rev_map_t::iterator rev_id_map_iter(m_TickMonitorReverseIdMap.find(sec));
					if (rev_id_map_iter != m_TickMonitorReverseIdMap.end())
						m_TickMonitorReverseIdMap.erase(rev_id_map_iter);

					// Remove the tick monitor entry
					m_TickMonitorIdMap.erase(id_map_iter);
				}
			}
		}

		// Everything has been deleted so clear the stop requests.
		m_StopMonitorRequestMap.clear();

		processStopMonitorQueue();
	}

	void CBloombergApi::CBBTickMonitor::handleTickMonitor(const bb_msg_monid_enhanced_t* msg_monid)
	{
		request_id_t request_id = msg_monid->comm_header.request_id;

		// Find the request
		tickmntr_req_map_t::iterator request_map_iter(m_TickMonitorRequestMap.find(request_id));
		if (request_map_iter == m_TickMonitorRequestMap.end())
			return; // we recieved a response we did not request.

		const vec_secdesc_t& secs = request_map_iter->second;

		if (msg_monid->comm_header.num_items == secs.size())
		{
			for (size_t i = 0; i < secs.size(); ++i)
			{
				const secdesc_t& sec = secs[i];
				monid_t monid = msg_monid->mon_id[i];

				if (monid != -1)
				{
					m_TickMonitorIdMap[monid] = sec;
					m_TickMonitorReverseIdMap[sec] = monid;
				}
			}
		}
		else
		{
			// need an error callback.
		}

		// We can remove the request, now we have an actual monitor id.
		m_TickMonitorRequestMap.erase(request_map_iter);

		processTickMonitorQueue();
	}

	void CBloombergApi::CBBTickMonitor::handleTickData(const bb_msg_tickx_t* msg_tick)
	{
		std::string NA = "N.A.";

		sec_retval_map_t sec_data_map;

		for (size_t i = 0; i < static_cast<size_t>(msg_tick->comm_header.num_items); ++i)
		{
			const bb_decode_tickx_t& tick_data = msg_tick->tick_data[i];

			// Find the security to which this monid pertains
			tickmntr_map_t::const_iterator tickmon_iter = m_TickMonitorIdMap.find(tick_data.mon_id);
			if (tickmon_iter == m_TickMonitorIdMap.end())
				continue;

			// make a copy so we can release the lock
			const secdesc_t& sec = tickmon_iter->second;

			retval_map_t data_map;

			// Handle the data
			switch (tick_data.action)
			{
			case bTickTRADE:
				data_map["TRADE_PRICE"] = (tick_data.data.TRADE.price     == BB_VAL_MISSING ? NA : bb_todouble(tick_data.data.TRADE.price));
				data_map["TRADE_SIZE"]  = (tick_data.data.TRADE.size      == BB_VAL_MISSING ? NA : bb_toint(tick_data.data.TRADE.size));
				data_map["TRADE_TIME"]  = (tick_data.data.TRADE.time      == BB_VAL_MISSING ? NA : Bloomberg::bb_time_t(tick_data.data.TRADE.time));
				break;
			case bTickBID:
				data_map["BID_PRICE"]   = (tick_data.data.BID.price       == BB_VAL_MISSING ? NA : bb_todouble(tick_data.data.BID.price));
				data_map["BID_SIZE"]    = (tick_data.data.BID.size        == BB_VAL_MISSING ? NA : bb_toint(tick_data.data.BID.size));
				break;
			case bTickASK:
				data_map["ASK_PRICE"]   = (tick_data.data.ASK.price       == BB_VAL_MISSING ? NA : bb_todouble(tick_data.data.ASK.price));
				data_map["ASK_SIZE"]    = (tick_data.data.ASK.size        == BB_VAL_MISSING ? NA : bb_toint(tick_data.data.ASK.size));
				break;
			case bTickHIT:				// Trade on Bid
				data_map["HIT_PRICE"]   = (tick_data.data.HIT.price       == BB_VAL_MISSING ? NA : bb_todouble(tick_data.data.HIT.price));
				data_map["HIT_SIZE"]    = (tick_data.data.HIT.size        == BB_VAL_MISSING ? NA : bb_toint(tick_data.data.HIT.size));
				break;
			case bTickTAKE:				// Trade on Ask
				data_map["TAKE_PRICE"]  = (tick_data.data.TAKE.price      == BB_VAL_MISSING ? NA : bb_todouble(tick_data.data.TAKE.price));
				data_map["TAKE_SIZE"]   = (tick_data.data.TAKE.size       == BB_VAL_MISSING ? NA : bb_toint(tick_data.data.TAKE.size));
				break;
			case bTickSETTLE:
				data_map["SETTLE"]      = (tick_data.data.SETTLE.price    == BB_VAL_MISSING ? NA : bb_todouble(tick_data.data.SETTLE.price));
				break;
			case bTickVOLUME:
				data_map["VOLUME"]      = (tick_data.data.VOLUME.volume   == BB_VAL_MISSING ? NA : bb_toint(tick_data.data.VOLUME.volume));
				break;
			case bTickOPEN_INTEREST:
				data_map["OPEN_INTEREST"] = "Y";
				break;
			case bTickHIGH:				// High Recap
				data_map["HIGH"]        = (tick_data.data.HIGH.price      == BB_VAL_MISSING ? NA : bb_todouble(tick_data.data.HIGH.price));
				break;
			case bTickLOW:				// Low Recap
				data_map["LOW"]         = (tick_data.data.LOW.price       == BB_VAL_MISSING ? NA : bb_todouble(tick_data.data.LOW.price));
				break;
			case bTickBID_YIELD:		// Up Limit
				data_map["BID_YIELD"]   = (tick_data.data.BID_YIELD.yield == BB_VAL_MISSING ? NA : bb_todouble(tick_data.data.BID_YIELD.yield));
				break;
			case bTickASK_YIELD:		// Down Limit
				data_map["ASK_YIELD"]   = (tick_data.data.ASK_YIELD.yield == BB_VAL_MISSING ? NA : bb_todouble(tick_data.data.ASK_YIELD.yield));
				break;
			case bTickBT_LAST_RECAP:	// Used for intraday bars
				break;
			case bTickNEWS_STORY:		// Just an indicator
				data_map["NEWS_STORY"]  = "Y";
				break;
			case bTickBID_MKT_MAKER:
				data_map["BID_MKT_MAKER_PRICE"] = (tick_data.data.BID_MKT_MAKER.price     == BB_VAL_MISSING ? NA : bb_todouble(tick_data.data.BID_MKT_MAKER.price));
				data_map["BID_MKT_MAKER_SIZE"]  = (tick_data.data.BID_MKT_MAKER.size      == BB_VAL_MISSING ? NA : bb_toint(tick_data.data.BID_MKT_MAKER.size));
				data_map["BID_MKT_MAKER"]       = (tick_data.data.BID_MKT_MAKER.mkt_maker == BB_VAL_MISSING ? NA : bb_toint(tick_data.data.BID_MKT_MAKER.mkt_maker));
				break;
			case bTickASK_MKT_MAKER:
				data_map["ASK_MKT_MAKER_PRICE"] = (tick_data.data.ASK_MKT_MAKER.price     == BB_VAL_MISSING ? NA : bb_todouble(tick_data.data.ASK_MKT_MAKER.price));
				data_map["ASK_MKT_MAKER_SIZE"]  = (tick_data.data.ASK_MKT_MAKER.size      == BB_VAL_MISSING ? NA : bb_toint(tick_data.data.ASK_MKT_MAKER.size));
				data_map["ASK_MKT_MAKER"]       = (tick_data.data.ASK_MKT_MAKER.mkt_maker == BB_VAL_MISSING ? NA : bb_toint(tick_data.data.ASK_MKT_MAKER.mkt_maker));
				break;
			case bTickBT_MID_PRICE:		// london Stock Exchange Mid Price
				data_map["MID_PRICE"]           = (tick_data.data.BT_MID_PRICE.price      == BB_VAL_MISSING ? NA : bb_todouble(tick_data.data.BT_MID_PRICE.price));
				break;
			case bTickBT_LSE_LAST:		// London Stock Exchange Last Trade
				data_map["BT_LSE_LAST_PRICE"]   = (tick_data.data.BT_LSE_LAST.price       == BB_VAL_MISSING ? NA : bb_todouble(tick_data.data.BT_LSE_LAST.price));
				break;
#ifdef bTickMAN_TRADE_WITH_SIZE
			case bTickMAN_TRADE_WITH_SIZE:		// copy of bTickBT_LSE_LAST with size
				data_map["BT_LSE_LAST_PRICE"]   = (tick_data.data.BT_LSE_LAST.price       == BB_VAL_MISSING ? NA : bb_todouble(tick_data.data.BT_LSE_LAST.price));
				data_map["BT_LSE_LAST_SIZE"]    = (tick_data.data.BT_LSE_LAST.size        == BB_VAL_MISSING ? NA : bb_toint(tick_data.data.BT_LSE_LAST.size));
				break;
#endif
			case bTickNEW_MKT_DAY:		// Just an indicator
				data_map["NEW_MKT_DAY"]         = "Y";
				break;
			case bTickCANCEL_CORRECT:
				data_map["CANCEL_CORRECT"]      = "Y";
				break;
			case bTickOPEN:				// Used for intraday bars
				data_map["OPEN"]                = (tick_data.data.OPEN.price              == BB_VAL_MISSING ? NA : bb_todouble(tick_data.data.OPEN.price));
				break;
			case bTickBT_BID_RECAP:
				break;
			case bTickBT_ASK_RECAP:
				break;
			case bTickMKT_INDICATOR:
				data_map["MKT_INDICATOR_TADING"]           = (tick_data.data.MKT_INDICATOR.trading          == BB_VAL_MISSING ? NA : bb_tobool(tick_data.data.MKT_INDICATOR.trading));
				data_map["MKT_INDICATOR_QUOTATION"]        = (tick_data.data.MKT_INDICATOR.quotation        == BB_VAL_MISSING ? NA : bb_tobool(tick_data.data.MKT_INDICATOR.quotation));
				data_map["MKT_INDICATOR_EXCH_MKT_STATUS"]  = (tick_data.data.MKT_INDICATOR.exch_mkt_status  == BB_VAL_MISSING ? NA : bb_toint(tick_data.data.MKT_INDICATOR.exch_mkt_status));
				data_map["MKT_INDICATOR_SIMPL_SEC_STATUS"] = (tick_data.data.MKT_INDICATOR.simpl_sec_status == BB_VAL_MISSING ? NA : bb_toint(tick_data.data.MKT_INDICATOR.simpl_sec_status));
				break;
			case bTickBT_MKT_TURN:		// Used for intraday bars
				break;
			case bTickVOLUME_UPDATE:
				data_map["MKT_INDICATOR_TADING"]    = (tick_data.data.MKT_INDICATOR.trading   == BB_VAL_MISSING ? NA : bb_tobool(tick_data.data.MKT_INDICATOR.trading));
				data_map["MKT_INDICATOR_QUOTATION"] = (tick_data.data.MKT_INDICATOR.quotation == BB_VAL_MISSING ? NA : bb_tobool(tick_data.data.MKT_INDICATOR.quotation));
				break;
			case bTickBT_SEC_BID:		// Secondary Bid
				data_map["BT_SEC_BID"]              = (tick_data.data.BT_SEC_BID.price == BB_VAL_MISSING ? NA : bb_todouble(tick_data.data.BT_SEC_BID.price));
				break;
			case bTickBT_SEC_ASK:		// Secondary Ask
				data_map["BT_SEC_ASK"]              = (tick_data.data.BT_SEC_ASK.price == BB_VAL_MISSING ? NA : bb_todouble(tick_data.data.BT_SEC_ASK.price));
				break;
			case bTickTICK_NUM:			// Used for intraday bars
				break;
			case bTickBID_LIFT:			// Just an indicator
				data_map["BID_LIFT"] = "Lifted";
				break;
			case bTickASK_LIFT:			// Just an indicator
				data_map["ASK_LIFT"] = "Lifted";
				break;
			case bTickVWAP:				// Vwap tick type
				data_map["VWAP_PRICE"]      = (tick_data.data.VWAP.price == BB_VAL_MISSING ? NA : bb_todouble(tick_data.data.VWAP.price));
				data_map["VWAP_VOLUME"]     = (tick_data.data.VWAP.size  == BB_VAL_MISSING ? NA : bb_todouble(tick_data.data.VWAP.size));
				break;
			// JAB Begin new tick types
			case bTickHIGH_YIELD:
				data_map["HIGH_YIELD"]      = (tick_data.data.HIGH_YIELD.yield == BB_VAL_MISSING ? NA : bb_todouble(tick_data.data.HIGH_YIELD.yield));
				break;
			case bTickLOW_YIELD:
				data_map["LOW_YIELD"]       = (tick_data.data.LOW_YIELD.yield  == BB_VAL_MISSING ? NA : bb_todouble(tick_data.data.LOW_YIELD.yield));
				break;
			case bTickYIELD:
				data_map["YIELD"]           = (tick_data.data.YIELD.yield      == BB_VAL_MISSING ? NA : bb_todouble(tick_data.data.YIELD.yield));
				break;
			case bTickAT_TRADE:
				data_map["AT_TRADE_PRICE"]  = (tick_data.data.AT_TRADE.price == BB_VAL_MISSING ? NA : bb_todouble(tick_data.data.AT_TRADE.price));
				data_map["AT_TRADE_VOLUME"] = (tick_data.data.AT_TRADE.size  == BB_VAL_MISSING ? NA : bb_toint(tick_data.data.AT_TRADE.size));
				break;
			// JAB End new tick types
			case bTickPRICE_UPDATE:
				break;
			case bTickALL_PRICE:
				data_map["ALL_PRICE"]      = (tick_data.data.ALL_PRICE.price == BB_VAL_MISSING ? NA : bb_todouble(tick_data.data.ALL_PRICE.price));
				data_map["ALL_PRICE_SIZE"] = (tick_data.data.ALL_PRICE.size  == BB_VAL_MISSING ? NA : bb_toint(tick_data.data.ALL_PRICE.size));
				break;
			/* Required to request Market Depth tick data */
			case bTickSMART:
			case bTickMARKET_DEPTH:
				handleSmartTickData(
					data_map,
					&tick_data,
					getTickExtra(msg_tick, static_cast<int>(i)));
				break;
			default:
				break;
			}

			if (!data_map.empty())
				sec_data_map[sec] = data_map;
		}

		callbackClient(sec_data_map);
	}

	void CBloombergApi::CBBTickMonitor::handleSmartTickData(retval_map_t& data_map, const bb_decode_tickx_t *pDecodeTickx, const bb_decode_tick_extra_t *pTickExtra)
	{
		if (pDecodeTickx == 0 || pTickExtra == 0)
			return;

		const bb_decode_tick_data_t *pDecodeTickData = pTickExtra->data;
		for(int4 fieldIndex = 0; fieldIndex < pTickExtra->count; fieldIndex++)
		{
			displaySmartTickData(data_map, pDecodeTickx->action, pDecodeTickData);
			pDecodeTickData = getNextDecodeTickData(pDecodeTickData);
		}
	}

	void CBloombergApi::CBBTickMonitor::displaySmartTickData(retval_map_t& data_map, int4 action, const bb_decode_tick_data_t *pDecodeTickData)
	{
		if (pDecodeTickData == 0)
			return;

		switch (action)
		{
		case bTickSMART:
		case bTickMARKET_DEPTH:
			switch (pDecodeTickData->field_id)
			{
			case bFldBEST_BID1:
				data_map["BEST_BID1"] << *pDecodeTickData;
				break;
			case bFldBEST_BID2:
				data_map["BEST_BID2"] << *pDecodeTickData;
				break;
			case bFldBEST_BID3:
				data_map["BEST_BID3"] << *pDecodeTickData;
				break;
			case bFldBEST_BID4:
				data_map["BEST_BID4"] << *pDecodeTickData;
				break;
			case bFldBEST_BID5:
				data_map["BEST_BID5"] << *pDecodeTickData;
				break;

			case bFldBEST_ASK1:
				data_map["BEST_ASK1"] << *pDecodeTickData;
				break;
			case bFldBEST_ASK2:
				data_map["BEST_ASK2"] << *pDecodeTickData;
				break;
			case bFldBEST_ASK3:
				data_map["BEST_ASK3"] << *pDecodeTickData;
				break;
			case bFldBEST_ASK4:
				data_map["BEST_ASK4"] << *pDecodeTickData;
				break;
			case bFldBEST_ASK5:
				data_map["BEST_ASK5"] << *pDecodeTickData;
				break;

			case bFldBEST_BID1_SZ:
				data_map["BEST_BID1_SZ"] << *pDecodeTickData;
				break;
			case bFldBEST_BID2_SZ:
				data_map["BEST_BID2_SZ"] << *pDecodeTickData;
				break;
			case bFldBEST_BID3_SZ:
				data_map["BEST_BID3_SZ"] << *pDecodeTickData;
				break;
			case bFldBEST_BID4_SZ:
				data_map["BEST_BID4_SZ"] << *pDecodeTickData;
				break;
			case bFldBEST_BID5_SZ:
				data_map["BEST_BID5_SZ"] << *pDecodeTickData;
				break;

			case bFldBEST_ASK1_SZ:
				data_map["BEST_ASK1_SZ"] << *pDecodeTickData;
				break;
			case bFldBEST_ASK2_SZ:
				data_map["BEST_ASK2_SZ"] << *pDecodeTickData;
				break;
			case bFldBEST_ASK3_SZ:
				data_map["BEST_ASK3_SZ"] << *pDecodeTickData;
				break;
			case bFldBEST_ASK4_SZ:
				data_map["BEST_ASK4_SZ"] << *pDecodeTickData;
				break;
			case bFldBEST_ASK5_SZ:
				data_map["BEST_ASK5_SZ"] << *pDecodeTickData;
				break;

			default:
				break;
			}
			break;
		default:
			break;
		}
	}

	void CBloombergApi::CBBTickMonitor::callbackClient(const sec_retval_map_t& sec_data_map)
	{
		// For each callback
		for (callback_map_t::const_iterator i_callback(m_CallbackMap.begin()); i_callback != m_CallbackMap.end(); ++i_callback)
		{
			const CClientCallback* callback = i_callback->first;
			const set_secdesc_t& secset = i_callback->second;

			// Make a data map containing the securities monitored by this callback
			sec_retval_map_t tmp_sec_data_map;
			for (set_secdesc_t::const_iterator i_secset(secset.begin()); i_secset != secset.end(); ++i_secset)
			{
				const secdesc_t& sec = *i_secset;
				sec_retval_map_t::const_iterator i_sec_data_map(sec_data_map.find(sec));
				if (i_sec_data_map != sec_data_map.end())
					tmp_sec_data_map[sec] = i_sec_data_map->second;
			}
			
			if (!tmp_sec_data_map.empty())
				const_cast<CClientCallback*>(callback)->handleData(tmp_sec_data_map);
		}
	}
}