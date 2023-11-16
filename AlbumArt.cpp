#include "stdafx.hpp"

AlbumArt::AlbumArt(const metadb_handle_ptr& handle, size_t id, bool want_stub) : m_handle(handle), m_guid(get_guid(id)), m_api(album_art_manager_v2::get())
{
	if (try_now_playing()) return;
	if (try_normal()) return;
	if (want_stub) try_stub();
}

AlbumArt::AlbumArt(size_t id) : m_guid(get_guid(id)), m_api(album_art_manager_v2::get())
{
	try_stub();
}

AlbumArt::Data AlbumArt::istream_to_data(IStream* stream)
{
	STATSTG sts;

	if SUCCEEDED(stream->Stat(&sts, STATFLAG_DEFAULT))
	{
		const auto bytes = sts.cbSize.LowPart;
		auto data = fb2k::service_new<album_art_data_impl>();
		data->set_size(bytes);
		ULONG bytes_read{};
		if SUCCEEDED(stream->Read(data->get_ptr(), bytes, &bytes_read))
		{
			return data;
		}
	}
	return Data();
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

GUID AlbumArt::get_guid(size_t id)
{
	return *guids::art[id];
}

HRESULT AlbumArt::check_id(size_t id)
{
	if (id < guids::art.size()) return S_OK;
	return E_INVALIDARG;
}

IJSImage* AlbumArt::get_attached_image(const metadb_handle_ptr& handle, size_t id)
{
	const GUID guid = get_guid(id);
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

void AlbumArt::attach_image(metadb_handle_list_cref handles, size_t id, wil::zwstring_view path)
{
	const GUID guid = get_guid(id);
	auto data = path_to_data(path);
	if (data.is_valid())
	{
		auto callback = fb2k::service_new<Attach>(Attach::Action::Attach, handles, guid, data);
		Attach::init(callback, "Attaching image...");
	}
}

void AlbumArt::remove_attached_image(metadb_handle_list_cref handles, size_t id)
{
	const GUID guid = get_guid(id);
	auto callback = fb2k::service_new<Attach>(Attach::Action::Remove, handles, guid);
	Attach::init(callback, "Removing attached images...");
}

void AlbumArt::remove_all_attached_images(metadb_handle_list_cref handles)
{
	auto callback = fb2k::service_new<Attach>(Attach::Action::RemoveAll, handles);
	Attach::init(callback, "Removing attached images...");
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

AlbumArt::Attach::Attach(Action action, metadb_handle_list_cref handles, const GUID& guid, const Data& data) : m_action(action), m_handles(handles), m_guid(guid), m_data(data) {}

void AlbumArt::Attach::init(threaded_process_callback::ptr callback, wil::zstring_view title)
{
	const auto flags = Component::get_threaded_process_flags();
	threaded_process::get()->run_modeless(callback, flags, Fb::wnd(), title.data());
}

void AlbumArt::Attach::run(threaded_process_status& status, abort_callback& abort)
{
	const size_t count = m_handles.get_count();
	auto api = file_lock_manager::get();
	album_art_editor::ptr ptr;
	std::set<string8> paths;

	for (auto&& [index, handle] : std::views::enumerate(m_handles))
	{
		const string8 path = handle->get_path();
		if (!paths.emplace(path).second) continue;
		if (!album_art_editor::g_get_interface(ptr, path)) continue;

		status.set_progress(index + 1, count);
		status.set_item_path(path);

		try
		{
			auto lock = api->acquire_write(path, abort);
			auto instance_ptr = ptr->open(nullptr, path, abort);
			switch (m_action)
			{
			case Action::Attach:
				instance_ptr->set(m_guid, m_data, abort);
				break;
			case Action::Remove:
				instance_ptr->remove(m_guid);
				break;
			case Action::RemoveAll:
				instance_ptr->remove_all_();
				break;
			}
			instance_ptr->commit(abort);
		}
		catch (...) {}
	}
}
