#pragma once

class AlbumArt
{
public:
	using Data = fb2k::memBlock::ptr;

	AlbumArt(const metadb_handle_ptr& handle, size_t id, bool want_stub);
	AlbumArt(size_t id); // stub only

	static Data istream_to_data(IStream* stream);
	static Data path_to_data(wil::zwstring_view path);
	static HRESULT check_id(size_t id);
	static IJSImage* get_embedded(const metadb_handle_ptr& handle, size_t id);

	IJSImage* to_image(uint32_t max_size = 0U);
	void show_viewer();

private:
	bool try_normal();
	bool try_now_playing();
	void set_path(const album_art_path_list::ptr& paths);
	void try_stub();

	Data m_data;
	GUID m_guid;
	album_art_manager_v2::ptr m_api;
	metadb_handle_ptr m_handle;
	std::wstring m_path;
};
