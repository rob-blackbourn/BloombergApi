#include "stdafx.h"

#define BB_USING_DLL
#include <bbapi.h>
#include <bbunix.h>

#include "bbutils.h"

#include "BloombergApi.h"

namespace Bloomberg
{
#if defined(BLOOMBERGAPI_CLIENT)

	bool CBloombergApi::Open(int port)
	{
		WORD i = MAKEWORD(1,1);
		WSADATA wver;
		int rcode = WSAStartup(i, &wver);
		if (rcode != 0 || LOBYTE(wver.wVersion) != 1 || HIBYTE(wver.wVersion) !=1 )
			return false;

		m_Connection = bb_connect(port);
		if (m_Connection == 0)
			return false;

		int4 id[4];
		if (bb_identify(0, id) != 0)
			return false;

		if (bb_setuser(m_Connection, id) != 0)
			return false;

		setConnection();

		return true;
	}

#elif defined(BLOOMBERGAPI_SERVER)

	bool CBloombergApi::Open(char* ip, int port)
	{
		long rcode = 0;
		m_Connection = bb_connect_server(&rcode, ip, port);
		if (rcode != 0)
			return false;

		setConnection();

		return true;
	}

	bool CBloombergApi::Open(char* ip, int port, int uuid, int sid, int sid_instance, int terminal_sid, int terminal_sid_instance)
	{
		long rcode = 0;
		m_Connection = bb_connect_server_user(&rcode, ip, port, uuid, sid, sid_instance, terminal_sid, terminal_sid_instance);
		if (rcode != 0)
			return false;

		return true;
	}

#else
#error Must define either BLOOMBERGAPI_CLIENT or BLOOMBERGAPI_SERVER
#endif

	bool CBloombergApi::EnableFeature(int feature)
	{
		int rcode = bb_enablefeature(m_Connection,	feature, NULL);
		return rcode > 0;
	}

	void CBloombergApi::setConnection()
	{
		m_StatusHandler.setConnection(m_Connection);
		m_DataRequest.setConnection(m_Connection);
		m_HeaderRequest.setConnection(m_Connection);
		m_TickMonitor.setConnection(m_Connection);
		m_HistDataRequest.setConnection(m_Connection);
	}

	bool CBloombergApi::Close()
	{
		if (m_Connection != NULL)
		{
			int rcode = bb_disconnect(m_Connection);
			m_Connection = 0;
			return rcode == ExitOK;
		}

		return true;
	}

}