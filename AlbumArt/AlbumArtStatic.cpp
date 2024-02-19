#include "stdafx.hpp"
#include "AlbumArtStatic.hpp"
#include "Attach.hpp"

GUID AlbumArtStatic::get_guid(size_t id)
{
	return *guids::art[id];
}

HRESULT AlbumArtStatic::check_id(size_t id)
{
	if (id < guids::art.size()) return S_OK;
	return E_INVALIDARG;
}

HRESULT AlbumArtStatic::to_bitmap(const album_art_data_ptr& data, wil::com_ptr_t<IWICBitmap>& bitmap)
{
	RETURN_HR_IF(E_FAIL, data.is_empty());
	if SUCCEEDED(ImageHelpers::libwebp_data_to_bitmap(static_cast<const uint8_t*>(data->data()), data->size(), bitmap)) return S_OK;

	wil::com_ptr_t<IStream> stream;
	RETURN_IF_FAILED(IStreamHelpers::create_from_album_art_data(data, stream));
	RETURN_IF_FAILED(ImageHelpers::istream_to_bitmap(stream.get(), bitmap));
	return S_OK;
}

IJSImage* AlbumArtStatic::get_attached_image(const metadb_handle_ptr& handle, size_t id)
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

		if SUCCEEDED(to_bitmap(data, bitmap))
		{
			const std::wstring wpath = wdisplay_path(path);
			return new ComObject<JSImage>(bitmap, wpath);
		}
	}
	catch (...) {}

	return nullptr;
}

album_art_data_ptr AlbumArtStatic::path_to_data(wil::zwstring_view path)
{
	album_art_data_ptr data;
	wil::com_ptr_t<IStream> stream;

	if SUCCEEDED(IStreamHelpers::create_from_path(path, stream))
	{
		data = IStreamHelpers::to_album_art_data(stream.get());
	}

	return data;
}

void AlbumArtStatic::attach_image(metadb_handle_list_cref handles, size_t id, wil::zwstring_view path)
{
	const GUID guid = get_guid(id);
	auto data = path_to_data(path);
	if (data.is_valid())
	{
		auto callback = fb2k::service_new<Attach>(Attach::Action::Attach, handles, guid, data);
		Attach::init(callback, "Attaching image...");
	}
}

void AlbumArtStatic::remove_attached_image(metadb_handle_list_cref handles, size_t id)
{
	const GUID guid = get_guid(id);
	auto callback = fb2k::service_new<Attach>(Attach::Action::Remove, handles, guid);
	Attach::init(callback, "Removing attached images...");
}

void AlbumArtStatic::remove_all_attached_images(metadb_handle_list_cref handles)
{
	auto callback = fb2k::service_new<Attach>(Attach::Action::RemoveAll, handles);
	Attach::init(callback, "Removing attached images...");
}
