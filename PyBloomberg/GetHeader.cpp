#include "stdafx.h"

#define BB_USING_DLL
#include <bbapi.h>
#include <bbunix.h>

#include <BloombergApi/bbutils.h>
#include <BloombergApi/BloombergApi.h>

#include "pack_unpack.h"

extern PyObject *PyBloombergError;
extern Bloomberg::CBloombergApi *bloomberg;

class CGetHeader : public Bloomberg::CBloombergApi::CBBHeaderRequest::CClientCallback
{
public:
	CGetHeader() {}

	const Bloomberg::CBloombergApi::CBBHeaderRequest::sec_retval_map_t& getDataCache() const
	{
		return m_DataCache;
	}

private:
	virtual void handleData(const Bloomberg::CBloombergApi::CBBHeaderRequest::sec_retval_map_t& secdata)
	{
		// Copy the data into the cache
		m_DataCache = secdata;
	}

private:
	Bloomberg::CBloombergApi::CBBHeaderRequest::sec_retval_map_t m_DataCache;
};

PyObject *
PyBloomberg_getheader(PyObject* self, PyObject* args)
{
	PyObject* Tickers = NULL;
	if (!PyArg_ParseTuple(args, "O", &Tickers))
		return NULL;

	try
	{
		Bloomberg::CBloombergApi::set_secdesc_t tickers = unpack_tickers(Tickers);

		CGetHeader header;
		bloomberg->requestHeader(tickers, &header);

		timeval t = {1, 0};
		bloomberg->dispatchLoop(false, t);

		return pack_result_set(header.getDataCache());
	}
	catch (const std::runtime_error& e)
	{
		PyErr_SetString(PyBloombergError, e.what());
		return NULL;
	}
	catch (const char* s)
	{
		PyErr_SetString(PyBloombergError, s);
		return NULL;
	}
}
