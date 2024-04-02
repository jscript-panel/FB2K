#include "stdafx.hpp"
#include "ContextMenuCommand.hpp"

ContextMenuCommand::ContextMenuCommand(wil::zwstring_view command) : m_command(from_wide(command))
{
	if (playback_control::get()->is_playing())
	{
		m_cm = contextmenu_manager::get();
		m_cm->init_context_now_playing(contextmenu_manager::flag_view_full);
	}
}

ContextMenuCommand::ContextMenuCommand(wil::zwstring_view command, metadb_handle_list_cref handles) : m_command(from_wide(command))
{
	if (handles.get_count() > 0)
	{
		m_cm = contextmenu_manager::get();
		m_cm->init_context(handles, contextmenu_manager::flag_view_full);
	}
}

bool ContextMenuCommand::execute()
{
	if (m_cm.is_empty()) return false;
	return execute_recur(m_cm->get_root());
}

bool ContextMenuCommand::execute_recur(contextmenu_node* parent, wil::zstring_view parent_path)
{
	for (const size_t i : std::views::iota(size_t{}, parent->get_num_children()))
	{
		contextmenu_node* child = parent->get_child(i);
		std::string path(parent_path);
		path.append(child->get_name());

		switch (child->get_type())
		{
		case contextmenu_item_node::type_group:
			path.append("/");
			if (execute_recur(child, path))
			{
				return true;
			}
			break;
		case contextmenu_item_node::type_command:
			if (match_command(path))
			{
				child->execute();
				return true;
			}
			break;
		}
	}
	return false;
}

bool ContextMenuCommand::match_command(wil::zstring_view what)
{
	return compare_string(m_command, what);
}
