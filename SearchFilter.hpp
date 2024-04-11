#pragma once

class SearchFilter
{
public:
	static metadb_handle_list get_library_items(wil::zwstring_view query);
	static metadb_handle_list get_items(const search_filter_v2::ptr& filter, metadb_handle_list_cref handles);
	static search_filter_v2::ptr get_filter(wil::zwstring_view query);
};
