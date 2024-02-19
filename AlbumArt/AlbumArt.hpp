#pragma once
#include "AlbumArtStatic.hpp"

class AlbumArt
{
public:
	AlbumArt(const metadb_handle_ptr& handle, size_t id, bool want_stub = false);
	AlbumArt(size_t id); // stub only

	IJSImage* to_image(uint32_t max_size = 0U);
	void show_viewer();

private:
	bool try_normal();
	bool try_now_playing();
	void set_path(const album_art_path_list::ptr& paths);
	void try_stub();

	GUID m_guid;
	album_art_data_ptr m_data;
	album_art_manager_v2::ptr m_api;
	metadb_handle_ptr m_handle;
	std::wstring m_path;
};
