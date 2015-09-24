#include "stdafx.h"

#include "stdafx.h"

#define BB_USING_DLL
#define MAIN
#include <bbapi.h>
#include <bbunix.h>

#include <BloombergApi/bbutils.h>
#include <BloombergApi/DataDictionary.h>
#include <BloombergApi/BloombergApi.h>

BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
					 )
{
    return TRUE;
}

#include "PyClientCallback.h"
#include "pack_unpack.h"

PyObject *PyBloombergError;
Bloomberg::CBloombergApi *bloomberg;

extern PyObject *PyBloomberg_open(PyObject* self, PyObject* args);
extern PyObject *PyBloomberg_close(PyObject* self, PyObject* args);
extern PyObject *PyBloomberg_registerstatushandler(PyObject* self, PyObject* args);
extern PyObject *PyBloomberg_unregisterstatushandler(PyObject* self, PyObject* args);
extern PyObject *PyBloomberg_getdata(PyObject* self, PyObject* args);
extern PyObject *PyBloomberg_getheader(PyObject* self, PyObject* args);
extern PyObject *PyBloomberg_gethistdata(PyObject* self, PyObject* args);
extern PyObject *PyBloomberg_getsocket(PyObject* self, PyObject* args);
extern PyObject *PyBloomberg_requestdata(PyObject* self, PyObject* args);
extern PyObject *PyBloomberg_requestheader(PyObject* self, PyObject* args);
extern PyObject *PyBloomberg_requestticks(PyObject* self, PyObject* args);
extern PyObject *PyBloomberg_derequestticks(PyObject* self, PyObject* args);
extern PyObject *PyBloomberg_isqueueempty(PyObject* self, PyObject* args);
extern PyObject *PyBloomberg_dispatchloop(PyObject* self, PyObject* args);
extern PyObject *PyBloomberg_dispatchevent(PyObject* self, PyObject* args);
extern PyObject *PyBloomberg_pricetodouble(PyObject* self, PyObject* args);
extern PyObject *PyBloomberg_enablefeature(PyObject* self, PyObject* args);

static PyMethodDef PyBloombergMethods[] = {
	{"open",					PyBloomberg_open,					METH_VARARGS,	"Open a Bloomberg connection."},
    {"close",					PyBloomberg_close,					METH_VARARGS,	"Close a Bloomberg connection."},
    {"enablefeature",			PyBloomberg_enablefeature,			METH_VARARGS,	"Enable a feature."},
    {"registerstatushandler",	PyBloomberg_registerstatushandler,	METH_VARARGS,	"Register a status handler."},
    {"unregisterstatushandler",	PyBloomberg_unregisterstatushandler,METH_VARARGS,	"Unregister a status handler."},
    {"getdata",					PyBloomberg_getdata,				METH_VARARGS,	"Get data from Bloomberg."},
    {"gethistdata",				PyBloomberg_gethistdata,			METH_VARARGS,	"Get historical data from Bloomberg."},
    {"getheader",				PyBloomberg_getheader,				METH_VARARGS,	"Get header from Bloomberg."},
    {"getsocket",				PyBloomberg_getsocket,				METH_NOARGS,	"Get the file descriptor."},
    {"requestdata",				PyBloomberg_requestdata,			METH_VARARGS,	"Request data from Bloomberg."},
    {"requestheader",			PyBloomberg_requestheader,			METH_VARARGS,	"Request headers from Bloomberg."},
    {"requestticks",			PyBloomberg_requestticks,			METH_VARARGS,	"Request ticks from Bloomberg."},
    {"derequestticks",			PyBloomberg_derequestticks,			METH_VARARGS,	"De-request ticks from Bloomberg."},
    {"isqueueempty",			PyBloomberg_isqueueempty,			METH_NOARGS,	"Is the message queue empty?"},
    {"dispatchloop",			PyBloomberg_dispatchloop,			METH_VARARGS,	"Run the dispatch loop - never returns."},
    {"dispatchevent",			PyBloomberg_dispatchevent,			METH_VARARGS,	"Dispatch an event."},
    {"pricetodouble",			PyBloomberg_pricetodouble,			METH_VARARGS,	"Convert a string price to a double."},
    {NULL, NULL, 0, NULL}        /* Sentinel */
};

