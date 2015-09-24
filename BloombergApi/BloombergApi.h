#pragma once

namespace Bloomberg
{
	class CBloombergApi
	{
	public:
		const static int BLP_PORT = 8194;

		typedef unsigned long group_request_id_t;
		typedef std::map<group_request_id_t, size_t> group_req_count_map_t;
		typedef std::map<request_id_t,group_request_id_t> group_rev_map_t;

		typedef std::vector<monid_t> vec_monid_t;
		typedef std::set<monid_t> set_monid_t;

		typedef std::vector<secname_t> vec_secname_t;
		typedef std::set<secname_t> set_secname_t;

		typedef std::vector<secdesc_t> vec_secdesc_t;
		typedef std::set<secdesc_t> set_secdesc_t;

		typedef std::vector<fieldid_t> vec_fieldid_t;
		typedef std::set<fieldid_t> set_fieldid_t;

		typedef std::pair<fieldid_t,std::string> override_t;
		typedef std::set< override_t > set_override_t;

		typedef std::vector<moncat_t> vec_moncat_t;
		typedef std::set<moncat_t> set_moncat_t;

		class CBBStatusHandler
		{
		public:

			struct CClientCallback
			{
				virtual void handleData(int4 request_id, int4 service_code, int4 return_code, int4 num_items) = 0;
			};

			typedef std::set<CClientCallback*>	callback_set_t;

		public:
			CBBStatusHandler() {}

			void setConnection(bb_connect_t *Connection) { m_Connection = Connection; }
			void handleMessage(const bb_msg_status_t* msg);
			void registerHandler(CClientCallback* callback);
			void unregisterHandler(CClientCallback* callback);

		private:
			bb_connect_t*	m_Connection;
			callback_set_t	m_CallbackSet;
		};

		class CBBHistDataRequest
		{
		public:
			typedef int4 longdate_t;
			typedef int4 histflags_t;
			typedef stdext::triplet<longdate_t, longdate_t, histflags_t> datespec_t;

			// To unpack the data this is all we need to know
			typedef std::pair<vec_secdesc_t, vec_fieldid_t> page_request_t;
			typedef std::map<request_id_t, page_request_t> page_request_map_t;

			// When we're queing the initial request we need to know all this
			typedef stdext::triplet<vec_secdesc_t, vec_fieldid_t, datespec_t> request_t;
			typedef std::pair<group_request_id_t, request_t> group_request_t;
			typedef std::deque<group_request_t> request_queue_t;

			typedef std::pair<longdate_t,double> retval_t;
			typedef std::vector<retval_t> vec_retval_t;
			typedef std::map<fieldid_t, vec_retval_t> retval_map_t;
			typedef std::map<secdesc_t, retval_map_t> sec_retval_map_t;
			typedef std::map<group_request_id_t, sec_retval_map_t> group_response_map_t;

			struct CClientCallback
			{
				virtual void handleData(const std::map<secdesc_t, std::map<fieldid_t, vec_retval_t> >& data) = 0;
			};

			typedef std::map<group_request_id_t, CClientCallback *> callback_map_t;

		public:
			CBBHistDataRequest(size_t QueueLength = 10)
				:	m_Connection(0),
					m_GroupRequestId(0),
					m_QueueLength(QueueLength)
			{
			}

			~CBBHistDataRequest()
			{
			}

			void setConnection(bb_connect_t *Connection) { m_Connection = Connection; }
			void request(const set_secdesc_t& secs, const set_fieldid_t& fields, const datespec_t &dataspec, CClientCallback* callback);
			bool isQueueEmpty() const { return m_RequestQueue.empty() && m_PagedRequestMap.empty(); }
			void processRequestQueue();
			void handleMessage(const bb_msg_mhistory_t* bb_msg_mhistory);
			void handleReceived(group_request_id_t group_request_id);

		private:
			bb_connect_t *m_Connection;
			group_request_id_t m_GroupRequestId;
			size_t m_QueueLength;
			request_queue_t m_RequestQueue;
			page_request_map_t m_PagedRequestMap;
			group_response_map_t m_GroupResponseMap;
			group_req_count_map_t m_GroupRequestCountMap;
			group_rev_map_t m_GroupRequestIdReverseMap;
			callback_map_t m_CallbackMap;
		};

		class CBBDataRequest
		{
		public:
			typedef std::pair<vec_secdesc_t, vec_fieldid_t> page_request_t;
			typedef std::map<request_id_t, page_request_t> page_request_map_t;

			typedef stdext::triplet<vec_secdesc_t, vec_fieldid_t, set_override_t> request_t;
			typedef std::pair<group_request_id_t, request_t> group_request_t;
			typedef std::deque<group_request_t> request_queue_t;

			typedef std::string retval_t;
			typedef std::map<fieldid_t, retval_t> retval_map_t;
			typedef std::map<secdesc_t, retval_map_t> sec_retval_map_t;
			typedef std::map<group_request_id_t, sec_retval_map_t> response_map_t;

