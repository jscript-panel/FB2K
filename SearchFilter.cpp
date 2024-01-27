#include "stdafx.hpp"
#include "SearchFilter.hpp"

metadb_handle_list SearchFilter::get_library_items(wil::zwstring_view query)
{
	metadb_handle_list items;
	library_manager::get()->get_all_items(items);
	if (query.empty()) return items;

	auto filter = get_filter(query);
	if (filter.is_empty()) return items;

	if (Fb::is_v1()) return get_items_legacy(filter, items);

	auto index = search_index_manager::get()->get_library_index();
	return get_items_from_index(filter, index);
}

metadb_handle_list SearchFilter::get_items(const search_filter_v2::ptr& filter, metadb_handle_list_cref handles)
{
	if (Fb::is_v1()) return get_items_legacy(filter, handles);

	auto index = search_index_manager::get()->create_index(handles, nullptr);
	return get_items_from_index(filter, index);
}

metadb_handle_list SearchFilter::get_items_from_index(const search_filter_v2::ptr& filter, const search_index::ptr& index)
{
	auto arr = index->search(filter, nullptr, 0, fb2k::noAbort);
	return arr->as_list_of<metadb_handle>();
}

metadb_handle_list SearchFilter::get_items_legacy(const search_filter_v2::ptr& filter, metadb_handle_list_cref handles)
{
	metadb_handle_list items(handles);
	auto mask = pfc_array<bool>(items.get_count());
	filter->test_multi(items, mask.get_ptr());
	items.filter_mask(mask.get_ptr());
	return items;
}

search_filter_v2::ptr SearchFilter::get_filter(wil::zwstring_view query)
{
	const string8 uquery = from_wide(query);
	search_filter_v2::ptr filter;

	try
	{
		filter = search_filter_manager_v2::get()->create_ex(uquery, fb2k::service_new<completion_notify_dummy>(), search_filter_manager_v2::KFlagSuppressNotify);
	}
	catch (...) {}

	return filter;
}
