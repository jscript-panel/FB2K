#include "stdafx.hpp"
#include "SearchFilter.hpp"

metadb_handle_list SearchFilter::get_library_items(std::wstring_view query)
{
	metadb_handle_list items;
	library_manager::get()->get_all_items(items);
	if (items.get_count() == 0 || query.empty())
		return items;

	auto filter = get_filter(query);
	if (filter.is_empty())
		return items;

	auto index = search_index_manager::get()->get_library_index();
	return get_items(filter, index);
}

metadb_handle_list SearchFilter::get_items(const search_filter_v2::ptr& filter, const search_index::ptr& index)
{
	auto arr = index->search(filter, nullptr, 0, fb2k::noAbort);
	return arr->as_list_of<metadb_handle>();
}

metadb_handle_list SearchFilter::get_items(const search_filter_v2::ptr& filter, metadb_handle_list handles)
{
	// not using search_index_manager::create_index because original order is not preserved
	// do it the old way

	auto mask = js::pfc_array<bool>(handles.get_count());
	filter->test_multi(handles, mask.get_ptr());
	handles.filter_mask(mask.get_ptr());
	return handles;
}

metadb_handle_list SearchFilter::get_items(const search_filter_v2::ptr& filter, size_t playlistIndex)
{
	const auto g = Plman::api()->playlist_get_guid(playlistIndex);
	auto index = search_index_manager::get()->create_playlist_index(g);
	return get_items(filter, index);
}

search_filter_v2::ptr SearchFilter::get_filter(std::wstring_view query)
{
	const string8 uquery = js::from_wide(query);
	search_filter_v2::ptr filter;

	try
	{
		filter = search_filter_manager_v2::get()->create_ex(uquery, fb2k::service_new<completion_notify_dummy>(), search_filter_manager_v2::KFlagSuppressNotify);
	}
	catch (...) {}

	return filter;
}
