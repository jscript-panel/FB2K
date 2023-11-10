#pragma once

class MoveSelectionCallback : public playlist_manager::enum_items_callback
{
public:
	MoveSelectionCallback(size_t new_pos) : m_new_pos(new_pos) {}

	bool on_item(size_t index, const metadb_handle_ptr&, bool selected) final
	{
		if (selected)
		{
			if (index < m_new_pos) m_selected_before++;
			m_selected.emplace_back(index);
		}
		else
		{
			m_not_selected.emplace_back(index);
		}
		return true;
	}

	bool reorder(size_t playlistIndex)
	{
		m_new_pos = std::min(m_new_pos - m_selected_before, m_not_selected.size());
		std::vector<size_t> new_order;
		std::ranges::copy(m_not_selected | std::views::take(m_new_pos), std::back_inserter(new_order));
		std::ranges::copy(m_selected, std::back_inserter(new_order));
		std::ranges::copy(m_not_selected | std::views::drop(m_new_pos), std::back_inserter(new_order));
		return Plman::api()->playlist_reorder_items(playlistIndex, new_order.data(), new_order.size());
	}

private:
	std::vector<size_t> m_selected, m_not_selected;
	size_t m_new_pos{}, m_selected_before{};
};
