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

typedef CPyClientCallback_Status callback_t;
typedef std::set<callback_t*> callback_ptr_set_t;
static callback_ptr_set_t callbacks;

PyObject*
PyBloomberg_registerstatushandler(PyObject* self, PyObject* args)
{
	PyObject* Callback = NULL;

	if (!PyArg_ParseTuple(args, "O", &Callback))
		return NULL;

	try
	{
		callback_t* callback = new callback_t(Callback, false);
		bloomberg->registerStatusHandler(callback);
		callbacks.insert(callback);
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

PyObject*
PyBloomberg_unregisterstatushandler(PyObject* self, PyObject* args)
{
	PyObject *Callback = NULL;
	if (!PyArg_ParseTuple(args, "O", &Callback))
	{
		PyErr_SetString(PyBloombergError, "invalid args");
		return NULL;
	}

	if (Callback == NULL)
	{
		PyErr_SetString(PyBloombergError, "callback not specified");
		return NULL;
	}

	try
	{
		callback_ptr_set_t::iterator i_callback(callbacks.begin());
		while (i_callback != callbacks.end())
		{
			if ((*i_callback)->getCallback() == Callback)
				break;
			++i_callback;
		}

		if (i_callback == callbacks.end())
			throw "callback not found";

		Bloomberg::CBloombergApi::CBBStatusHandler::CClientCallback* callback = *i_callback;
		bloomberg->unregisterStatusHandler(callback);

		callbacks.erase(i_callback);
		delete callback;

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
