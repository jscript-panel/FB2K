#pragma once

class Attach : public threaded_process_callback
{
public:
	enum class Action
	{
		Attach,
		Remove,
		RemoveAll
	};

	Attach(Action action, metadb_handle_list_cref handles, const GUID& guid = pfc::guid_null, const album_art_data_ptr& data = album_art_data_ptr());

	static void init(threaded_process_callback::ptr callback, std::string_view title);

	void run(threaded_process_status& status, abort_callback& abort) final;

private:
	Action m_action{};
	GUID m_guid;
	album_art_data_ptr m_data;
	metadb_handle_list m_handles;
};
