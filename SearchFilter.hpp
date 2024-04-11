#pragma once

class SearchFilter
{
public:
	static metadb_handle_list get_library_items(wil::zwstring_view query);
	static metadb_handle_list get_items(const search_filter_v2::ptr& filter, metadb_handle_list_cref handles);
	static metadb_handle_list get_playlist_items(size_t playlistIndex, wil::zwstring_view query);
	static search_filter_v2::ptr get_filter(wil::zwstring_view query);

private:
	static metadb_handle_list get_items_from_index(const search_filter_v2::ptr& filter, const search_index::ptr& index);
	static metadb_handle_list get_items_legacy(const search_filter_v2::ptr& filter, metadb_handle_list_cref handles);
};
