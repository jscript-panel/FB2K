#pragma once

class PlaylistLock : public playlist_lock
{
public:
	PlaylistLock(uint32_t mask);

	static bool add(size_t playlistIndex, uint32_t mask);
	static bool is_my_lock(size_t playlistIndex);
	static bool remove(size_t playlistIndex);

	bool execute_default_action(size_t) final;
	bool query_items_add(size_t, const pfc::list_base_const_t<metadb_handle_ptr>&, const bit_array&) final;
	bool query_items_remove(const bit_array&, bool) final;
	bool query_items_reorder(const size_t*, size_t) final;
	bool query_item_replace(size_t, const metadb_handle_ptr&, const metadb_handle_ptr&) final;
	bool query_playlist_remove() final;
	bool query_playlist_rename(const char*, size_t) final;
	uint32_t get_filter_mask() final;
	void get_lock_name(pfc::string_base& out) final;

	void on_playlist_index_change(size_t) final {}
	void on_playlist_remove() final {}
	void show_ui() final {}

	inline static std::unordered_map<uint64_t, playlist_lock::ptr> s_map;

private:
	uint32_t m_mask{};
};
