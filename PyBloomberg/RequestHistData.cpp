#include "stdafx.h"

#define BB_USING_DLL
#include <bbapi.h>
#include <bbunix.h>

#include <BloombergApi/bbutils.h>
#include <BloombergApi/BloombergApi.h>

extern PyObject *PyBloombergError;
extern Bloomberg::CBloombergApi *bloomberg;

#include "PyClientCallback.h"
#include "pack_unpack.h"

PyObject*
PyBloomberg_requesthistdata(PyObject* self, PyObject* args)
{
	PyObject* Tickers = NULL, * Fields = NULL, * Callback = NULL;
	int from, to, flags = 0;
	if (!PyArg_ParseTuple(args, "OOiiiO", &Tickers, &Fields, &from, &to, &flags, &Callback))
		return NULL;

	try
	{
		Bloomberg::CBloombergApi::set_secdesc_t tickers = unpack_tickers(Tickers);
		Bloomberg::CBloombergApi::set_fieldid_t fields = unpack_fields(Fields);
		Bloomberg::CBloombergApi::CBBHistDataRequest::datespec_t datespec(from, to, flags);

		Bloomberg::CBloombergApi::CBBHistDataRequest::CClientCallback* callback = new CPyClientCallback_HistData(Callback, true);
		bloomberg->requestHistData(tickers, fields, datespec, callback);
		Py_INCREF(Py_None);
		return Py_None;
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
