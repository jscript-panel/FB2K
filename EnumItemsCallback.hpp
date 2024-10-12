#pragma once

class MoveSelectionCallback : public playlist_manager::enum_items_callback
{
public:
	MoveSelectionCallback(size_t pos, size_t count) : m_pos(std::min(pos, count))
	{
		m_order.reserve(count);
	}

	bool on_item(size_t index, const metadb_handle_ptr&, bool selected) final
	{
		if (selected)
		{
			m_selected.emplace_back(index);
		}
		else
		{
			if (index < m_pos)
				m_not_selected_before.emplace_back(index);
			else
				m_not_selected_after.emplace_back(index);
		}

		return true;
	}

	void reorder(size_t playlistIndex)
	{
		m_order.append_range(m_not_selected_before);
		m_order.append_range(m_selected);
		m_order.append_range(m_not_selected_after);
		Plman::api()->playlist_reorder_items(playlistIndex, m_order.data(), m_order.size());
	}

private:
	std::vector<size_t> m_selected, m_not_selected_before, m_not_selected_after, m_order;
	size_t m_pos{};
};

class SelectedIndexesCallback : public playlist_manager::enum_items_callback
{
public:
	SelectedIndexesCallback(size_t count)
	{
		m_selected.reserve(count);
	}

	bool on_item(size_t index, const metadb_handle_ptr&, bool selected) final
	{
		if (selected)
		{
			m_selected.emplace_back(index);
		}

		return true;
	}

	std::vector<size_t> m_selected;
};
