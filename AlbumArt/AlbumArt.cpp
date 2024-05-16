#include "stdafx.hpp"

AlbumArt::AlbumArt(const metadb_handle_ptr& handle, size_t id, bool want_stub) : m_handle(handle), m_guid(AlbumArtStatic::get_guid(id)), m_api(album_art_manager_v2::get())
{
	if (try_now_playing()) return;
	if (try_normal()) return;
	if (want_stub) try_stub();
}

AlbumArt::AlbumArt(size_t id) : m_guid(AlbumArtStatic::get_guid(id)), m_api(album_art_manager_v2::get())
{
	try_stub();
}

IJSImage* AlbumArt::to_image(uint32_t max_size)
{
	wil::com_ptr_t<IWICBitmap> bitmap;
	if FAILED(AlbumArtStatic::to_bitmap(m_data, bitmap)) return nullptr;
	if FAILED(js::fit_to(max_size, bitmap)) return nullptr;
	return new ComObject<JSImage>(bitmap, m_path);
}

bool AlbumArt::try_normal()
{
	auto handles = js::pfc_list(m_handle);
	auto guids = js::pfc_list(m_guid);

	try
	{
		auto instance = m_api->open(handles, guids, fb2k::noAbort);
		m_data = instance->query(m_guid, fb2k::noAbort);
		auto paths = instance->query_paths(m_guid, fb2k::noAbort);
		set_path(paths);
		return true;
	}
	catch (...) {}

	return false;
}

bool AlbumArt::try_now_playing()
{
	const std::string path = m_handle->get_path();
	if (m_guid == album_art_ids::cover_front && core_api::is_main_thread() && !path.starts_with("file://") && path == Fb::get_now_playing_path())
	{
		auto info = now_playing_album_art_notify_manager_v2::get()->current_v2();
		if (info)
		{
			m_data = info.data;
			set_path(info.paths);
			return true;
		}
	}

	return false;
}

void AlbumArt::set_path(const album_art_path_list::ptr& paths)
{
	if (paths.is_valid() && paths->get_count() > 0)
	{
		m_path = js::wdisplay_path(paths->get_path(0));
	}
}

void AlbumArt::show_viewer()
{
	if (m_data.is_valid())
	{
		fb2k::imageViewer::get()->show(Fb::wnd(), m_data);
	}
	else
	{
		const auto name = album_art_ids::capitalized_name_of(m_guid);
		const auto msg = fmt::format("{} not found.", name);
		Component::popup(msg);
	}
}

void AlbumArt::try_stub()
{
	try
	{
		auto instance = m_api->open_stub(fb2k::noAbort);
		m_data = instance->query(m_guid, fb2k::noAbort);
		auto paths = instance->query_paths(m_guid, fb2k::noAbort);
		set_path(paths);
	}
	catch (...) {}
}
