#include "stdafx.hpp"
#include "AlbumArtStatic.hpp"
#include "Attach.hpp"

GUID AlbumArtStatic::get_guid(size_t id)
{
	return *guids::art[id];
}

HRESULT AlbumArtStatic::bitmap_to_jpg_data(IWICBitmap* bitmap, album_art_data_ptr& data)
{
	D2D1_SIZE_U size{};
	RETURN_IF_FAILED(bitmap->GetSize(&size.width, &size.height));
	auto rect = js::to_WICRect(size);

	wil::com_ptr_t<IStream> stream;
	wil::com_ptr_t<IWICBitmapEncoder> encoder;
	wil::com_ptr_t<IWICBitmapFrameEncode> frame_encode;
	wil::com_ptr_t<IWICStream> wic_stream;

	RETURN_IF_FAILED(CreateStreamOnHGlobal(nullptr, TRUE, &stream));
	RETURN_IF_FAILED(factory::imaging->CreateStream(&wic_stream));
	RETURN_IF_FAILED(wic_stream->InitializeFromIStream(stream.get()));
	RETURN_IF_FAILED(factory::imaging->CreateEncoder(GUID_ContainerFormatJpeg, nullptr, &encoder));
	RETURN_IF_FAILED(encoder->Initialize(stream.get(), WICBitmapEncoderNoCache));
	RETURN_IF_FAILED(encoder->CreateNewFrame(&frame_encode, nullptr));
	RETURN_IF_FAILED(frame_encode->Initialize(nullptr));
	RETURN_IF_FAILED(frame_encode->SetSize(size.width, size.height));
	RETURN_IF_FAILED(frame_encode->WriteSource(bitmap, &rect));
	RETURN_IF_FAILED(frame_encode->Commit());
	RETURN_IF_FAILED(encoder->Commit());

	HGLOBAL hg{};
	RETURN_IF_FAILED(GetHGlobalFromStream(stream.get(), &hg));
	auto image = wil::unique_hglobal_locked(hg);
	auto gsize = GlobalSize(image.get());
	data = album_art_data_impl::g_create(image.get(), gsize);

	return S_OK;
}

HRESULT AlbumArtStatic::bitmap_to_webp_data(IWICBitmap* bitmap, album_art_data_ptr& data)
{
	D2D1_SIZE_U size{};
	RETURN_IF_FAILED(bitmap->GetSize(&size.width, &size.height));
	auto rect = js::to_WICRect(size);

	uint8_t* ptr{};
	uint8_t* output{};
	uint32_t dummy{}, stride{};
	wil::com_ptr_t<IWICBitmapLock> lock;

	RETURN_IF_FAILED(bitmap->Lock(&rect, WICBitmapLockWrite, &lock));
	RETURN_IF_FAILED(lock->GetDataPointer(&dummy, &ptr));
	RETURN_IF_FAILED(lock->GetStride(&stride));

	const size_t new_size = WebPEncodeBGRA(ptr, rect.Width, rect.Height, js::to_int(stride), 95.f, &output);

	if (new_size == 0)
		return E_FAIL;

	data = album_art_data_impl::g_create(output, new_size);
	WebPFree(output);

	return S_OK;
}

HRESULT AlbumArtStatic::check_id(size_t id)
{
	if (id < guids::art.size())
		return S_OK;

	return DISP_E_BADINDEX;
}

HRESULT AlbumArtStatic::image_to_data(IJSImage* image, Format format, album_art_data_ptr& data)
{
	IWICBitmap* bitmap{};
	RETURN_IF_FAILED(image->get(js::arg_helper(&bitmap)));

	if (format == Format::JPG)
		return bitmap_to_jpg_data(bitmap, data);

	return bitmap_to_webp_data(bitmap, data);
}

HRESULT AlbumArtStatic::to_bitmap(const album_art_data_ptr& data, wil::com_ptr_t<IWICBitmap>& bitmap)
{
	RETURN_HR_IF_EXPECTED(E_FAIL, data.is_empty());
	if SUCCEEDED(js::libwebp_data_to_bitmap(static_cast<const uint8_t*>(data->data()), data->size(), bitmap))
		return S_OK;

	wil::com_ptr_t<IStream> stream;
	RETURN_IF_FAILED(to_istream(data, stream));
	RETURN_IF_FAILED(js::istream_to_bitmap(stream.get(), bitmap));
	return S_OK;
}

HRESULT AlbumArtStatic::to_istream(const album_art_data_ptr& data, wil::com_ptr_t<IStream>& stream)
{
	RETURN_HR_IF_EXPECTED(E_FAIL, data.is_empty());

	auto ptr = static_cast<const uint8_t*>(data->data());
	const uint32_t size = js::to_uint(data->size());

	auto tmp = SHCreateMemStream(ptr, size);
	RETURN_HR_IF_NULL(E_FAIL, tmp);

	stream.attach(tmp);
	return S_OK;
}

album_art_data_ptr AlbumArtStatic::get_embedded(const metadb_handle_ptr& handle, size_t id)
{
	const GUID guid = get_guid(id);
	const string8 path = handle->get_path();
	album_art_extractor::ptr ptr;
	album_art_data_ptr data;

	if (album_art_extractor::g_get_interface(ptr, path))
	{
		try
		{
			auto instance = ptr->open(nullptr, path, fb2k::noAbort);
			data = instance->query(guid, fb2k::noAbort);
		}
		catch (...) {}
	}

	return data;
}

album_art_data_ptr AlbumArtStatic::to_data(IStream* stream)
{
	const auto size = js::get_stream_size(stream);

	if (size <= Component::max_image_size)
	{
		auto data = fb2k::service_new<album_art_data_impl>();
		data->set_size(size);
		ULONG bytes_read{};

		if SUCCEEDED(stream->Read(data->get_ptr(), size, &bytes_read))
			return data;
	}

	return album_art_data_ptr();
}

album_art_data_ptr AlbumArtStatic::to_data(std::wstring_view path)
{
	album_art_data_ptr data;
	wil::com_ptr_t<IStream> stream;

	if SUCCEEDED(FileHelper(path).read(stream))
	{
		data = to_data(stream.get());
	}

	return data;
}

void AlbumArtStatic::attach_from_path(metadb_handle_list_cref handles, size_t id, std::wstring_view path)
{
	const GUID guid = get_guid(id);
	auto data = to_data(path);

	if (data.is_valid())
	{
		auto callback = fb2k::service_new<Attach>(Attach::Action::Attach, handles, guid, data);
		Attach::init(callback, "Attaching image...");
	}
}

void AlbumArtStatic::attach_from_image(metadb_handle_list_cref handles, size_t id, Format format, IJSImage* image)
{
	const GUID guid = get_guid(id);
	album_art_data_ptr data;

	if SUCCEEDED(image_to_data(image, format, data))
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

void AlbumArtStatic::show_viewer(const album_art_data_ptr& data)
{
	if (data.is_valid())
	{
		fb2k::imageViewer::get()->show(core_api::get_main_window(), data);
	}
}
