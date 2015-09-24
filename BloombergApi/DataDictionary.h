#pragma once

namespace Bloomberg
{
	class CDataDictionary
	{
	public:
		typedef std::map<fieldid_t,bb_field_info_t> field_info_map_t;
		typedef std::map<std::string,fieldid_t> fieldid_map_t;

	public:
		CDataDictionary(const char* bbfields = "C:\\blp\\API\\bbfields.tbl");

		field_info_map_t& getDictionary() { return m_DataDictionary; }
		const field_info_map_t& getDictionary() const { return m_DataDictionary; }

		const bb_field_info_t& getFieldInfo(fieldid_t fieldid) const;
		const bb_field_info_t& getFieldInfo(const std::string& field_mnemonic) const;

		fieldid_t getFieldId(const std::string& field_mnemonic) const;
		std::string getFieldMnemonic(fieldid_t field_id) const;

	private:
		void load(const char* bbfields);

		field_info_map_t m_DataDictionary;
		fieldid_map_t m_FieldMnemonicMap;
	};

}