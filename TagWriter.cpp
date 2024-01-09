#include "stdafx.hpp"
#include "TagWriter.hpp"

#include "FileInfoFilter.hpp"

TagWriter::TagWriter(metadb_handle_list_cref handles) : m_handles(handles) {}

HRESULT TagWriter::from_json_array(JSON& arr)
{
	std::vector<file_info_impl> infos(m_handles.get_count());

	for (auto&& [info, obj, handle] : std::views::zip(infos, arr, m_handles))
	{
		RETURN_HR_IF(E_INVALIDARG, !obj.is_object() || obj.empty());

		info = handle->get_info_ref()->info();

		for (const auto& [name, values] : obj.items())
		{
			RETURN_HR_IF(E_INVALIDARG, name.empty());

			info.meta_remove_field(name.c_str());

			for (auto&& value : JSONHelper::to_strings(values))
			{
				info.meta_add(name.c_str(), value.c_str());
			}
		}
	}

	auto list = pfc::ptr_list_const_array_t<const file_info, file_info_impl*>(infos.data(), infos.size());
	auto filter = fb2k::service_new<file_info_filter_impl>(m_handles, list);
	metadb_io_v2::get()->update_info_async(m_handles, filter, Fb::wnd(), get_flags(), nullptr);
	return S_OK;
}

HRESULT TagWriter::from_json_object(JSON& obj)
{
	FileInfoFilter::Tags tags;

	for (const auto& [name, value] : obj.items())
	{
		RETURN_HR_IF(E_INVALIDARG, name.empty());

		FileInfoFilter::Tag tag;
		tag.name = name;
		tag.values = JSONHelper::to_strings(value);
		tags.emplace_back(tag);
	}

	auto filter = fb2k::service_new<FileInfoFilter>(tags);
	metadb_io_v2::get()->update_info_async(m_handles, filter, Fb::wnd(), get_flags(), nullptr);
	return S_OK;
}

uint32_t TagWriter::get_flags()
{
	if (Fb::is_v2()) return metadb_io_v2::op_flag_silent;
	return metadb_io_v2::op_flag_delay_ui;
}
