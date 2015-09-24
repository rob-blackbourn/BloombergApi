#pragma once

class CPyClientCallback_Status : public Bloomberg::CBloombergApi::CBBStatusHandler::CClientCallback
{
public:
	CPyClientCallback_Status(PyObject* Callback, bool OneOff)
		:	m_OneOff(OneOff)
	{
		if (!PyCallable_Check(Callback))
			throw "invalid callback";

		Py_INCREF(Callback);
		m_Callback = Callback;
	}

	~CPyClientCallback_Status()
	{
		if (m_Callback != NULL)
		{
			Py_DECREF(m_Callback);
			m_Callback = NULL;
		}
	}

	PyObject* getCallback() { return m_Callback; }
	const PyObject* getCallback() const { return m_Callback; }

private:
	virtual void handleData(int4 request_id, int4 service_code, int4 return_code, int4 num_items)
	{
		const char* service_code_str;

		switch (service_code)
		{
		case bMsgTypeCLIENTREQUEST:
			service_code_str = "CLIENTREQUEST";
			break;
		case bMsgTypeCONNECTASK:
			service_code_str = "CONNECTASK";
			break;
		case bMsgTypeCONNECTGRANT:
			service_code_str = "CONNECTGRANT";
			break;
		case bMsgTypeDISCONNECT:
			service_code_str = "DISCONNECT";
			break;
		case bMsgTypeNOTIFICATION:
			service_code_str = "NOTIFICATION";
			break;
		case bMsgTypeECHOCOMM:
			service_code_str = "ECHOCOMM";
			break;
		case bMsgTypeERRORCOMM:
			service_code_str = "ERRORCOMM";
			break;
		case bMsgTypeERRORBBDB:
			service_code_str = "ERRORBBDB";
			break;
		case bMsgTypeRESETCOMM:
			service_code_str = "RESETCOMM";
			break;
		case bMsgTypeSERVERRESPONSE:
			service_code_str = "SERVERRESPONSE";
			break;
		case bMsgTypeNOTIFY_LUW:
			service_code_str = "NOTIFY_LUW";
			break;
		case bMsgTypeTICKDATA:
			service_code_str = "TICKDATA";
			break;
		case bMsgTypeTRACEDB:
			service_code_str = "TRACEDB";
			break;
		case bMsgTypeTRACECOMM:
			service_code_str = "TRACECOMM";
			break;
		case bMsgTypeLOOKUPADDRESS:
			service_code_str = "LOOKUPADDRESS";
			break;
		case bMsgTypeLOOKUPRESPONSE:
			service_code_str = "LOOKUPRESPONSE";
			break;
		case bMsgTypeHEARTBEAT:
			service_code_str = "HEARTBEAT";
			break;
		case bMsgTypeWAITSELECT:
			service_code_str = "WAITSELECT";
			break;
		case bMsgTypeSENDACCUM:
			service_code_str = "SENDACCUM";
			break;
		case bMsgTypePEERFLAGS:
			service_code_str = "PEERFLAGS";
			break;
		case bMsgTypeALIVE:
			service_code_str = "ALIVE";
			break;
		case bMsgTypeREPORTCOMM:
			service_code_str = "REPORTCOMM";
			break;
		case bMsgTypeREPORTDB:
			service_code_str = "REPORTDB";
			break;
		case bMsgTypeKILLUSER:
			service_code_str = "KILLUSER";
			break;
		case bMsgTypeEXITCOMM:
			service_code_str = "EXITCOMM";
			break;
		case bMsgTypeEXITDB:
			service_code_str = "EXITDB";
			break;
		case bMsgTypeCOMMPARMSET:
			service_code_str = "COMMPARMSET";
			break;
//		case bMsgTypeSETVALUES:
//			service_code_str = "SETVALUES";
//			break;
		case bMsgTypeROUTEADD:
			service_code_str = "ROUTEADD";
			break;
		case bMsgTypeROUTEDELETE:
			service_code_str = "ROUTEDELETE";
			break;
		case bMsgTypeROUTEREPLACE:
			service_code_str = "ROUTEREPLACE";
			break;
		case bMsgTypeROUTEFILE:
			service_code_str = "ROUTEFILE";
			break;
		case bMsgTypeREROUTEREQUEST:
			service_code_str = "REROUTEREQUEST";
			break;
		case bMsgTypeSEND_TEST_WRAP_TICK:
			service_code_str = "SEND_TEST_WRAP_TICK";
			break;
		case bMsgTypeCONTROLLER_HEARTBEAT:
			service_code_str = "CONTROLLER_HEARTBEAT";
			break;
		case bMsgTypeCONTROLLER_VERSION:
			service_code_str = "CONTROLLER_VERSION";
			break;
		case bMsgTypeCONTROLLER_LUW:
			service_code_str = "CONTROLLER_LUW";
			break;
		case bMsgTypeCONTROLLER_TICKDATA:
			service_code_str = "CONTROLLER_TICKDATA";
			break;
		case bMsgTypeCONTROLLER_SUBSCRIBE:
			service_code_str = "CONTROLLER_SUBSCRIBE";
			break;
		case bMsgTypeCONTROLLER_DESUBSCRIBE:
			service_code_str = "CONTROLLER_DESUBSCRIBE";
			break;
		case bMsgTypeCONTROLLER_MISCTEXT:
			service_code_str = "CONTROLLER_MISCTEXT";
			break;
		case bMsgTypeCONTROLLER_RESUBSCRIBE:
			service_code_str = "CONTROLLER_RESUBSCRIBE";
			break;
		case bMsgTypeCOOKIEON:
			service_code_str = "COOKIEON";
			break;
		case bMsgTypeCOOKIEOFF:
			service_code_str = "COOKIEOFF";
			break;
		case bMsgTypeCOOKIESTATS:
			service_code_str = "COOKIESTATS";
			break;
		case bMsgTypeCOOKIEFLAGS:
			service_code_str = "COOKIEFLAGS";
			break;
		default:
			service_code_str = "unknown";
			break;
		}

		PyObject* result = PyObject_CallFunctionObjArgs(
			m_Callback,
			PyInt_FromLong(request_id),
			PyInt_FromLong(service_code),
			PyInt_FromLong(return_code),
			PyInt_FromLong(num_items), NULL);
		if (result != NULL)
			Py_DECREF(result);

		if (m_OneOff)
			delete this;
	}

private:
	PyObject* m_Callback;
	bool m_OneOff;
};

