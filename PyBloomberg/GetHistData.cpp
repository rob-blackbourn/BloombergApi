#include "stdafx.h"

#define BB_USING_DLL
#include <bbapi.h>
#include <bbunix.h>

#include <BloombergApi/bbutils.h>
#include <BloombergApi/BloombergApi.h>

#include "pack_unpack.h"

extern PyObject *PyBloombergError;
extern Bloomberg::CBloombergApi *bloomberg;

class CGetHistData : public Bloomberg::CBloombergApi::CBBHistDataRequest::CClientCallback
{
public:
	CGetHistData() {}

	const Bloomberg::CBloombergApi::CBBHistDataRequest::sec_retval_map_t& getDataCache() const
	{
		return m_DataCache;
	}

private:
	virtual void handleData(const Bloomberg::CBloombergApi::CBBHistDataRequest::sec_retval_map_t& secdata)
	{
		// Copy the data into the cache
		m_DataCache = secdata;
	}

private:
	Bloomberg::CBloombergApi::CBBHistDataRequest::sec_retval_map_t m_DataCache;
};

PyObject *
PyBloomberg_gethistdata(PyObject* self, PyObject* args)
{
	PyObject* Tickers = NULL, * Fields = NULL;
	int from, to, flags = 0;
	if (!PyArg_ParseTuple(args, "OOii|i", &Tickers, &Fields, &from, &to, &flags))
		return NULL;

	try
	{
		Bloomberg::CBloombergApi::set_secdesc_t tickers = unpack_tickers(Tickers);
		Bloomberg::CBloombergApi::set_fieldid_t fields = unpack_fields(Fields);

		Bloomberg::CBloombergApi::CBBHistDataRequest::datespec_t datespec(from, to, flags);

		CGetHistData hist_data;
		bloomberg->requestHistData(tickers, fields, datespec, &hist_data);

		timeval t = {1, 0};
		bloomberg->dispatchLoop(false, t);

		return pack_result_set(hist_data.getDataCache());
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
