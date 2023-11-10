#pragma once

class Embed : public threaded_process_callback
{
public:
	enum class Action
	{
		Attach,
		Remove,
		RemoveAll
	};

	Embed(Action action, metadb_handle_list_cref handles, size_t id = 0, const album_art_data_ptr& data = album_art_data_ptr());

	void run(threaded_process_status& status, abort_callback& abort) final;

private:
	Action m_action{};
	album_art_data_ptr m_data;
	metadb_handle_list m_handles;
	size_t m_id{};
};