class CPyClientCallback_Data : public Bloomberg::CBloombergApi::CBBDataRequest::CClientCallback
{
public:
	CPyClientCallback_Data(PyObject* Callback, bool OneOff)
		:	m_OneOff(OneOff)
	{
		if (!PyCallable_Check(Callback))
			throw "invalid callback";

		Py_INCREF(Callback);
		m_Callback = Callback;
	}

	~CPyClientCallback_Data()
	{
		if (m_Callback != NULL)
		{
			Py_DECREF(m_Callback);
			m_Callback = NULL;
		}
	}

	PyObject* getCallback() { return m_Callback; }
	const PyObject* getCallback() const { return m_Callback; }

private:
	virtual void handleData(const Bloomberg::CBloombergApi::CBBDataRequest::sec_retval_map_t& secdata)
	{
		PyObject* py_secdata = PyDict_New();
		for (Bloomberg::CBloombergApi::CBBDataRequest::sec_retval_map_t::const_iterator i_secdata(secdata.begin()); i_secdata != secdata.end(); ++i_secdata)
		{
			const Bloomberg::secdesc_t& sec = i_secdata->first;
			const Bloomberg::CBloombergApi::CBBDataRequest::retval_map_t& data = i_secdata->second;

			PyObject* py_data = PyDict_New();
			for (Bloomberg::CBloombergApi::CBBDataRequest::retval_map_t::const_iterator i_data(data.begin()); i_data != data.end(); ++i_data)
			{
				PyDict_SetItem(
					py_data,
					PyInt_FromLong(i_data->first),
					PyString_FromString(i_data->second.c_str()));
			}

			PyDict_SetItem(
				py_secdata,
				PyString_FromString(sec.first.c_str()),
				py_data);
		}

		PyObject* args = Py_BuildValue("N", py_secdata);
		PyObject* result = PyObject_CallFunctionObjArgs(m_Callback, py_secdata, NULL);
		Py_DECREF(args);
		if (result != NULL)
			Py_DECREF(result);

		if (m_OneOff)
			delete this;
	}

private:
	PyObject* m_Callback;
	bool m_OneOff;
};

