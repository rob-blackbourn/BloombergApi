#include "stdafx.h"

#define BB_USING_DLL
#include <bbapi.h>
#include <bbunix.h>

#include "bbutils.h"
#include "stringutils.h"

#include "DataDictionary.h"

namespace Bloomberg
{
	CDataDictionary::CDataDictionary(const char* bbfields)
	{
		load(bbfields);
	}

	void CDataDictionary::load(const char* bbfields)
	{
		static const size_t LINE_LEN = 2048;

		// Open bbfields.tbl
		std::ifstream is(bbfields);
		char buf[LINE_LEN];
		while (is.good())
		{
			// Read in a line
			is.getline(buf, LINE_LEN);
			if (is.eof())
				break;

			// tokenise the delimited line
			std::vector<std::string> line = split(std::string(buf), '|');
			if (line.size() != 10)
				continue;

			// Unpack the data into the structure.

			bb_field_info_t field_info;
			field_info.FieldCatCode = atoi(line[0].c_str());
			strncpy(field_info.FieldCatName, line[1].c_str(), sizeof(field_info.FieldCatName));
			field_info.FieldSubCode = atoi(line[2].c_str());
			strncpy(field_info.FieldSubName, line[3].c_str(), sizeof(field_info.FieldSubName));
			field_info.FieldId = strtol(line[4].c_str(), NULL, 16);
			strncpy(field_info.FieldDesc, line[5].c_str(), sizeof(field_info.FieldDesc));
			strncpy(field_info.FieldMnemonic, line[6].c_str(), sizeof(field_info.FieldMnemonic));
			field_info.FieldMarket = atoi(line[7].c_str());
			field_info.FieldSource = atoi(line[8].c_str());
			field_info.FieldFormat = atoi(line[9].c_str());

			// Skip obsolete mnemonics
			if (field_info.FieldCatCode == 999)
				continue; // Obsolete Mnemonic.

			// Add the information to the data dictionary
			m_DataDictionary[field_info.FieldId] = field_info;
			m_FieldMnemonicMap[field_info.FieldMnemonic] = field_info.FieldId;
		}
		is.close();
	}

	const bb_field_info_t& CDataDictionary::getFieldInfo(fieldid_t fieldid) const
	{
		field_info_map_t::const_iterator i(m_DataDictionary.find(fieldid));
		if (i == m_DataDictionary.end())
			throw std::runtime_error("unable to find field id");

		return i->second;
	}

	const bb_field_info_t& CDataDictionary::getFieldInfo(const std::string& field_mnemonic) const
	{
		return getFieldInfo(getFieldId(field_mnemonic));
	}

	fieldid_t CDataDictionary::getFieldId(const std::string& field_mnemonic) const
	{
		fieldid_map_t::const_iterator i(m_FieldMnemonicMap.find(field_mnemonic));
		if (i == m_FieldMnemonicMap.end())
			throw std::runtime_error("unable to find field mnemonic");

		return i->second;
	}

	std::string CDataDictionary::getFieldMnemonic(fieldid_t field_id) const
	{
		return getFieldInfo(field_id).FieldMnemonic;
	}

}