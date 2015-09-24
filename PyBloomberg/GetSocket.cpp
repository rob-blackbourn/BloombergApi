#include "stdafx.h"

#define BB_USING_DLL
#include <bbapi.h>
#include <bbunix.h>

#include <BloombergApi/bbutils.h>
#include <BloombergApi/BloombergApi.h>

extern Bloomberg::CBloombergApi *bloomberg;

PyObject*
PyBloomberg_getsocket(PyObject* self, PyObject* args)
{
	return PyInt_FromLong(bloomberg->getSocket());
}