static struct { char* name; int id; } IntVars[] = {
	{"BB_IDX_TCM",				BB_IDX_TCM},
	{"BB_IDX_CUSIP",			BB_IDX_CUSIP},
	{"BB_IDX_EUROCLEAR",		BB_IDX_EUROCLEAR},
	{"BB_IDX_ISMA",				BB_IDX_ISMA},
	{"BB_IDX_SEDOL1",			BB_IDX_SEDOL1},
	{"BB_IDX_SEDOL2",			BB_IDX_SEDOL2},
	{"BB_IDX_CEDEL",			BB_IDX_CEDEL},
	{"BB_IDX_WPK",				BB_IDX_WPK},
	{"BB_IDX_RGA",				BB_IDX_RGA},
	{"BB_IDX_ISIN",				BB_IDX_ISIN},
	{"BB_IDX_DUTCH",			BB_IDX_DUTCH},
	{"BB_IDX_VALOREN",			BB_IDX_VALOREN},
	{"BB_IDX_FRENCH",			BB_IDX_FRENCH},
	{"BB_IDX_COMMON_NUMBER",	BB_IDX_COMMON_NUMBER},
	{"BB_IDX_CUSIP8",			BB_IDX_CUSIP8},
	{"BB_IDX_JAPAN",			BB_IDX_JAPAN},
	{"BB_IDX_BELGIAN",			BB_IDX_BELGIAN},
	{"BB_IDX_DANISH",			BB_IDX_DANISH},
	{"BB_IDX_AUSTRIAN",			BB_IDX_AUSTRIAN},
	{"BB_IDX_LUXEMBOURG",		BB_IDX_LUXEMBOURG},
	{"BB_IDX_SWEDEN",			BB_IDX_SWEDEN},
	{"BB_IDX_NORWAY",			BB_IDX_NORWAY},
	{"BB_IDX_ITALY",			BB_IDX_ITALY},
	{"BB_IDX_JAPAN_COMPANY",	BB_IDX_JAPAN_COMPANY},
	{"BB_IDX_SPAIN",			BB_IDX_SPAIN},
	{"BB_IDX_FIRMID",			BB_IDX_FIRMID},
	{"BB_IDX_MISC_DOMESTIC",	BB_IDX_MISC_DOMESTIC},
	{"BB_IDX_AIBD",				BB_IDX_AIBD},
	{"BB_IDX_CINS",				BB_IDX_CINS},
	{"BB_IDX_TICKERX",			BB_IDX_TICKERX},
	{"BB_IDX_CUSIPX",			BB_IDX_CUSIPX},
	{"BB_IDX_SCM",				BB_IDX_SCM},
	{"BB_IDX_TICKER",			BB_IDX_TICKER},
	{"BB_IDX_STCM",				BB_IDX_STCM},
	{"BB_IDX_TICKERDIV",		BB_IDX_TICKERDIV},
	{"BB_IDX_CLIENTPORT",		BB_IDX_CLIENTPORT},
	{"BB_IDX_TCA",				BB_IDX_TCA},
	{"BB_IDX_IRISH_SEDOL",		BB_IDX_IRISH_SEDOL},
	{"BB_IDX_CATS",				BB_IDX_CATS},
	{"BB_IDX_OBJECT_ID",		BB_IDX_OBJECT_ID},

	{"BHeaderDEFAULTSEARCH",			BHeaderDEFAULTSEARCH},
	{"BHeaderTODAYONLY",				BHeaderTODAYONLY},
	{"BHeaderENHANCEDSEARCH",			BHeaderENHANCEDSEARCH},
	{"BHeaderGARYPSEARCH",				BHeaderGARYPSEARCH},
	{"BHeaderGETSECINFO_ONLY",			BHeaderGETSECINFO_ONLY},
	{"BHeaderBYOBJECTID",				BHeaderBYOBJECTID},

