#include "stdafx.h"

#define BB_USING_DLL
#include <bbapi.h>
#include <bbunix.h>

#include <BloombergApi/bbutils.h>
#include <BloombergApi/BloombergApi.h>

extern PyObject *PyBloombergError;
extern Bloomberg::CBloombergApi *bloomberg;

PyObject*
PyBloomberg_dispatchloop(PyObject* self, PyObject* args)
{
	int continuous;
	double timeout;
	if (!PyArg_ParseTuple(args, "id", &continuous, &timeout))
	{
		PyErr_SetString(PyBloombergError, "invalid args");
		return NULL;
	}

	if (timeout < 0)
	{
		PyErr_SetString(PyBloombergError, "timeout may not be negative");
		return NULL;
	}

	struct timeval t;
	t.tv_sec = static_cast<int>(::floor(timeout));
	t.tv_usec = static_cast<int>(::floor(timeout * 1e6)) % 1000000;

	// This never returns
	try
	{
		bloomberg->dispatchLoop(continuous != 0, t);
	}
	catch (std::runtime_error& e)
	{
		PyErr_SetString(PyBloombergError, e.what());
		return NULL;
	}

	Py_INCREF(Py_None);
	return Py_None;
}

PyObject*
PyBloomberg_dispatchevent(PyObject* self, PyObject* args)
{
	int fd;
	if (!PyArg_ParseTuple(args, "i", &fd))
	{
		PyErr_SetString(PyBloombergError, "invalid args");
		return NULL;
	}

	bloomberg->dispatchReadEvent(fd);

	Py_INCREF(Py_None);
	return Py_None;
}