class CPyClientCallback_Header : public Bloomberg::CBloombergApi::CBBHeaderRequest::CClientCallback
{
public:
	CPyClientCallback_Header(PyObject* Callback, bool OneOff)
		:	m_OneOff(OneOff)
	{
		if (!PyCallable_Check(Callback))
			throw "invalid callback";

		Py_INCREF(Callback);
		m_Callback = Callback;
	}

	~CPyClientCallback_Header()
	{
		if (m_Callback != NULL)
		{
			Py_DECREF(m_Callback);
			m_Callback = NULL;
		}
	}

	PyObject* getCallback() { return m_Callback; }
	const PyObject* getCallback() const { return m_Callback; }

private:
	virtual void handleData(const Bloomberg::CBloombergApi::CBBHeaderRequest::sec_retval_map_t& secdata)
	{
		PyObject* py_secdata = PyDict_New();
		for (Bloomberg::CBloombergApi::CBBHeaderRequest::sec_retval_map_t::const_iterator i_secdata(secdata.begin()); i_secdata != secdata.end(); ++i_secdata)
		{
			const Bloomberg::secdesc_t& sec = i_secdata->first;
			const Bloomberg::CBloombergApi::CBBHeaderRequest::retval_map_t& data = i_secdata->second;

			PyObject* py_data = PyDict_New();
			for (Bloomberg::CBloombergApi::CBBHeaderRequest::retval_map_t::const_iterator i_data(data.begin()); i_data != data.end(); ++i_data)
			{
				PyDict_SetItem(
					py_data,
					PyString_FromString(i_data->first.c_str()),
					PyString_FromString(i_data->second.c_str()));
			}

			PyDict_SetItem(
				py_secdata,
				PyString_FromString(sec.first.c_str()),
				py_data);
		}

		PyObject* args = Py_BuildValue("N", py_secdata);
		PyObject* result = PyObject_CallFunctionObjArgs(m_Callback, py_secdata, NULL);
		Py_DECREF(args);
		if (result != NULL)
			Py_DECREF(result);

		if (m_OneOff)
			delete this;
	}

private:
	PyObject* m_Callback;
	bool m_OneOff;
};

class CPyClientCallback_Tick : public Bloomberg::CBloombergApi::CBBTickMonitor::CClientCallback
{
public:
	CPyClientCallback_Tick(PyObject* Callback, bool OneOff)
		:	m_OneOff(OneOff)
	{
		if (!PyCallable_Check(Callback))
			throw "invalid callback";

		Py_INCREF(Callback);
		m_Callback = Callback;
	}

	~CPyClientCallback_Tick()
	{
		if (m_Callback != NULL)
		{
			Py_DECREF(m_Callback);
			m_Callback = NULL;
		}
	}