			struct CClientCallback
			{
				virtual void handleData(const sec_retval_map_t& data) = 0;
			};

			typedef std::map<group_request_id_t, CClientCallback *> callback_map_t;

		public:
			CBBDataRequest(size_t QueueLength = 10)
				:	m_Connection(0),
					m_GroupRequestId(0),
					m_QueueLength(QueueLength)
			{
			}

			~CBBDataRequest()
			{
			}

			void setConnection(bb_connect_t *Connection) { m_Connection = Connection; }
			void request(const set_secdesc_t& secs, const set_fieldid_t& fields, const set_override_t&overrides, CClientCallback* callback);
			bool isQueueEmpty() const { return m_RequestQueue.empty() && m_PagedRequestMap.empty(); }
			void processRequestQueue();
			void handleMessage(const bb_msg_fieldsx_t* msg);
			void handleReceived(group_request_id_t group_request_id);

		private:
			bb_connect_t *m_Connection;
			group_request_id_t m_GroupRequestId;
			size_t m_QueueLength;
			request_queue_t m_RequestQueue;
			page_request_map_t m_PagedRequestMap;
			response_map_t m_GroupResponseMap;
			group_req_count_map_t m_GroupRequestCountMap;
			group_rev_map_t m_GroupRequestIdReverseMap;
			callback_map_t m_CallbackMap;
		};

		class CBBHeaderRequest
		{
		public:
			typedef std::string mnemonic_t;
			typedef std::string value_t;
			typedef vec_secdesc_t request_t;
			typedef std::pair<group_request_id_t, request_t> group_request_t;
			typedef std::deque<group_request_t> request_queue_t;

			typedef vec_secdesc_t page_request_t;
			typedef std::map<request_id_t, page_request_t> page_request_map_t;

			typedef std::map<mnemonic_t, value_t> retval_map_t;
			typedef std::map<secdesc_t, retval_map_t> sec_retval_map_t;
			typedef std::map<group_request_id_t, sec_retval_map_t> response_map_t;

			struct CClientCallback
			{
				virtual void handleData(const sec_retval_map_t& data) = 0;
			};

			typedef std::map<group_request_id_t, CClientCallback *> callback_map_t;

		public:
			CBBHeaderRequest(size_t QueueLength = 10)
				:	m_GroupRequestId(0),
					m_QueueLength(QueueLength)
			{
			}

			void setConnection(bb_connect_t *Connection) { m_Connection = Connection; }
			void request(const set_secdesc_t& secs, CClientCallback* callback);
			bool isQueueEmpty() const { return m_RequestQueue.empty() && m_PagedRequestMap.empty(); }
			void processRequestQueue();
			void handleMessage(const bb_msg_headerx_t* msg);
			void handleReceived(group_request_id_t group_request_id);

		private:
			bb_connect_t*			m_Connection;
			group_request_id_t		m_GroupRequestId;
			size_t					m_QueueLength;
			request_queue_t			m_RequestQueue;
			page_request_map_t		m_PagedRequestMap;
			response_map_t			m_GroupResponseMap;
			group_req_count_map_t	m_GroupRequestCountMap;
			group_rev_map_t			m_GroupRequestIdReverseMap;
			callback_map_t			m_CallbackMap;
		};

		class CBBTickMonitor
		{
		public:
			typedef std::pair<vec_secdesc_t, set_moncat_t> request_tickmntr_t;
			typedef std::pair<group_request_id_t, request_tickmntr_t> group_request_tickmntr_t;
			typedef std::pair<group_request_id_t, vec_monid_t> group_request_stopmntr_t;

			typedef std::deque<group_request_tickmntr_t> tickmntr_queue_t;
			typedef std::deque<group_request_stopmntr_t> stopmntr_queue_t;

			typedef std::map<request_id_t, vec_secdesc_t> tickmntr_req_map_t;
			typedef std::map<request_id_t, vec_monid_t> stopmntr_req_map_t;

			typedef std::map<monid_t, secdesc_t> tickmntr_map_t;
			typedef std::map<secdesc_t, monid_t> tickmntr_rev_map_t;

			typedef std::map<std::string, std::string> retval_map_t;
			typedef std::map<secdesc_t, retval_map_t> sec_retval_map_t;

			struct CClientCallback
			{
				virtual void handleData(const sec_retval_map_t& data) = 0;
			};

			typedef std::map<CClientCallback*, set_secdesc_t > callback_map_t;

		public:
			CBBTickMonitor(size_t QueueLength = 10)
				:	m_GroupRequestId(0),
					m_QueueLength(QueueLength)
			{
			}

			~CBBTickMonitor()
			{
			}

			void setConnection(bb_connect_t *Connection) { m_Connection = Connection; }
			void request(const set_secdesc_t& secs, const set_moncat_t& cats, CClientCallback* callback);
			void derequest(CClientCallback* callback);
			void derequest(const set_secdesc_t& secs);
			void processTickMonitorQueue();
			void processStopMonitorQueue();

