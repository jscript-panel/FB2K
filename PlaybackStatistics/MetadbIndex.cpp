#include "stdafx.hpp"

namespace
{
	static constexpr std::string_view default_pin_to = "$lower($meta(artist,0) - %title%)";
	cfg_string pin_to(guids::cfg_string_pin_to, default_pin_to.data());

	advconfig_branch_factory advconfig_branch(Component::name.data(), guids::advconfig_branch, advconfig_branch::guid_branch_tools, 0.0);
	advconfig_string_factory advconfig_pin_to("Playback Statistics Title Format", guids::advconfig_pin_to, guids::advconfig_branch, 0.0, default_pin_to.data(), preferences_state::needs_restart);

	void init_stage()
	{
		const auto adv_pin_to = advconfig_pin_to.get();

		if (adv_pin_to != pin_to)
		{
			// If pattern has changed, nuke old data before GUID is registered
			pin_to = adv_pin_to;
			PlaybackStatistics::api()->erase_orphaned_data(guids::metadb_index);
		}

		MetadbIndex::init();
	}

	FB2K_ON_INIT_STAGE(init_stage, init_stages::after_config_read);
}

MetadbIndex::MetadbIndex() : m_hasher(hasher_md5::get())
{
	titleformat_compiler::get()->compile_safe(m_obj, pin_to.get());
}

#pragma region static
MetadbIndex* MetadbIndex::client()
{
	static MetadbIndex* cached = new service_impl_single_t<MetadbIndex>();
	return cached;
}

void MetadbIndex::init()
{
	auto api = PlaybackStatistics::api();

	try
	{
		api->add(client(), guids::metadb_index, system_time_periods::week * 4);
		api->dispatch_global_refresh();
	}
	catch (const std::exception& e)
	{
		api->remove(guids::metadb_index);
		const auto msg = fmt::format("Playback Statistics critical initialisation failure: {}", e.what());
		Component::log(msg);
	}
}
#pragma endregion

metadb_index_hash MetadbIndex::transform(const file_info& info, const playable_location& location)
{
	string8 str;
	m_obj->run_simple(location, &info, str);
	return m_hasher->process_single_string(str).xorHalve();
}