	{"BHistoryxPERIODMASK",				BHistoryxPERIODMASK},
	{"BHistoryxDEFAULT",				BHistoryxDEFAULT},
	{"BHistoryxDAILY",					BHistoryxDAILY},
	{"BHistoryxWEEKLY",					BHistoryxWEEKLY},
	{"BHistoryxMONTHLY",				BHistoryxMONTHLY},
	{"BHistoryxQUARTERLY",				BHistoryxQUARTERLY},
	{"BHistoryxYEARLY",					BHistoryxYEARLY},
	{"BHistoryxQUOTEMASK",				BHistoryxQUOTEMASK},
	{"BHistoryxYIELD",					BHistoryxYIELD},
	{"BHistoryxPRICE",					BHistoryxPRICE},
	{"BHistoryxFUNCMASK",				BHistoryxFUNCMASK},
	{"BHistoryxCLOSE",					BHistoryxCLOSE},
	{"BHistoryxGPA",					BHistoryxGPA},
	{"BHistoryxSHOWNOAXMASK",			BHistoryxSHOWNOAXMASK},
	{"BHistoryxOMITNOAX",				BHistoryxOMITNOAX},
	{"BHistoryxINCLUDENOAX",			BHistoryxINCLUDENOAX},
	{"BHistoryxALLCALDAYS",				BHistoryxALLCALDAYS},
	{"BHistoryxNOAXFORMMASK",			BHistoryxNOAXFORMMASK},
	{"BHistoryxBBHANDLES",				BHistoryxBBHANDLES},
	{"BHistoryxSHOWPREVIOUS",			BHistoryxSHOWPREVIOUS},
	{"BHistoryxNONUM",					BHistoryxNONUM},
	{"BHistoryxYield2",					BHistoryxYield2},

	{"FeatureCC",						FeatureCC},
	{"FeatureALL_PRICE_ETC",			FeatureALL_PRICE_ETC},
	{"FeatureALL_PRICE_COND_CODE",		FeatureALL_PRICE_COND_CODE},

	{"bCategoryCORE",					bCategoryCORE},
	{"bCategoryMARKET_DEPTH",			bCategoryMARKET_DEPTH},
	{"bCategoryGREEKS",					bCategoryGREEKS},
#ifdef BLOOMBERGAPI_SERVER
	{"bCategoryCONDITION_CODES",		bCategoryCONDITION_CODES},
#endif
	{"bCategoryNUM_TICKMNTR_CATEGORIES",bCategoryNUM_TICKMNTR_CATEGORIES},

	{"BB_FMT_STRING",		1},
	{"BB_FMT_NUMERIC",		2},
	{"BB_FMT_PRICE",		3},
	{"BB_FMT_SECURITY",		4},
	{"BB_FMT_DATE",			5},
	{"BB_FMT_TIME",			6},
	{"BB_FMT_DATETIME",		7},
	{"BB_FMT_BULK",			8},
	{"BB_FMT_MONTHYEAR",	9},
	{"BB_FMT_BOOL",			10},
	{"BB_FMT_ISOCCY",		11},

	{"BB_SEC_COMMODITY",		BB_SEC_COMMODITY},
	{"BB_SEC_EQUITY",			BB_SEC_EQUITY},
	{"BB_SEC_MUNICIPAL_BOND",	BB_SEC_MUNICIPAL_BOND},
	{"BB_SEC_PREFERRED_STOCK",	BB_SEC_PREFERRED_STOCK},
	{"BB_SEC_CLIENT_PORTFOLIO",	BB_SEC_CLIENT_PORTFOLIO},
	{"BB_SEC_MONEY_MARKET",		BB_SEC_MONEY_MARKET},
	{"BB_SEC_GOVERNMENT",		BB_SEC_GOVERNMENT},
	{"BB_SEC_CORPORATE_BOND",	BB_SEC_CORPORATE_BOND},
	{"BB_SEC_INDEX",			BB_SEC_INDEX},
	{"BB_SEC_CURRENCY",			BB_SEC_CURRENCY},
	{"BB_SEC_MORTGAGE",			BB_SEC_MORTGAGE},

	{"Comdty",	Comdty},
	{"Equity",	Equity},
	{"Muni",	Muni},
	{"Pfd",		Pfd},
	{"Client",	Client},
	{"MMkt",	MMkt},
	{"Govt",	Govt},
	{"Corp",	Corp},
	{"Index",	Index},
	{"Curncy",	Curncy},
	{"Mtge",	Mtge}

};

extern "C" {
	__declspec(dllexport) void initPyBloomberg(void)
	{
		PyObject* module = Py_InitModule("PyBloomberg", PyBloombergMethods);

		PyObject* d = PyModule_GetDict(module);
		PyBloombergError = PyErr_NewException("PyBloomberg.error", NULL, NULL);
		PyDict_SetItemString(d, "error", PyBloombergError);

		// Setup all the integer constants
		for (size_t i = 0; i < sizeof(IntVars) / sizeof(IntVars[0]); ++i)
			PyModule_AddIntConstant(module, IntVars[i].name, IntVars[i].id);

		bloomberg = new Bloomberg::CBloombergApi();
	}
}
