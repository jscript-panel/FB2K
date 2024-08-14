#pragma once

class GetAlbumArtAsync : public SimpleThreadTask
{
public:
	GetAlbumArtAsync(CWindow wnd, const metadb_handle_ptr& handle, size_t id, uint32_t max_size = 0)
		: m_wnd(wnd)
		, m_handle(handle)
		, m_id(id)
		, m_max_size(max_size) {}

	void run() final
	{
		if (m_handle.is_empty())
			return;

		auto album_art = AlbumArt(m_handle, m_id);
		IJSImage* image = album_art.to_image(m_max_size);
		IMetadbHandle* handle = new ComObject<MetadbHandle>(m_handle);

		auto data = AlbumArtCallbackData(handle, m_id, image);
		m_wnd.SendMessageW(std::to_underlying(CallbackID::on_get_album_art_done), reinterpret_cast<WPARAM>(&data));
	}

private:
	CWindow m_wnd;
	metadb_handle_ptr m_handle;
	size_t m_id{};
	uint32_t m_max_size{};
};
