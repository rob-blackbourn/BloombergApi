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
PyBloomberg_requestheader(PyObject* self, PyObject* args)
{
	PyObject* Tickers = NULL;
	PyObject* Callback = NULL;

	if (!PyArg_ParseTuple(args, "OO", &Tickers, &Callback))
		return NULL;

	try
	{
		Bloomberg::CBloombergApi::set_secdesc_t tickers = unpack_tickers(Tickers);
		Bloomberg::CBloombergApi::CBBHeaderRequest::CClientCallback* callback = new CPyClientCallback_Header(Callback, true);
		bloomberg->requestHeader(tickers, callback);
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
