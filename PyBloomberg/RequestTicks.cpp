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

typedef std::map<CPyClientCallback_Tick*,Bloomberg::CBloombergApi::set_secdesc_t> py_callback_map_t;
static py_callback_map_t callbacks;

PyObject*
PyBloomberg_requestticks(PyObject* self, PyObject* args)
{
	PyObject* Tickers = NULL, *Categories, *Callback = NULL;
	if (!PyArg_ParseTuple(args, "OOO", &Tickers, &Categories, &Callback))
	{
		PyErr_SetString(PyBloombergError, "invalid args");
		return NULL;
	}

	if (Tickers == NULL)
	{
		PyErr_SetString(PyBloombergError, "tickers not specified");
		return NULL;
	}

	if (Categories == NULL)
	{
		PyErr_SetString(PyBloombergError, "categories not specified");
		return NULL;
	}

	if (Callback == NULL)
	{
		PyErr_SetString(PyBloombergError, "callback not specified");
		return NULL;
	}

	try
	{
		Bloomberg::CBloombergApi::set_secdesc_t tickers = unpack_tickers(Tickers);
		Bloomberg::CBloombergApi::set_monid_t categories = unpack_moncat(Categories);
		CPyClientCallback_Tick* callback = new CPyClientCallback_Tick(Callback, false);
		bloomberg->requestTickMonitor(tickers, categories, callback);
		callbacks[callback] = tickers;
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
PyBloomberg_derequestticks(PyObject* self, PyObject* args)
{
	PyObject *CallbackOrTickers = NULL;
	if (!PyArg_ParseTuple(args, "O", &CallbackOrTickers))
	{
		PyErr_SetString(PyBloombergError, "invalid args");
		return NULL;
	}

	if (CallbackOrTickers == NULL)
	{
		PyErr_SetString(PyBloombergError, "callback or tickers not specified");
		return NULL;
	}

	try
	{
		if (PyCallable_Check(CallbackOrTickers) == 1)
		{
			py_callback_map_t::iterator i_callback(callbacks.begin());
			while (i_callback != callbacks.end())
			{
				if (i_callback->first->getCallback() == CallbackOrTickers)
					break;
				++i_callback;
			}

			if (i_callback == callbacks.end())
				throw "callback not found";

			Bloomberg::CBloombergApi::CBBTickMonitor::CClientCallback* callback = i_callback->first;
			bloomberg->derequestTickMonitor(callback);

			callbacks.erase(i_callback);
			delete callback;
		}
		else
		{
			Bloomberg::CBloombergApi::set_secdesc_t tickers = unpack_tickers(CallbackOrTickers);

			// Go through each of the callbacks and look for tickers.
			for(
				py_callback_map_t::iterator i_callback(callbacks.begin());
				i_callback != callbacks.end();
				++i_callback)
			{
				// Go through each of the specified tickers and remove them from the callback
				Bloomberg::CBloombergApi::set_secdesc_t& callback_tickers = i_callback->second;
				for (Bloomberg::CBloombergApi::set_secdesc_t::iterator i_ticker(tickers.begin()); i_ticker != tickers.end(); ++i_ticker)
				{
					Bloomberg::CBloombergApi::set_secdesc_t::iterator i_callback_ticker = callback_tickers.find(*i_ticker);
					if (i_callback_ticker != callback_tickers.end())
						callback_tickers.erase(i_callback_ticker);
				}
			}

			bloomberg->derequestTickMonitor(tickers);

			// Delete any empty callbacks here.
			typedef std::set<CPyClientCallback_Tick*> py_callback_set_t;
			py_callback_set_t deletable_callbacks;
			for (
				py_callback_map_t::iterator i_callback(callbacks.begin());
				i_callback != callbacks.end();
				++i_callback)
			{
				if (i_callback->second.empty())
					deletable_callbacks.insert(i_callback->first);
			}
			for (py_callback_set_t::iterator i_deletable_callback(deletable_callbacks.begin()); i_deletable_callback != deletable_callbacks.end(); ++i_deletable_callback)
			{
				callbacks.erase(*i_deletable_callback);
				delete *i_deletable_callback;
			}

		}

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
