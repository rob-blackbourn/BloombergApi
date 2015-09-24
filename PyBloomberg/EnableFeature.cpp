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

PyObject *
PyBloomberg_enablefeature(PyObject* self, PyObject* args)
{
	int Feature;
	if (!PyArg_ParseTuple(args, "i", &Feature))
		return NULL;

	if (!bloomberg->EnableFeature(Feature))
	{
		PyErr_SetString(PyBloombergError, "Failed to enable the feature. ");
		return NULL;
	}

	Py_INCREF(Py_None);
	return Py_None;
}
