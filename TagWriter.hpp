#pragma once

class TagWriter
{
public:
	TagWriter(metadb_handle_list_cref handles);

	HRESULT from_json_array(JSON& arr);
	HRESULT from_json_object(JSON& obj);

private:
	static uint32_t get_flags();

	metadb_handle_list m_handles;
};
