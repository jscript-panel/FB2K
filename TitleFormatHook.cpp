#include "stdafx.hpp"
#include "TitleFormatHook.hpp"

#include "CountryFlag.hpp"

TitleFormatHook::TitleFormatHook(size_t playlistIndex) : m_playlistIndex(playlistIndex) {}

bool TitleFormatHook::process_country_flag(titleformat_text_out* out, const char* func, titleformat_hook_function_params* params, bool& found_flag)
{
	if (!js::compare_string(func, "country_flag")) return false;

	if (m_param_count == 1)
	{
		const auto country_or_code = get_string(params, 0);
		const auto flag_code = CountryFlag::get(country_or_code);
		found_flag = flag_code.length() > 0;

		if (found_flag)
		{
			out->write(titleformat_inputtypes::unknown, flag_code.c_str());
		}
	}

	return true;
}

bool TitleFormatHook::process_field(titleformat_text_out* out, const char* field, size_t, bool& found_flag)
{
	found_flag = false;

	if (js::compare_string(field, "jsp3_playlist_name"))
	{
		found_flag = m_playlistIndex < Plman::api()->get_playlist_count();

		if (found_flag)
		{
			string8 str;
			Plman::api()->playlist_get_name(m_playlistIndex, str);
			out->write(titleformat_inputtypes::unknown, str);
		}

		return true;
	}
	else if (js::compare_string(field, "fb2k_profile_path"))
	{
		found_flag = true;
		static const string8 path = js::from_wide(Path::profile());
		out->write(titleformat_inputtypes::unknown, path);
		return true;
	}

	return false;
}

bool TitleFormatHook::process_font(titleformat_text_out* out, const char* func, titleformat_hook_function_params* params, bool& found_flag)
{
	if (!js::compare_string(func, "font")) return false;

	static constexpr fmt::string_view bel = "\x7";
	static constexpr fmt::string_view tab = "\t";
	static const auto DPI = QueryScreenDPI(Fb::wnd());

	if (m_param_count == 0)
	{
		found_flag = true;
		out->write(titleformat_inputtypes::unknown, fmt::format("{0}{0}", bel).c_str());
	}
	else if (m_param_count >= 2 && m_param_count <= 6)
	{
		const auto name = get_string(params, 0);
		const auto size = get_num(params, 1, 8, 144, 12) * DPI / 72;
		const auto weight = get_num(params, 2, 100, 950, 400);
		const auto style = get_num(params, 3, 0, 2, 0);
		const auto underline = get_num(params, 4, 0, 1, 0);
		const auto strikethrough = get_num(params, 5, 0, 1, 0);
		const std::array nums = { size, weight, style, underline, strikethrough };

		found_flag = true;
		out->write(titleformat_inputtypes::unknown, fmt::format("{}{}{}{}{}", bel, name, tab, fmt::join(nums, tab), bel).c_str());
	}

	return true;
}

bool TitleFormatHook::process_function(titleformat_text_out* out, const char* func, size_t, titleformat_hook_function_params* params, bool& found_flag)
{
	found_flag = false;
	m_param_count = params->get_param_count();

	if (process_font(out, func, params, found_flag)) return true;
	if (process_country_flag(out, func, params, found_flag)) return true;
	if (process_since(out, func, params, found_flag)) return true;
	return false;
}

bool TitleFormatHook::process_since(titleformat_text_out* out, const char* func, titleformat_hook_function_params* params, bool& found_flag)
{
	if (!js::compare_string(func, "jsp3_since")) return false;

	if (m_param_count == 1)
	{
		const auto date_string = get_string(params, 0);
		const auto ts = PlaybackStatistics::string_to_timestamp(date_string);
		const auto now = PlaybackStatistics::now();

		if (ts < now)
		{
			found_flag = true;

			string8 str;
			uint32_t diff = now - ts;

			if (diff < day_in_seconds * 2)
			{
				const auto today_string = PlaybackStatistics::timestamp_to_string(now).subString(0, 10);
				const auto yesterday_string = PlaybackStatistics::timestamp_to_string(now - day_in_seconds).subString(0, 10);

				if (date_string.starts_with(today_string.get_ptr()))
				{
					str = "Today";
				}
				else if (date_string.starts_with(yesterday_string.get_ptr()))
				{
					str = "Yesterday";
				}
				else
				{
					str = "1d";
				}
			}
			else
			{
				bool include_weeks_days = true;

				const auto years = diff / year_in_seconds;
				if (years > 0)
				{
					include_weeks_days = false;
					diff -= years * year_in_seconds;
					str << years << "y ";
				}

				const auto months = diff / month_in_seconds;
				if (months > 0)
				{
					diff -= months * month_in_seconds;
					str << months << "m ";
				}

				if (include_weeks_days)
				{
					const auto weeks = diff / week_in_seconds;
					if (weeks > 0)
					{
						diff -= weeks * week_in_seconds;
						str << weeks << "w ";
					}

					const auto days = diff / day_in_seconds;
					if (days > 0)
					{
						str << days << "d";
					}
				}
			}

			out->write(titleformat_inputtypes::unknown, str.trim(' '));
		}
	}
	return true;
}

std::string TitleFormatHook::get_string(titleformat_hook_function_params* params, size_t index)
{
	if (index < m_param_count)
	{
		const char* str;
		size_t len{};
		params->get_param(index, str, len);

		return std::string(str, len);
	}
	return std::string();
}

size_t TitleFormatHook::get_num(titleformat_hook_function_params* params, size_t index, size_t min, size_t max, size_t fallback)
{
	if (index < m_param_count)
	{
		const auto value = params->get_param_uint(index);
		if (value >= min && value <= max) return value;
	}
	return fallback;
}