	PyObject* getCallback() { return m_Callback; }
	const PyObject* getCallback() const { return m_Callback; }

private:
	virtual void handleData(const Bloomberg::CBloombergApi::CBBTickMonitor::sec_retval_map_t& secdata)
	{
		PyObject* py_secdata = PyDict_New();
		for (Bloomberg::CBloombergApi::CBBTickMonitor::sec_retval_map_t::const_iterator i_secdata(secdata.begin()); i_secdata != secdata.end(); ++i_secdata)
		{
			const Bloomberg::secdesc_t& sec = i_secdata->first;
			const Bloomberg::CBloombergApi::CBBTickMonitor::retval_map_t& data = i_secdata->second;

			PyObject* py_data = PyDict_New();
			for (Bloomberg::CBloombergApi::CBBTickMonitor::retval_map_t::const_iterator i_data(data.begin()); i_data != data.end(); ++i_data)
			{
				PyDict_SetItem(
					py_data,
					PyString_FromString(i_data->first.c_str()),
					PyString_FromString(i_data->second.c_str()));
			}

			PyDict_SetItem(
				py_secdata,
				PyString_FromString(sec.first.c_str()),
				py_data);
		}

		PyObject* args = Py_BuildValue("N", py_secdata);
		PyObject* result = PyObject_CallFunctionObjArgs(m_Callback, py_secdata, NULL);
		Py_DECREF(args);
		if (result != NULL)
			Py_DECREF(result);

		if (m_OneOff)
			delete this;
	}

private:
	PyObject* m_Callback;
	bool m_OneOff;
};

class CPyClientCallback_HistData : public Bloomberg::CBloombergApi::CBBHistDataRequest::CClientCallback
{
public:
	CPyClientCallback_HistData(PyObject* Callback, bool OneOff)
		:	m_OneOff(OneOff)
	{
		if (!PyCallable_Check(Callback))
			throw "invalid callback";

		Py_INCREF(Callback);
		m_Callback = Callback;
	}

	~CPyClientCallback_HistData()
	{
		if (m_Callback != NULL)
		{
			Py_DECREF(m_Callback);
			m_Callback = NULL;
		}
	}

	PyObject* getCallback() { return m_Callback; }
	const PyObject* getCallback() const { return m_Callback; }

private:
	virtual void handleData(const Bloomberg::CBloombergApi::CBBHistDataRequest::sec_retval_map_t& secdata)
	{
		PyObject* py_secdata = PyDict_New();
		for (Bloomberg::CBloombergApi::CBBHistDataRequest::sec_retval_map_t::const_iterator i_secdata(secdata.begin()); i_secdata != secdata.end(); ++i_secdata)
		{
			const Bloomberg::secdesc_t& sec = i_secdata->first;
			const Bloomberg::CBloombergApi::CBBHistDataRequest::retval_map_t& data = i_secdata->second;

			PyObject* py_data = PyDict_New();
			for (Bloomberg::CBloombergApi::CBBHistDataRequest::retval_map_t::const_iterator i_data(data.begin()); i_data != data.end(); ++i_data)
			{
				const Bloomberg::fieldid_t& fieldid = i_data->first;
				const Bloomberg::CBloombergApi::CBBHistDataRequest::vec_retval_t& vec = i_data->second;

				PyObject* py_vec = PyDict_New();
				for (Bloomberg::CBloombergApi::CBBHistDataRequest::vec_retval_t::const_iterator i_vec(vec.begin()); i_vec != vec.end(); ++i_vec)
				{
					PyDict_SetItem(
						py_vec,
						PyInt_FromLong(i_vec->first),
						PyFloat_FromDouble(i_vec->second));
				}

				PyDict_SetItem(
					py_data,
					PyInt_FromLong(fieldid),
					py_vec);
			}

			PyDict_SetItem(
				py_secdata,
				PyString_FromString(sec.first.c_str()),
				py_data);
		}

		PyObject* args = Py_BuildValue("N", py_secdata);
		PyObject* result = PyObject_CallFunctionObjArgs(m_Callback, py_secdata, NULL);
		Py_DECREF(args);
		if (result != NULL)
			Py_DECREF(result);

		if (m_OneOff)
			delete this;
	}

private:
	PyObject* m_Callback;
	bool m_OneOff;
};
