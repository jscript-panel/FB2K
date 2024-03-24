#pragma once

class AlbumArtStatic
{
public:
	enum class Type
	{
		JPG,
		WEBP,
	};

	static GUID get_guid(size_t id);
	static HRESULT bitmap_to_jpg_data(IWICBitmap* bitmap, album_art_data_ptr& data);
	static HRESULT bitmap_to_webp_data(IWICBitmap* bitmap, album_art_data_ptr& data);
	static HRESULT check_id(size_t id);
	static HRESULT image_to_data(IJSImage* image, Type type, album_art_data_ptr& data);
	static HRESULT to_bitmap(const album_art_data_ptr& data, wil::com_ptr_t<IWICBitmap>& bitmap);
	static IJSImage* get_attached_image(const metadb_handle_ptr& handle, size_t id);
	static album_art_data_ptr path_to_data(wil::zwstring_view path);
	static void attach_image(metadb_handle_list_cref handles, size_t id, wil::zwstring_view path);
	static void attach_image2(metadb_handle_list_cref handles, size_t id, Type type, IJSImage* image);
	static void remove_attached_image(metadb_handle_list_cref handles, size_t id);
	static void remove_all_attached_images(metadb_handle_list_cref handles);
};
