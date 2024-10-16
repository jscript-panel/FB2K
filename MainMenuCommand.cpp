#include "stdafx.hpp"
#include "MainMenuCommand.hpp"

MainMenuCommand::MainMenuCommand(std::wstring_view command) : m_command(js::from_wide(command)) {}

#pragma region static
bool MainMenuCommand::is_disabled(const mainmenu_commands::ptr& ptr, uint32_t index)
{
	string8 display;
	uint32_t flags{};
	ptr->get_display(index, display, flags);

	return WI_IsFlagSet(flags, mainmenu_commands::flag_disabled);
}

pfc::map_t<GUID, mainmenu_group::ptr> MainMenuCommand::get_group_guid_map()
{
	pfc::map_t<GUID, mainmenu_group::ptr> group_guid_map;

	for (auto ptr : mainmenu_group::enumerate())
	{
		group_guid_map.set(ptr->get_guid(), ptr);
	}

	return group_guid_map;
}

std::string MainMenuCommand::build_parent_path(GUID parent)
{
	static const auto group_guid_map = get_group_guid_map();
	Strings strings;

	while (parent != pfc::guid_null)
	{
		const auto group_ptr = group_guid_map[parent];
		mainmenu_group_popup::ptr group_popup_ptr;

		if (group_ptr->cast(group_popup_ptr))
		{
			string8 str;
			group_popup_ptr->get_display_string(str);
			strings.emplace_back(str.get_ptr());
		}

		parent = group_ptr->get_parent();
	}

	return fmt::format("{}/", fmt::join(strings | std::views::reverse, "/"));
}
#pragma endregion

bool MainMenuCommand::execute()
{
	// Ensure commands on the Edit menu are enabled
	ui_edit_context_manager::get()->set_context_active_playlist();

	for (auto ptr : mainmenu_commands::enumerate())
	{
		mainmenu_commands_v2::ptr v2_ptr;
		ptr->cast(v2_ptr);

		const auto parent_path = build_parent_path(ptr->get_parent());

		for (const uint32_t i : std::views::iota(0U, ptr->get_command_count()))
		{
			if (v2_ptr.is_valid() && v2_ptr->is_command_dynamic(i))
			{
				const auto node = v2_ptr->dynamic_instantiate(i);

				if (execute_recur(node, parent_path))
					return true;
			}
			else
			{
				string8 name;
				ptr->get_name(i, name);
				const auto path = parent_path + name.get_ptr();

				if (match_command(path))
				{
					if (is_disabled(ptr, i))
						return false;

					ptr->execute(i, nullptr);
					return true;
				}
			}
		}
	}
	return false;
}

bool MainMenuCommand::execute_recur(mainmenu_node::ptr node, std::string_view parent_path)
{
	string8 text;
	uint32_t flags{};
	node->get_display(text, flags);

	const auto type = node->get_type();
	auto path = fmt::format("{}{}", parent_path, text.get_ptr());

	if (type == mainmenu_node::type_group)
	{
		if (!path.ends_with("/"))
		{
			path.append("/");
		}

		for (const size_t i : std::views::iota(size_t{}, node->get_children_count()))
		{
			const auto child = node->get_child(i);

			if (execute_recur(child, path))
				return true;
		}
	}
	else if (type == mainmenu_node::type_command && match_command(path))
	{
		if (WI_IsFlagSet(flags, mainmenu_commands::flag_disabled))
			return false;

		node->execute(nullptr);
		return true;
	}

	return false;
}

bool MainMenuCommand::match_command(std::string_view what)
{
	return js::compare_string(m_command, what);
}
