#include "stdafx.h"

#define BB_USING_DLL
#include <bbapi.h>
#include <bbunix.h>

#include <BloombergApi/bbutils.h>
#include <BloombergApi/BloombergApi.h>

#define DISABLE_Py_DEBUG
#include <Python.h>

extern PyObject *PyBloombergError;
extern Bloomberg::CBloombergApi *bloomberg;

#if defined(BLOOMBERGAPI_CLIENT)

PyObject *
PyBloomberg_open(PyObject* self, PyObject* args)
{
	int Port = Bloomberg::CBloombergApi::BLP_PORT;
	if (!PyArg_ParseTuple(args, "|i", &Port))
		return NULL;

	if (!bloomberg->Open(Port))
	{
		PyErr_SetString(PyBloombergError, "Failed to open the Bloomberg connection");
		return NULL;
	}

	Py_INCREF(Py_None);
	return Py_None;
}
#elif defined(BLOOMBERGAPI_SERVER)

PyObject *
PyBloomberg_open(PyObject* self, PyObject* args)
{
	char* Server = "127.0.0.1";
	int uuid = -1, userSid = -1, userSidInstance = -1, termSid = -1, termSidInstance = -1;

	int Port = Bloomberg::CBloombergApi::BLP_PORT;
	if (!PyArg_ParseTuple(args, "|siiiiii", &Server, &Port, &uuid, &userSid, &userSidInstance, &termSid, &termSidInstance))
		return NULL;

	if (uuid == -1)
	{
		if (!bloomberg->Open(Server, Port))
		{
			PyErr_SetString(PyBloombergError, "Failed to open the Bloomberg connection");
			return NULL;
		}
	}
	else
	{
		if (!bloomberg->Open(Server, Port, uuid, userSid, userSidInstance, termSid, termSidInstance))
		{
			PyErr_SetString(PyBloombergError, "Failed to open the Bloomberg connection");
			return NULL;
		}
	}

	Py_INCREF(Py_None);
	return Py_None;
}
#else
#error Must define either BLOOMBERGAPI_CLIENT or BLOOMBERGAPI_SERVER
#endif