			void handleTickMonitor(const bb_msg_monid_enhanced_t* msg);
			void handleStopMonitor(const bb_comm_header_t* msg);
			void handleTickData(const bb_msg_tickx_t* msg);
			void callbackClient(const sec_retval_map_t& sec_data_map);

			bool isQueueEmpty() { return m_TickMonitorQueue.empty() && m_TickMonitorRequestMap.empty() && m_StopMonitorQueue.empty() && m_StopMonitorRequestMap.empty(); }

		private:
			void handleSmartTickData(retval_map_t& data_map, const bb_decode_tickx_t *pDecodeTickx, const bb_decode_tick_extra_t *pTickExtra);
			void displaySmartTickData(retval_map_t& data_map, int4 action, const bb_decode_tick_data_t *pDecodeTickData);

		private:
			bb_connect_t			*m_Connection;
			size_t					m_QueueLength;
			group_request_id_t		m_GroupRequestId;
			group_req_count_map_t	m_GroupRequestCountMap;
			group_rev_map_t			m_GroupRequestIdReverseMap;
			tickmntr_queue_t		m_TickMonitorQueue;
			stopmntr_queue_t		m_StopMonitorQueue;
			tickmntr_req_map_t		m_TickMonitorRequestMap;
			stopmntr_req_map_t		m_StopMonitorRequestMap;
			tickmntr_map_t			m_TickMonitorIdMap;
			tickmntr_rev_map_t		m_TickMonitorReverseIdMap;
			callback_map_t			m_CallbackMap;
		};

	public:
		CBloombergApi(size_t DataQueueLength = 10, size_t HeaderQueueLength = 10, size_t TickMonitorQueueLength = 10, size_t HistDataQueueLength = 10)
			:	m_Connection(NULL),
				m_StatusHandler(),
				m_DataRequest(DataQueueLength),
				m_HeaderRequest(HeaderQueueLength),
				m_TickMonitor(TickMonitorQueueLength),
				m_HistDataRequest(HistDataQueueLength)
		{
		}

		~CBloombergApi()
		{
			Close();
		}

#if defined(BLOOMBERGAPI_CLIENT)
		bool Open(int port = BLP_PORT);
#elif defined(BLOOMBERGAPI_SERVER)
		bool Open(char* ip, int port = BLP_PORT);
		bool Open(char* ip, int port, int uuid, int sid, int sid_instance, int terminal_sid, int terminal_sid_instance);
#else
#error Must define either BLOOMBERGAPI_CLIENT or BLOOMBERGAPI_SERVER
#endif

		bool Close();

		bool EnableFeature(int feature);

		int getSocket() { return bb_getsocket(m_Connection); }

		void registerStatusHandler(CBBStatusHandler::CClientCallback* callback)
		{
			m_StatusHandler.registerHandler(callback);
		}

		void unregisterStatusHandler(CBBStatusHandler::CClientCallback* callback)
		{
			m_StatusHandler.unregisterHandler(callback);
		}

		void requestHistData(const set_secdesc_t& secs, const set_fieldid_t& fields, const stdext::triplet<int4, int4, int4>& datespec, CBBHistDataRequest::CClientCallback* callback)
		{
			m_HistDataRequest.request(secs, fields, datespec, callback);
		}

		void requestData(const set_secdesc_t& secs, const set_fieldid_t& fields, const set_override_t&overrides, CBBDataRequest::CClientCallback* callback)
		{
			m_DataRequest.request(secs, fields, overrides, callback);
		}

		void requestHeader(const set_secdesc_t& secs, CBBHeaderRequest::CClientCallback* callback)
		{
			m_HeaderRequest.request(secs, callback);
		}

		void requestTickMonitor(const set_secdesc_t& secs, const set_moncat_t& cats, CBBTickMonitor::CClientCallback* callback)
		{
			m_TickMonitor.request(secs, cats, callback);
		}

		void derequestTickMonitor(CBBTickMonitor::CClientCallback* callback)
		{
			m_TickMonitor.derequest(callback);
		}

		void derequestTickMonitor(const set_secdesc_t& secs)
		{
			m_TickMonitor.derequest(secs);
		}

		void dispatchReadEvent(SOCKET fd);
		void dispatchLoop(bool continuous, const timeval& timeout);

		bool isQueueEmpty() { return m_DataRequest.isQueueEmpty() && m_HeaderRequest.isQueueEmpty() & m_TickMonitor.isQueueEmpty() & m_HistDataRequest.isQueueEmpty(); }

	private:

		void setConnection();

	private:
		CRITICAL_SECTION m_Lock;

		bb_connect_t *m_Connection;

		CBBStatusHandler	m_StatusHandler;
		CBBDataRequest		m_DataRequest;
		CBBHeaderRequest	m_HeaderRequest;
		CBBTickMonitor		m_TickMonitor;
		CBBHistDataRequest	m_HistDataRequest;

	};

	BB_MKT getMarketFromTicker(const std::string& ticker);
}