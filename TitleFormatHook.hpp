#pragma once

class TitleFormatHook : public titleformat_hook
{
public:
	TitleFormatHook(size_t playlistIndex = SIZE_MAX);

	bool process_field(titleformat_text_out* out, const char* field, size_t, bool& found_flag) final;
	bool process_function(titleformat_text_out* out, const char* func, size_t, titleformat_hook_function_params* params, bool& found_flag) final;

private:
	bool process_font(titleformat_text_out* out, const char* func, titleformat_hook_function_params* params, bool& found_flag);
	bool process_country_flag(titleformat_text_out* out, const char* func, titleformat_hook_function_params* params, bool& found_flag);
	bool process_since(titleformat_text_out* out, const char* func, titleformat_hook_function_params* params, bool& found_flag);
	std::string get_string(titleformat_hook_function_params* params, size_t index);
	size_t get_num(titleformat_hook_function_params* params, size_t index, size_t min, size_t max, size_t fallback);

	size_t m_param_count{}, m_playlistIndex{};

	inline static const uint64_t init_time = pfc::fileTimeWtoU(pfc::fileTimeNow());
	inline static constexpr uint64_t day_in_seconds = 24 * 60 * 60;
	inline static constexpr uint64_t week_in_seconds = 7 * day_in_seconds;
	inline static constexpr uint64_t month_in_seconds = 30 * day_in_seconds;
	inline static constexpr uint64_t year_in_seconds = 365 * day_in_seconds;
};
