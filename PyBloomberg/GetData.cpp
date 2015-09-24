#include "stdafx.h"

#define BB_USING_DLL
#include <bbapi.h>
#include <bbunix.h>

#include <BloombergApi/bbutils.h>
#include <BloombergApi/BloombergApi.h>

#include "pack_unpack.h"

extern PyObject *PyBloombergError;
extern Bloomberg::CBloombergApi *bloomberg;

class CGetData : public Bloomberg::CBloombergApi::CBBDataRequest::CClientCallback
{
public:
	CGetData() {}

	const Bloomberg::CBloombergApi::CBBDataRequest::sec_retval_map_t& getDataCache() const
	{
		return m_DataCache;
	}

private:
	virtual void handleData(const Bloomberg::CBloombergApi::CBBDataRequest::sec_retval_map_t& secdata)
	{
		// Copy the data into the cache
		m_DataCache = secdata;
	}

private:
	Bloomberg::CBloombergApi::CBBDataRequest::sec_retval_map_t m_DataCache;
};

PyObject *
PyBloomberg_getdata(PyObject* self, PyObject* args)
{
	PyObject* Tickers = NULL, * Fields = NULL, * Overrides = NULL;
	if (!PyArg_ParseTuple(args, "OO|O", &Tickers, &Fields, &Overrides))
		return NULL;

	try
	{
		Bloomberg::CBloombergApi::set_secdesc_t tickers = unpack_tickers(Tickers);
		Bloomberg::CBloombergApi::set_fieldid_t fields = unpack_fields(Fields);
		Bloomberg::CBloombergApi::set_override_t overrides;
		if (Overrides != NULL)
			overrides = unpack_overrides(Overrides);

		CGetData data;
		bloomberg->requestData(tickers, fields, overrides, &data);

		timeval t = {1, 0};
		bloomberg->dispatchLoop(false, t);

		return pack_result_set(data.getDataCache());
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
