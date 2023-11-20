#include "stdafx.hpp"

PlaybackStatistics::Fields PlaybackStatistics::get_fields(metadb_index_hash hash)
{
	mem_block_container_impl temp;
	api()->get_user_data(guids::metadb_index, hash, temp);
	if (temp.get_size() > 0)
	{
		try
		{
			stream_reader_formatter_simple_ref reader(temp.get_ptr(), temp.get_size());
			Fields f;
			reader >> f.first_played;
			reader >> f.last_played;
			reader >> f.loved;
			reader >> f.playcount;
			reader >> f.rating;
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

uint32_t PlaybackStatistics::get_total_playcount(metadb_handle_list_cref handles, track_property_provider_v5_info_source& source)
{
	HashSet hash_set;
	uint32_t total{};

	const size_t count = handles.get_count();

	for (const size_t i : std::views::iota(0U, count))
	{
		auto rec = source.get_info(i);
		if (rec.info.is_empty()) continue;
		const auto hash = MetadbIndex::client()->transform(rec.info->info(), handles[i]->get_location());
		if (hash_set.emplace(hash).second)
		{
			total += get_fields(hash).playcount;
		}
	}

	return total;
}

void PlaybackStatistics::clear(metadb_handle_list_cref handles)
{
	HashSet hash_set;
	metadb_index_hash hash{};

	for (auto&& handle : handles)
	{
		if (MetadbIndex::client()->hashHandle(handle, hash) && hash_set.emplace(hash).second)
		{
			set_fields(hash, Fields());
		}
	}
}

void PlaybackStatistics::refresh(metadb_handle_list_cref handles)
{
	HashList hash_list;
	HashSet hash_set;
	metadb_index_hash hash{};

	for (auto&& handle : handles)
	{
		if (MetadbIndex::client()->hashHandle(handle, hash) && hash_set.emplace(hash).second)
		{
			hash_list.add_item(hash);
		}
	}

	api()->dispatch_refresh(guids::metadb_index, hash_list);
}

void PlaybackStatistics::set_fields(metadb_index_hash hash, const Fields& f)
{
	stream_writer_formatter_simple writer;
	writer << f.first_played;
	writer << f.last_played;
	writer << f.loved;
	writer << f.playcount;
	writer << f.rating;
	api()->set_user_data(guids::metadb_index, hash, writer.m_buffer.get_ptr(), writer.m_buffer.get_size());
}
