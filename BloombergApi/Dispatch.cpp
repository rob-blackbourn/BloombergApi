#include "stdafx.h"

#define BB_USING_DLL
#include <bbapi.h>
#include <bbunix.h>

#include "bbutils.h"

#include "BloombergApi.h"

namespace Bloomberg
{
	void CBloombergApi::dispatchReadEvent(SOCKET fd)
	{
		bool done = false;
		while (!done)
		{
			long msg_len = bb_sizeof_nextmsg(m_Connection);
			char* msg = 0;
			if (msg_len > 0)
				msg = new char[msg_len];

			int retcode;
			if (msg_len > 0)
				retcode = bb_rcvdata(m_Connection, msg, msg_len);
			else
				retcode = 0;

			switch (retcode)
			{
			case BB_SVC_AUTHORIZATION:
				// permissioning problem
				done = true;
				break;

			case BB_SVC_CONNECTSUCCEEDED:
				// connection succeeded
				done = true;
				break;

			case BB_SVC_CONNECTFAILED:
				// unable to connect to bbcomm
				done = true;
				break;

			case BB_SVC_INCOMPLETE:
				// message incomplete
				done = true;
				break;

			case BB_SVC_GETDATAX:
				try {
					m_DataRequest.handleMessage((bb_msg_fieldsx_t*) msg);
				} catch (const std::runtime_error& /*e*/) {
					// Oh dear
				}
				break;

//			case BB_SVC_GETHEADERX:
			case BB_SVC_GETHEADER_TYPEDX:
				try {
					m_HeaderRequest.handleMessage((bb_msg_headerx_t*) msg);
				} catch (const std::runtime_error& /*e*/) {
					// Oh dear
				}
				break;

//			case BB_SVC_TICKMONITOR:
//				try {
//					m_TickMonitor.handleTickMonitor((bb_msg_monid_t*) msg);
//				} catch (const std::runtime_error& /*e*/) {
//					// Oh dear
//				}
//				break;

//			case BB_SVC_TICKMONITOR_TYPED:
//				try {
//					m_TickMonitor.handleTickMonitor((bb_msg_monid_t*) msg);
//				} catch (const std::runtime_error& /*e*/) {
//					// Oh dear
//				}
//				break;

			case BB_SVC_TICKMONITOR_ENHANCED:
				try {
					m_TickMonitor.handleTickMonitor((bb_msg_monid_enhanced_t*) msg);
				} catch (const std::runtime_error& /*e*/) {
					// Oh dear
				}
				break;

			case BB_SVC_STOPMONITOR:
				try {
					m_TickMonitor.handleStopMonitor((bb_comm_header_t*) msg);
				} catch (const std::runtime_error& /*e*/) {
					// Oh dear
				}
				break;

			case BB_SVC_TICKDATA:
				try {
					m_TickMonitor.handleTickData((bb_msg_tickx_t*) msg);
//				} catch (const std::runtime_error& /*e*/) {
				} catch (...) {
					// Oh dear
				}
				break;

			case BB_SVC_MGETHISTORYX:
				try {
					m_HistDataRequest.handleMessage((bb_msg_mhistory_t *) msg);
				} catch (const std::runtime_error& /*e*/) {
					// Oh dear
				}
				break;

			case BB_SVC_STATUS:
				try {
					m_StatusHandler.handleMessage((bb_msg_status_t *) msg);
				} catch (const std::runtime_error& /*e*/) {
					// Oh dear
				}
				break;

			case ExitFAILCONNECTION:
				// connection failed
				done = true;
				break;

			case ExitFAILRECEIVE:
				// receive failed
				done = true;
				break;

			default:
				if (retcode < -1900)
				{
					// unknown receive error
					done = true;
				}
				else
				{
					// unexpected (hopefully no-fatal) message 
				}
				break;
			}

			if (msg_len > 0)
			{
				delete [] msg;
				msg = 0;
			}
		}
	}

	void CBloombergApi::dispatchLoop(bool continuous, const timeval& timeout)
	{
		int bb_sock = getSocket();

		do {
			/* build select masks with our socket in them */
			fd_set read_set, exec_set;
			FD_ZERO(&read_set);
			FD_ZERO(&exec_set);
			FD_SET((int) bb_sock, &read_set);
			FD_SET((int) bb_sock, &exec_set);

			/* set the select timeout to 1 second */

			/*
			** We care about the socket being readable or having an exception,
			** not about it being writeable, so the third paramter of select is
			** null
			*/
			int rcode = select(bb_sock + 1, &read_set, NULL, &exec_set, &timeout);

			if (rcode == SOCKET_ERROR)
			{
				std::stringstream errstr;
				errstr << "socket error (" << WSAGetLastError() << ") from select";
				throw std::runtime_error(errstr.str());
			}

			if (FD_ISSET(bb_sock, &exec_set))
			{
				throw std::runtime_error("exception on Bloomberg socket");
			}

			if (FD_ISSET(bb_sock, &read_set))
			{
				dispatchReadEvent(bb_sock);

				if (rcode == -1)
				{
					throw std::runtime_error("receive failed");
				}
			}
		} while (continuous || (!continuous && !isQueueEmpty()) );
	}
}