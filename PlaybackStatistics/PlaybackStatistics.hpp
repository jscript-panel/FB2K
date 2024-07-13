#pragma once

class PlaybackStatistics
{
public:
	struct Fields
	{
		uint32_t first_played{}, last_played{}, loved{}, playcount{}, rating{}, skipcount{};

		operator bool() const { return first_played || last_played || loved || playcount || rating || skipcount; }
	};

	struct TransactionScope
	{
		TransactionScope();
		~TransactionScope();

		metadb_index_transaction::ptr ptr;
	};

	using HashList = pfc::list_t<metadb_index_hash>;
	using HashSet = std::set<metadb_index_hash>;

	static Fields get_fields(metadb_index_hash hash);
	static metadb_index_manager::ptr api();
	static string8 timestamp_to_string(uint32_t ts);
	static uint32_t get_total_playcount(metadb_handle_list_cref handles, track_property_provider_v5_info_source& source);
	static uint32_t now();
	static uint32_t string_to_timestamp(std::string_view str);
	static void clear(metadb_handle_list_cref handles);
	static void refresh(const HashList& hash_list);
	static void refresh(metadb_handle_list_cref handles);
	static void set_fields(metadb_index_hash hash, const Fields& f, const metadb_index_transaction::ptr& transaction_ptr = dummy_transaction);

	inline static const metadb_index_transaction::ptr dummy_transaction;
};
