#include "stdafx.hpp"

PlaybackStatistics::TransactionScope::TransactionScope()
{
	if (Fb::is_v2())
	{
		ptr = metadb_index_manager_v2::get()->begin_transaction();
	}
}

PlaybackStatistics::TransactionScope::~TransactionScope()
{
	if (ptr.is_valid())
	{
		ptr->commit();
	}
}

PlaybackStatistics::Fields PlaybackStatistics::get_fields(metadb_index_hash hash)
{
	mem_block_container_impl temp;
	api()->get_user_data(guids::metadb_index, hash, temp);
	if (temp.get_size() > 0)
	{
		try
		{
			Fields f;
			stream_reader_formatter_simple_ref reader(temp.get_ptr(), temp.get_size());

			reader >> f.first_played;
			reader >> f.last_played;
			reader >> f.loved;
			reader >> f.playcount;
			reader >> f.rating;

			if (reader.get_remaining() >= sizeof(uint32_t))
			{
				reader >> f.skipcount;
			}

			return f;
		}
		catch (exception_io_data) {}
	}
	return Fields();
}

metadb_index_manager::ptr PlaybackStatistics::api()
{
	// latest foo_sample does this
	static metadb_index_manager* cached = metadb_index_manager::get().detach();
	return cached;
}

string8 PlaybackStatistics::timestamp_to_string(uint64_t ts)
{
	const uint64_t windows_time = pfc::fileTimeUtoW(ts);
	return pfc::format_filetimestamp(windows_time);
}

uint32_t PlaybackStatistics::get_total_playcount(metadb_handle_list_cref handles, track_property_provider_v5_info_source& source)
{
	HashSet hash_set;
	uint32_t total{};

	const size_t count = handles.get_count();
	auto client = MetadbIndex::client();

	for (const size_t i : std::views::iota(0U, count))
	{
		auto rec = source.get_info(i);
		if (rec.info.is_empty()) continue;

		const auto hash = client->transform(rec.info->info(), handles[i]->get_location());
		if (hash_set.emplace(hash).second)
		{
			total += get_fields(hash).playcount;
		}
	}

	return total;
}

uint32_t PlaybackStatistics::now()
{
	return to_uint(pfc::fileTimeWtoU(pfc::fileTimeNow()));
}

uint32_t PlaybackStatistics::string_to_timestamp(wil::zwstring_view str)
{
	const string8 ustr = from_wide(str);
	const uint64_t windows_time = pfc::filetimestamp_from_string(ustr);
	if (windows_time == filetimestamp_invalid) return UINT_MAX;
	return to_uint(pfc::fileTimeWtoU(windows_time));
}

void PlaybackStatistics::clear(metadb_handle_list_cref handles)
{
	HashList hash_list;

	{
		HashSet hash_set;
		metadb_index_hash hash{};
		auto client = MetadbIndex::client();
		auto scope = TransactionScope();

		for (auto&& handle : handles)
		{
			if (client->hashHandle(handle, hash) && hash_set.emplace(hash).second)
			{
				set_fields(hash, Fields(), scope.ptr);
				hash_list.add_item(hash);
			}
		}
	}

	refresh(hash_list);
}

void PlaybackStatistics::refresh(const HashList& hash_list)
{
	if (hash_list.get_count() > 0)
	{
		api()->dispatch_refresh(guids::metadb_index, hash_list);
	}
}

void PlaybackStatistics::refresh(metadb_handle_list_cref handles)
{
	HashList hash_list;
	HashSet hash_set;
	metadb_index_hash hash{};
	auto client = MetadbIndex::client();

	for (auto&& handle : handles)
	{
		if (client->hashHandle(handle, hash) && hash_set.emplace(hash).second)
		{
			hash_list.add_item(hash);
		}
	}

	refresh(hash_list);
}

void PlaybackStatistics::set_fields(metadb_index_hash hash, const Fields& f, const metadb_index_transaction::ptr& transaction_ptr)
{
	stream_writer_formatter_simple writer;
	writer << f.first_played;
	writer << f.last_played;
	writer << f.loved;
	writer << f.playcount;
	writer << f.rating;
	writer << f.skipcount;

	if (transaction_ptr.is_valid())
	{
		transaction_ptr->set_user_data(guids::metadb_index, hash, writer.m_buffer.get_ptr(), writer.m_buffer.get_size());
	}
	else
	{
		api()->set_user_data(guids::metadb_index, hash, writer.m_buffer.get_ptr(), writer.m_buffer.get_size());
	}
}
