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

/// <summary>
/// Close the connection to Bloomberg
/// </summary>
PyObject *
PyBloomberg_close(PyObject* self, PyObject* args)
{
	if (!bloomberg->Close())
	{
		PyErr_SetString(PyBloombergError, "Failed to close the Bloomberg connection. ");
		return NULL;
	}

	Py_INCREF(Py_None);
	return Py_None;
}
