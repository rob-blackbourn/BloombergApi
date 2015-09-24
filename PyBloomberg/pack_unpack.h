#pragma once

/*
 * Unpack an override which is a sequence of two items; an integer field id, and a string value.
 */
inline
std::pair<Bloomberg::fieldid_t,std::string>
unpack_override(PyObject* Override)
{
	if (PySequence_Check(Override) != 1)
		throw "unpack_override: expected a sequence";

	if (PySequence_Size(Override) != 2)
		throw "unpack_override: expected a sequence of two items";

	PyObject* FieldId = PySequence_GetItem(Override, 0);
	if (FieldId == NULL)
		throw "unpack_override: unable to get first element of sequence";

	if (PyObject_TypeCheck(FieldId, &PyInt_Type) != 1)
	{
		Py_DECREF(FieldId);
		throw "unpack_override: first argument of override must always be an int";
	}

	long field_id = PyInt_AsLong(FieldId);
	Py_DECREF(FieldId);

	PyObject* Value = PySequence_GetItem(Override, 1);
	if (Value == NULL)
		throw "unpack_override: unable to get second item of sequence";

	if (PyObject_TypeCheck(Value, &PyString_Type) != 1)
	{
		Py_DECREF(Value);
		throw "unpack_override: expected the second item of the sequence to be a string";
	}

	std::string value = PyString_AsString(Value);
	Py_DECREF(Value);

	return std::pair<Bloomberg::fieldid_t,std::string>(field_id, value);
}

/*
 * Unpack a sequence of overrides.
 */
inline
std::set< std::pair<Bloomberg::fieldid_t,std::string> >
unpack_overrides(PyObject* Overrides)
{
	// Argument should be a sequence
	if (PySequence_Check(Overrides) != 1)
		throw "unpack_overrides: expected a sequence";

	// Go through each item in the sequence
	std::set< std::pair<Bloomberg::fieldid_t,std::string> > overrides;
	for (long i = 0; i < PySequence_Size(Overrides); ++i)
	{
		// Get the override
		PyObject* Override = PySequence_GetItem(Overrides, i);
		if (Override == NULL)
			throw "unpack_overrides: unable to get sequence item";

		// Attempt to unpack the override
		try
		{
			overrides.insert(unpack_override(Override));
		}
		catch (const char* err)
		{
			Py_DECREF(Override);
			throw err;
		}

		Py_DECREF(Override);
	}

	return overrides;
}

/*
 * Unpack a ticker which is a sequence of two items; a string name, and an integer ticker type.
 */
inline
Bloomberg::secdesc_t
unpack_ticker(PyObject* Ticker)
{

	if (PySequence_Check(Ticker) != 1)
		throw "unpack_ticker: expected a sequence";

	// The sequence should have two elements
	if (PySequence_Size(Ticker) != 2)
		throw "unpack_ticker: expected a sequence of two items";

	// Get the first item
	PyObject* Name = PySequence_GetItem(Ticker, 0);
	if (Name == NULL)
		throw "unpack_ticker: unable to get first element of sequence";

	// Check that the ticker is a string
	if (PyObject_TypeCheck(Name, &PyString_Type) != 1)
	{
		Py_DECREF(Name);
		throw "unpack_ticker: expected the first item of the sequence to be a string";
	}

	// Get the string and discard the object.
	std::string name = PyString_AsString(Name);
	Py_DECREF(Name);

	// Get the second item
	PyObject* Type = PySequence_GetItem(Ticker, 1);
	if (Type == NULL)
		throw "unpack_ticker: unable to get second element of sequence";

	// Check that the ticker type is an integer.
	if (PyObject_TypeCheck(Type, &PyInt_Type) != 1)
	{
		Py_DECREF(Type);
		throw "unpack_ticker: first argument of override must always be an int";
	}

	// Get the int and discard the object
	long type = PyInt_AsLong(Type);
	Py_DECREF(Type);

	return Bloomberg::secdesc_t(name, static_cast<Bloomberg::BB_TICKTYPE>(type));
}

/*
 * Unpack a sequence of tickers
 */
