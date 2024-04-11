#include "stdafx.hpp"
#include "SearchFilter.hpp"

metadb_handle_list SearchFilter::get_library_items(wil::zwstring_view query)
{
	metadb_handle_list items;
	library_manager::get()->get_all_items(items);
	if (query.empty()) return items;

	auto filter = get_filter(query);
	if (filter.is_empty()) return items;

	return get_items(filter, items);
}

metadb_handle_list SearchFilter::get_items(const search_filter_v2::ptr& filter, metadb_handle_list_cref handles)
{
	metadb_handle_list items(handles);
	auto mask = js::pfc_array<bool>(items.get_count());
	filter->test_multi(items, mask.get_ptr());
	items.filter_mask(mask.get_ptr());
	return items;
}

search_filter_v2::ptr SearchFilter::get_filter(wil::zwstring_view query)
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
