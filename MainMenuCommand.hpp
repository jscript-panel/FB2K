#pragma once

class MainMenuCommand
{
public:
	MainMenuCommand(wil::zwstring_view command);

	bool execute();

private:
	bool execute_recur(mainmenu_node::ptr node, wil::zstring_view parent_path);
	bool match_command(wil::zstring_view what);
	std::string build_parent_path(GUID parent);

	inline static pfc::map_t<GUID, mainmenu_group::ptr> s_group_guid_map;
	std::string m_command;
};
