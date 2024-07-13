#pragma once

class SearchFilter
{
public:
	static metadb_handle_list get_library_items(std::wstring_view query);
	static metadb_handle_list get_items(const search_filter_v2::ptr& filter, const search_index::ptr& index);
	static metadb_handle_list get_items(const search_filter_v2::ptr& filter, metadb_handle_list handles);
	static metadb_handle_list get_items(const search_filter_v2::ptr& filter, size_t playlistIndex);
	static search_filter_v2::ptr get_filter(std::wstring_view query);
};
