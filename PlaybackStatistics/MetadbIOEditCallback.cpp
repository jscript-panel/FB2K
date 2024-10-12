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
				auto client = MetadbIndex::client();
				auto scope = PlaybackStatistics::TransactionScope();

				for (const size_t i : std::views::iota(size_t{}, handles.get_count()))
				{
					const auto& location = handles[i]->get_location();
					const auto old_hash = client->transform(*before[i], location);
					const auto new_hash = client->transform(*after[i], location);

					if (old_hash == new_hash)
						continue;

					if (!hash_set.emplace(new_hash).second)
						continue;

					const auto f = PlaybackStatistics::get_fields(old_hash);

					if (!f)
						continue;

					PlaybackStatistics::set_fields(new_hash, f, scope.ptr);
					hash_list.add_item(new_hash);
				}
			}

			PlaybackStatistics::refresh(hash_list);
		}
	};

	FB2K_SERVICE_FACTORY(MetadbIOEditCallback);
}
