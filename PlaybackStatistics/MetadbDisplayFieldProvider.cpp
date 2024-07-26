#include "stdafx.hpp"

namespace
{
	using namespace std::literals::string_view_literals;

	static constexpr std::array field_names =
	{
		"jsp3_first_played"sv,
		"jsp3_last_played"sv,
		"jsp3_loved"sv,
		"jsp3_playcount"sv,
		"jsp3_rating"sv,
		"jsp3_skipcount"sv,
	};

	class MetadbDisplayFieldProvider : public metadb_display_field_provider_v2
	{
	public:
		bool process_field(uint32_t index, metadb_handle* handle, titleformat_text_out* out) final
		{
			return process_field_v2(index, handle, handle->query_v2_(), out);
		}

		bool process_field_v2(uint32_t index, metadb_handle* handle, const metadb_v2::rec_t& rec, titleformat_text_out* out) final
		{
			if (rec.info.is_empty()) return false;

			const auto hash = MetadbIndex::client()->transform(rec.info->info(), handle->get_location());
			const auto f = PlaybackStatistics::get_fields(hash);

			switch (index)
			{
			case 0:
				if (f.first_played == 0) return false;
				out->write(titleformat_inputtypes::meta, PlaybackStatistics::timestamp_to_string(f.first_played));
				return true;
			case 1:
				if (f.last_played == 0) return false;
				out->write(titleformat_inputtypes::meta, PlaybackStatistics::timestamp_to_string(f.last_played));
				return true;
			case 2:
				if (f.loved == 0) return false;
				out->write_int(titleformat_inputtypes::meta, f.loved);
				return true;
			case 3:
				if (f.playcount == 0) return false;
				out->write_int(titleformat_inputtypes::meta, f.playcount);
				return true;
			case 4:
				if (f.rating == 0) return false;
				out->write_int(titleformat_inputtypes::meta, f.rating);
				return true;
			case 5:
				if (f.skipcount == 0) return false;
				out->write_int(titleformat_inputtypes::meta, f.skipcount);
				return true;
			}
			return false;
		}

		uint32_t get_field_count() final
		{
			return js::sizeu(field_names);
		}

		void get_field_name(uint32_t index, pfc::string_base& out) final
		{
			out = field_names[index];
		}
	};

	FB2K_SERVICE_FACTORY(MetadbDisplayFieldProvider);
}
