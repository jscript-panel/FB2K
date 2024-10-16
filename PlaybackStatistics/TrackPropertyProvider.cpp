#include "stdafx.hpp"

namespace
{
	class TrackPropertyProvider : public track_property_provider_v5
	{
	public:
		bool is_our_tech_info(const char*) final
		{
			return false;
		}

		void enumerate_properties_v5(metadb_handle_list_cref handles, track_property_provider_v5_info_source& source, track_property_callback_v2& callback, abort_callback&) final
		{
			if (callback.is_group_wanted(Component::name.data()))
			{
				if (handles.get_count() == 1)
				{
					auto rec = source.get_info(0);

					if (rec.info.is_empty())
						return;

					const auto hash = MetadbIndex::client()->transform(rec.info->info(), handles[0]->get_location());
					const auto f = PlaybackStatistics::get_fields(hash);

					if (f.playcount > 0)
					{
						callback.set_property(Component::name.data(), 0.0, "Playcount", pfc::format_uint(f.playcount));
					}

					if (f.loved > 0)
					{
						callback.set_property(Component::name.data(), 1.0, "Loved", pfc::format_uint(f.loved));
					}

					if (f.rating > 0)
					{
						callback.set_property(Component::name.data(), 2.0, "Rating", pfc::format_uint(f.rating));
					}

					if (f.skipcount > 0)
					{
						callback.set_property(Component::name.data(), 3.0, "Skipcount", pfc::format_uint(f.skipcount));
					}

					if (f.first_played > 0)
					{
						callback.set_property(Component::name.data(), 4.0, "First Played", PlaybackStatistics::timestamp_to_string(f.first_played));
					}

					if (f.last_played > 0)
					{
						callback.set_property(Component::name.data(), 5.0, "Last Played", PlaybackStatistics::timestamp_to_string(f.last_played));
					}
				}
				else
				{
					const uint32_t total = PlaybackStatistics::get_total_playcount(handles, source);

					if (total > 0)
					{
						callback.set_property(Component::name.data(), 0.0, "Playcount", pfc::format_uint(total));
					}
				}
			}
		}
	};

	FB2K_SERVICE_FACTORY(TrackPropertyProvider);
}
