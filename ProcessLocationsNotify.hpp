#pragma once

class ProcessLocationsNotify : public process_locations_notify
{
public:
	ProcessLocationsNotify(const GUID& guid, size_t base, bool to_select) : m_guid(guid), m_base(base), m_to_select(to_select) {}

	static void init(const pfc::string_list_impl& list, process_locations_notify::ptr ptr)
	{
		static constexpr uint32_t flags = playlist_incoming_item_filter_v2::op_flag_no_filter | playlist_incoming_item_filter_v2::op_flag_delay_ui;
		playlist_incoming_item_filter_v2::get()->process_locations_async(list, flags, nullptr, nullptr, nullptr, ptr);
	}

	void on_aborted() final {}

	void on_completion(metadb_handle_list_cref handles) final
	{
		const size_t playlistIndex = Plman::api()->find_playlist_by_guid(m_guid);
		if (playlistIndex == SIZE_MAX) return;

		const uint32_t mask = Plman::api()->playlist_lock_get_filter_mask(playlistIndex);
		if (WI_IsFlagSet(mask, playlist_lock::filter_add)) return;

		if (m_to_select)
		{
			Plman::api()->playlist_insert_items(playlistIndex, m_base, handles, pfc::bit_array_true());
			Plman::api()->set_active_playlist(playlistIndex);
			Plman::api()->playlist_set_focus_item(playlistIndex, m_base);
		}
		else
		{
			Plman::api()->playlist_insert_items(playlistIndex, m_base, handles, pfc::bit_array_false());
		}
	}

private:
	GUID m_guid;
	bool m_to_select{};
	size_t m_base{};
};
