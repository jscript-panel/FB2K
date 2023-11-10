#pragma once

class ContextMenuCommand
{
public:
	ContextMenuCommand(wil::zwstring_view command);
	ContextMenuCommand(wil::zwstring_view command, metadb_handle_list_cref handles);

	bool execute();

private:
	bool execute_recur(contextmenu_node* parent, wil::zstring_view parent_path = "");
	bool match_command(wil::zstring_view what);

	contextmenu_manager::ptr m_cm;
	std::string m_command;
};
