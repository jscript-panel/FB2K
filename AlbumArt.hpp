#pragma once

class AlbumArt
{
public:
	using Data = fb2k::memBlock::ptr;

	AlbumArt(const metadb_handle_ptr& handle, size_t id, bool want_stub);
	AlbumArt(size_t id); // stub only

	static Data istream_to_data(IStream* stream);
	static Data path_to_data(wil::zwstring_view path);
	static GUID get_guid(size_t id);
	static HRESULT check_id(size_t id);
	static IJSImage* get_attached_image(const metadb_handle_ptr& handle, size_t id);
	static void attach_image(metadb_handle_list_cref handles, size_t id, wil::zwstring_view path);
	static void remove_attached_image(metadb_handle_list_cref handles, size_t id);
	static void remove_all_attached_images(metadb_handle_list_cref handles);

	IJSImage* to_image(uint32_t max_size = 0U);
	void show_viewer();

private:
	class Attach : public threaded_process_callback
	{
	public:
		enum class Action { Attach, Remove, RemoveAll };

		Attach(Action action, metadb_handle_list_cref handles, const GUID& guid = pfc::guid_null, const Data& data = Data());

		static void init(threaded_process_callback::ptr callback, wil::zstring_view title);

		void run(threaded_process_status& status, abort_callback& abort) final;

	private:
		Action m_action{};
		Data m_data;
		GUID m_guid;
		metadb_handle_list m_handles;
	};

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
