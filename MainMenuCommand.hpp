#pragma once

class MainMenuCommand
{
public:
	MainMenuCommand(std::wstring_view command);

	bool execute();

private:
	static bool is_disabled(const mainmenu_commands::ptr& ptr, uint32_t index);
	static pfc::map_t<GUID, mainmenu_group::ptr> get_group_guid_map();
	static std::string build_parent_path(GUID parent);

	bool execute_recur(mainmenu_node::ptr node, std::string_view parent_path);
	bool match_command(std::string_view what);

	std::string m_command;
};
