#include "stdafx.hpp"

namespace
{
	class MetadbIOEditCallback : public metadb_io_edit_callback
	{
	public:
		void on_edited(metadb_handle_list_cref handles, t_infosref before, t_infosref after)
		{
			PlaybackStatistics::HashList hash_list;

			{
				PlaybackStatistics::HashSet hash_set;
				const size_t count = handles.get_count();
				auto client = MetadbIndex::client();
				auto scope = PlaybackStatistics::TransactionScope();

				for (const size_t i : std::views::iota(0U, count))
				{
					const auto old_hash = client->transform(*before[i], handles[i]->get_location());
					const auto new_hash = client->transform(*after[i], handles[i]->get_location());
					if (old_hash == new_hash) continue;
					if (!hash_set.emplace(new_hash).second) continue;

					const auto f = PlaybackStatistics::get_fields(old_hash);
					if (!f) continue;

					PlaybackStatistics::set_fields(new_hash, f, scope.ptr);
					hash_list.add_item(new_hash);
				}
			}

			PlaybackStatistics::refresh(hash_list);
		}
	};

	FB2K_SERVICE_FACTORY(MetadbIOEditCallback);
}
