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
PyBloomberg_requestdata(PyObject* self, PyObject* args)
{
	PyObject* Tickers = NULL;
	PyObject* Fields = NULL;
	PyObject* Overrides = NULL;
	PyObject* Callback = NULL;

	if (!PyArg_ParseTuple(args, "OOOO", &Tickers, &Fields, &Overrides, &Callback))
		return NULL;

	try
	{
		Bloomberg::CBloombergApi::set_secdesc_t tickers = unpack_tickers(Tickers);
		Bloomberg::CBloombergApi::set_fieldid_t fields = unpack_fields(Fields);
		Bloomberg::CBloombergApi::set_override_t overrides;
		if (Overrides != NULL)
			overrides = unpack_overrides(Overrides);
		Bloomberg::CBloombergApi::CBBDataRequest::CClientCallback* callback = new CPyClientCallback_Data(Callback, true);
		bloomberg->requestData(tickers, fields, overrides, callback);
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
