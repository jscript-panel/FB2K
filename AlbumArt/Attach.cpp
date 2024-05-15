#include "stdafx.hpp"
#include "Attach.hpp"

Attach::Attach(Action action, metadb_handle_list_cref handles, const GUID& guid, const album_art_data_ptr& data) : m_action(action), m_handles(handles), m_guid(guid), m_data(data) {}

#pragma region static
void Attach::init(threaded_process_callback::ptr callback, wil::zstring_view title)
{
	threaded_process::get()->run_modeless(callback, threaded_process::flag_silent, Fb::wnd(), title.data());
}
#pragma endregion

void Attach::run(threaded_process_status& status, abort_callback& abort)
{
	const size_t count = m_handles.get_count();
	auto api = file_lock_manager::get();
	album_art_editor::ptr ptr;
	std::set<string8> paths;

	for (auto&& [index, handle] : std::views::enumerate(m_handles))
	{
		const string8 path = handle->get_path();
		if (!paths.emplace(path).second) continue;
		if (!album_art_editor::g_get_interface(ptr, path)) continue;

		status.set_progress(index + 1, count);
		status.set_item_path(path);

		try
		{
			auto lock = api->acquire_write(path, abort);
			auto instance_ptr = ptr->open(nullptr, path, abort);
			switch (m_action)
			{
			case Action::Attach:
				instance_ptr->set(m_guid, m_data, abort);
				break;
			case Action::Remove:
				instance_ptr->remove(m_guid);
				break;
			case Action::RemoveAll:
				instance_ptr->remove_all_();
				break;
			}
			instance_ptr->commit(abort);
		}
		catch (...) {}
	}
}
