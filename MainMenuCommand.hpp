#pragma once

class MainMenuCommand
{
public:
	MainMenuCommand(wil::zwstring_view command);

	bool execute();

private:
	static pfc::map_t<GUID, mainmenu_group::ptr> get_group_guid_map();
	static std::string build_parent_path(GUID parent);

	bool execute_recur(mainmenu_node::ptr node, wil::zstring_view parent_path);
	bool match_command(wil::zstring_view what);

	std::string m_command;
};
