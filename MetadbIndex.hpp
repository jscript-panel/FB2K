#pragma once

class MetadbIndex : public metadb_index_client
{
public:
	MetadbIndex() : m_hasher(hasher_md5::get())
	{
		titleformat_compiler::get()->compile_safe(m_obj, Component::pin_to.get());
	}

	static MetadbIndex* client()
	{
		static MetadbIndex* cached = new service_impl_single_t<MetadbIndex>();
		return cached;
	}

	static void init()
	{
		try
		{
			PlaybackStatistics::api()->add(client(), guids::metadb_index, system_time_periods::week * 4);
			PlaybackStatistics::api()->dispatch_global_refresh();
		}
		catch (std::exception const& e)
		{
			PlaybackStatistics::api()->remove(guids::metadb_index);
			FB2K_console_formatter() << fmt::format("{}: Playback Statistics critical initialisation failure: {}", Component::name, e.what());
		}
	}

	metadb_index_hash transform(const file_info& info, const playable_location& location) final
	{
		string8 str;
		m_obj->run_simple(location, &info, str);
		return m_hasher->process_single_string(str).xorHalve();
	}

private:
	hasher_md5::ptr m_hasher;
	titleformat_object::ptr m_obj;
};
