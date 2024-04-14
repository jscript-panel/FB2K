#include "stdafx.hpp"
#include "PlaylistLock.hpp"

PlaylistLock::PlaylistLock(uint32_t mask) : m_mask(mask) {}

#pragma region static
bool PlaylistLock::add(size_t playlistIndex, uint32_t mask)
{
	if (mask == 0U) return false;
	if (Plman::api()->playlist_lock_is_present(playlistIndex)) return false;

	auto lock = fb2k::service_new<PlaylistLock>(mask);
	if (!Plman::api()->playlist_lock_install(playlistIndex, lock)) return false;

	Plman::api()->playlist_set_property_int(playlistIndex, guids::playlist_lock_mask, mask);
	const auto id = Plman::get_id(playlistIndex);
	s_map.emplace(id, lock);
	return true;
}

bool PlaylistLock::is_my_lock(size_t playlistIndex)
{
	const auto id = Plman::get_id(playlistIndex);
	return s_map.contains(id);
}

bool PlaylistLock::remove(size_t playlistIndex)
{
	const auto id = Plman::get_id(playlistIndex);
	const auto it = s_map.find(id);

	if (it == s_map.end()) return false;

	const bool ret = Plman::api()->playlist_lock_uninstall(playlistIndex, it->second);
	Plman::api()->playlist_remove_property(playlistIndex, guids::playlist_lock_mask);
	s_map.erase(id);
	return ret;
}
#pragma endregion

bool PlaylistLock::execute_default_action(size_t)
{
	return false;
}

bool PlaylistLock::query_items_add(size_t, const pfc::list_base_const_t<metadb_handle_ptr>&, const bit_array&)
{
	return !WI_IsFlagSet(m_mask, filter_add);
}

bool PlaylistLock::query_items_remove(const bit_array&, bool)
{
	return !WI_IsFlagSet(m_mask, filter_remove);
}

bool PlaylistLock::query_items_reorder(const size_t*, size_t)
{
	return !WI_IsFlagSet(m_mask, filter_reorder);
}

bool PlaylistLock::query_item_replace(size_t, const metadb_handle_ptr&, const metadb_handle_ptr&)
{
	return !WI_IsFlagSet(m_mask, filter_replace);
}

bool PlaylistLock::query_playlist_remove()
{
	return !WI_IsFlagSet(m_mask, filter_remove_playlist);
}

bool PlaylistLock::query_playlist_rename(const char*, size_t)
{
	return !WI_IsFlagSet(m_mask, filter_rename);
}

uint32_t PlaylistLock::get_filter_mask()
{
	return m_mask;
}

void PlaylistLock::get_lock_name(pfc::string_base& out)
{
	out = Component::name;
}
