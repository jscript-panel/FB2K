#include "stdafx.hpp"

AlbumArt::AlbumArt(const metadb_handle_ptr& handle, size_t id, bool want_stub) : m_handle(handle), m_guid(*guids::art[id]), m_api(album_art_manager_v2::get())
{
	if (try_now_playing()) return;
	if (try_normal()) return;
	if (want_stub) try_stub();
}

AlbumArt::AlbumArt(size_t art_id) : m_guid(*guids::art[art_id]), m_api(album_art_manager_v2::get())
{
	try_stub();
}

AlbumArt::Data AlbumArt::istream_to_data(IStream* stream)
{
	Data data;
	STATSTG sts;

	if SUCCEEDED(stream->Stat(&sts, STATFLAG_DEFAULT))
	{
		const DWORD bytes = sts.cbSize.LowPart;
		std::vector<uint8_t> image_data(bytes);
		ULONG bytes_read{};
		if SUCCEEDED(stream->Read(image_data.data(), bytes, &bytes_read))
		{
			data = album_art_data_impl::g_create(image_data.data(), image_data.size());
		}
	}
	return data;
}

AlbumArt::Data AlbumArt::path_to_data(wil::zwstring_view path)
{
	Data data;
	wil::com_ptr_t<IStream> stream;

	if SUCCEEDED(SHCreateStreamOnFileEx(path.data(), STGM_READ | STGM_SHARE_DENY_WRITE, GENERIC_READ, FALSE, nullptr, &stream))
	{
		data = istream_to_data(stream.get());
	}
	return data;
}

HRESULT AlbumArt::check_id(size_t id)
{
	if (id < guids::art.size()) return S_OK;
	return E_INVALIDARG;
}

IJSImage* AlbumArt::get_embedded(const metadb_handle_ptr& handle, size_t id)
{
	const GUID guid = *guids::art[id];
	const string8 path = handle->get_path();
	album_art_extractor::ptr ptr;

	if (!album_art_extractor::g_get_interface(ptr, path)) return nullptr;
	
	try
	{
		auto instance = ptr->open(nullptr, path, fb2k::noAbort);
		auto data = instance->query(guid, fb2k::noAbort);
		wil::com_ptr_t<IWICBitmap> bitmap;

		if SUCCEEDED(ImageHelpers::album_art_data_to_bitmap(data, bitmap))
		{
			const std::wstring wpath = wdisplay_path(path);
			return new ComObject<JSImage>(bitmap, wpath);
		}
	}
	catch (...) {}

	return nullptr;
}

IJSImage* AlbumArt::to_image(uint32_t max_size)
{
	if (m_data.is_empty()) return nullptr;

	wil::com_ptr_t<IWICBitmap> bitmap;
	if FAILED(ImageHelpers::album_art_data_to_bitmap(m_data, bitmap)) return nullptr;
	if FAILED(ImageHelpers::fit_to(max_size, bitmap)) return nullptr;
	return new ComObject<JSImage>(bitmap, m_path);
}

bool AlbumArt::try_normal()
{
	auto handles = pfc::list_single_ref_t<metadb_handle_ptr>(m_handle);
	auto guids = pfc::list_single_ref_t<GUID>(m_guid);

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
		m_path = wdisplay_path(paths->get_path(0));
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
		Component::popup("Album art not found.");
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
