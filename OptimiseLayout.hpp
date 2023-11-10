#pragma once

class OptimiseLayout : public threaded_process_callback
{
public:
	OptimiseLayout(metadb_handle_list_cref handles, bool minimise);

	void run(threaded_process_status& status, abort_callback& abort) final;

private:
	bool m_minimise{};
	metadb_handle_list m_handles;
};
