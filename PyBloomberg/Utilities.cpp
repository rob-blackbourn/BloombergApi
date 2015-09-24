#include "stdafx.h"

#define BB_USING_DLL
#include <bbapi.h>
#include <bbunix.h>

#include <BloombergApi/bbutils.h>
#include <BloombergApi/BloombergApi.h>

extern PyObject *PyBloombergError;
extern Bloomberg::CBloombergApi *bloomberg;

PyObject*
PyBloomberg_pricetodouble(PyObject* self, PyObject* args)
{
	const char* s;
	if (!PyArg_ParseTuple(args, "s", &s))
	{
		PyErr_SetString(PyBloombergError, "invalid args");
		return NULL;
	}

	return PyFloat_FromDouble(bb_pricetodouble((unsigned char*) s));
}