inline
std::set< Bloomberg::secdesc_t>
unpack_tickers(PyObject* Tickers)
{
	// Argument should be a sequence
	if (PySequence_Check(Tickers) != 1)
		throw "unpack_tickers: expected a sequence";

	// Go through each item in the sequence
	std::set<Bloomberg::secdesc_t> tickers;
	for (long i = 0; i < PySequence_Size(Tickers); ++i)
	{
		// Get the override
		PyObject* Ticker = PySequence_GetItem(Tickers, i);
		if (Ticker == NULL)
			throw "unpack_tickers: unable to get sequence item";

		// Attempt to unpack the ticker
		try
		{
			tickers.insert(unpack_ticker(Ticker));
		}
		catch (const char* err)
		{
			Py_DECREF(Ticker);
			throw err;
		}

		Py_DECREF(Ticker);
	}

	return tickers;
}

/*
 * Unpack a sequence of string security names
 */
inline
std::set<Bloomberg::secname_t>
unpack_secs(PyObject* Secs)
{
	if (PySequence_Check(Secs) != 1)
		throw "unpack_secs: expected a sequence";

	std::set<Bloomberg::secname_t> secs;
	for (long i = 0; i < PySequence_Size(Secs); ++i)
	{
		PyObject* Sec = PySequence_GetItem(Secs, i);
		if (Sec == NULL)
			throw "unpack_secs: unable to get sequence item";

		if (PyObject_TypeCheck(Sec, &PyString_Type) == 0)
		{
			Py_DECREF(Sec);
			throw "unpack_secs: expected sequence item to be a string";
		}

		secs.insert(PyString_AsString(Sec));

		Py_DECREF(Sec);
	}

	return secs;
}

/*
 * Unpack a sequence of integer field id's.
 */
inline
std::set<Bloomberg::fieldid_t>
unpack_fields(PyObject* Fields)
{
	if (PySequence_Check(Fields) != 1)
		throw "unpack_fields: expected a sequence";

	std::set<Bloomberg::fieldid_t> fields;
	for (long i = 0; i < PySequence_Size(Fields); ++i)
	{
		PyObject* Field = PySequence_GetItem(Fields, i);
		if (Field == NULL)
			throw "unpack_fields: unable to get sequence item";

		if (PyObject_TypeCheck(Field, &PyInt_Type) == 0)
		{
			Py_DECREF(Field);
			throw "unpack_fields: expected sequence item to be an int";
		}

		fields.insert(PyInt_AsLong(Field));

		Py_DECREF(Field);
	}

	return fields;
}

/*
 * Unpack a sequence of integer categories.
 */
inline
std::set<Bloomberg::moncat_t>
unpack_moncat(PyObject* Categories)
{
	if (PySequence_Check(Categories) != 1)
		throw "unpack_moncat: expected a sequence";

	std::set<Bloomberg::moncat_t> categories;
	for (long i = 0; i < PySequence_Size(Categories); ++i)
	{
		PyObject* Category = PySequence_GetItem(Categories, i);
		if (Categories == NULL)
			throw "unpack_moncat: unable to get sequence item";

		if (PyObject_TypeCheck(Category, &PyInt_Type) == 0)
		{
			Py_DECREF(Category);
			throw "unpack_moncat: expected sequence item to be an int";
		}

		categories.insert(PyInt_AsLong(Category));

		Py_DECREF(Category);
	}

	return categories;
}

// Data
inline
PyObject* 
pack_sec_result_set(const Bloomberg::CBloombergApi::CBBDataRequest::retval_map_t& data_map)
{
	PyObject* dict = PyDict_New();

	for (Bloomberg::CBloombergApi::CBBDataRequest::retval_map_t::const_iterator i_data_map(data_map.begin()); i_data_map != data_map.end(); ++i_data_map)
	{
		PyDict_SetItem(
			dict,
			PyInt_FromLong(i_data_map->first),
			PyString_FromString(i_data_map->second.c_str()));
	}

	return dict;
}

