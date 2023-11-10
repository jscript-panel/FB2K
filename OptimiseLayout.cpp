#include "stdafx.hpp"
#include "OptimiseLayout.hpp"

OptimiseLayout::OptimiseLayout(metadb_handle_list_cref handles, bool minimise) : m_handles(handles), m_minimise(minimise) {}

void OptimiseLayout::run(threaded_process_status& status, abort_callback& abort)
{
	const size_t count = m_handles.get_count();
	auto api = file_lock_manager::get();
	std::set<string8> paths;

	for (auto&& [index, handle] : std::views::enumerate(m_handles))
	{
		const string8 path = handle->get_path();
		if (!paths.emplace(path).second) continue;

		status.set_progress(index + 1, count);
		status.set_item_path(path);

		try
		{
			auto lock = api->acquire_write(path, abort);
			for (auto ptr : file_format_sanitizer::enumerate())
			{
				if (ptr->sanitize_file(path, m_minimise, abort)) break;
			}
		}
		catch (...) {}
	}
}
