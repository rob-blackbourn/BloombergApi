#include "stdafx.h"

#define BB_USING_DLL
#include <bbapi.h>
#include <bbunix.h>

#include "bbutils.h"

#include "BloombergApi.h"

namespace Bloomberg
{
	void CBloombergApi::CBBStatusHandler::registerHandler(CClientCallback* callback)
	{
		m_CallbackSet.insert(callback);
	}

	void CBloombergApi::CBBStatusHandler::unregisterHandler(CClientCallback* callback)
	{
		m_CallbackSet.erase(callback);
	}

	void CBloombergApi::CBBStatusHandler::handleMessage(const bb_msg_status_t* msg_status)
	{

		// Invoke each registered handler
		for (callback_set_t::iterator i_callback(m_CallbackSet.begin()); i_callback != m_CallbackSet.end(); ++i_callback)
			(*i_callback)->handleData(
				msg_status->comm_header.request_id,
				msg_status->comm_header.service_code,
				msg_status->comm_header.return_code,
				msg_status->comm_header.num_items);
	}
}