#pragma once

class ContextMenuCommand
{
public:
	ContextMenuCommand(std::wstring_view command);
	ContextMenuCommand(std::wstring_view command, metadb_handle_list_cref handles);

	bool execute();

private:
	bool execute_recur(contextmenu_node* parent, std::string_view parent_path = "");
	bool match_command(std::string_view what);

	contextmenu_manager::ptr m_cm;
	std::string m_command;
};