inline
PyObject* 
pack_result_set(const Bloomberg::CBloombergApi::CBBDataRequest::sec_retval_map_t& data_cache)
{
	PyObject* dict = PyDict_New();

	// For each security in the map
	for (
		Bloomberg::CBloombergApi::CBBDataRequest::sec_retval_map_t::const_iterator i_sec(data_cache.begin());
		i_sec != data_cache.end();
		++i_sec)
	{
		const Bloomberg::secdesc_t& sec = i_sec->first;
		const Bloomberg::CBloombergApi::CBBDataRequest::retval_map_t& data_map = i_sec->second;

		PyDict_SetItem(
			dict,
			PyString_FromString(sec.first.c_str()),
			pack_sec_result_set(data_map));
	}

	return dict;
}

// Header
inline
PyObject* 
pack_sec_result_set(const Bloomberg::CBloombergApi::CBBHeaderRequest::retval_map_t& data_map)
{
	PyObject* dict = PyDict_New();

	for (Bloomberg::CBloombergApi::CBBHeaderRequest::retval_map_t::const_iterator i_data_map(data_map.begin()); i_data_map != data_map.end(); ++i_data_map)
	{
		PyDict_SetItem(
			dict,
			PyString_FromString(i_data_map->first.c_str()),
			PyString_FromString(i_data_map->second.c_str()));
	}

	return dict;
}

// Header
inline
PyObject* 
pack_result_set(const Bloomberg::CBloombergApi::CBBHeaderRequest::sec_retval_map_t& data_cache)
{
	PyObject* dict = PyDict_New();

	// For each security in the map
	for (
		Bloomberg::CBloombergApi::CBBHeaderRequest::sec_retval_map_t::const_iterator i_sec(data_cache.begin());
		i_sec != data_cache.end();
		++i_sec)
	{
		const Bloomberg::secdesc_t& sec = i_sec->first;
		const Bloomberg::CBloombergApi::CBBHeaderRequest::retval_map_t& data_map = i_sec->second;

		PyDict_SetItem(
			dict,
			PyString_FromString(sec.first.c_str()),
			pack_sec_result_set(data_map));
	}

	return dict;
}

// HistData
inline
PyObject*
pack_sec_result_vec(const Bloomberg::CBloombergApi::CBBHistDataRequest::vec_retval_t& data_vec)
{
	PyObject* dict = PyDict_New();

	size_t i = 0;
	for (
		Bloomberg::CBloombergApi::CBBHistDataRequest::vec_retval_t::const_iterator i_data(data_vec.begin());
		i_data != data_vec.end();
		++i_data)
	{
		Bloomberg::CBloombergApi::CBBHistDataRequest::longdate_t date = i_data->first;
		double value = i_data->second;

		PyDict_SetItem(
			dict,
			PyInt_FromLong(date),
			PyFloat_FromDouble(value));
	}

	return dict;
}

inline
PyObject* 
pack_sec_result_set(const Bloomberg::CBloombergApi::CBBHistDataRequest::retval_map_t& data_map)
{
	PyObject* dict = PyDict_New();

	for (
		Bloomberg::CBloombergApi::CBBHistDataRequest::retval_map_t::const_iterator i_data_map(data_map.begin());
		i_data_map != data_map.end();
		++i_data_map)
	{
		const Bloomberg::fieldid_t& fieldid = i_data_map->first;
		const Bloomberg::CBloombergApi::CBBHistDataRequest::vec_retval_t& data_vec = i_data_map->second;

		PyDict_SetItem(
			dict,
			PyInt_FromLong(fieldid),
			pack_sec_result_vec(data_vec));
	}

	return dict;
}

// Data
inline
PyObject* 
pack_result_set(const Bloomberg::CBloombergApi::CBBHistDataRequest::sec_retval_map_t& data_cache)
{
	PyObject* dict = PyDict_New();

	// For each security in the map
	for (
		Bloomberg::CBloombergApi::CBBHistDataRequest::sec_retval_map_t::const_iterator i_sec(data_cache.begin());
		i_sec != data_cache.end();
		++i_sec)
	{
		const Bloomberg::secdesc_t& sec = i_sec->first;
		const Bloomberg::CBloombergApi::CBBHistDataRequest::retval_map_t& data_map = i_sec->second;

		PyDict_SetItem(
			dict,
			PyString_FromString(sec.first.c_str()),
			pack_sec_result_set(data_map));
	}

	return dict;
}
