#include "stdafx.hpp"
#include "TitleFormatHook.hpp"

#include "CountryFlag.hpp"

bool TitleFormatHook::process_country_flag(titleformat_text_out* out, const char* func, titleformat_hook_function_params* params, bool& found_flag)
{
	if (!compare_string(func, "country_flag")) return false;

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

bool TitleFormatHook::process_field(titleformat_text_out*, const char*, size_t, bool& found_flag)
{
	found_flag = false;
	return false;
}

bool TitleFormatHook::process_font(titleformat_text_out* out, const char* func, titleformat_hook_function_params* params, bool& found_flag)
{
	if (!compare_string(func, "font")) return false;

	static constexpr fmt::string_view bel = "\x7";
	static constexpr fmt::string_view tab = "\t";

	if (m_param_count == 0)
	{
		found_flag = true;
		out->write(titleformat_inputtypes::unknown, fmt::format("{0}{0}", bel).c_str());
	}
	else if (m_param_count >= 2 && m_param_count <= 6)
	{
		const auto name = get_string(params, 0);
		const auto size = get_num(params, 1, 8, 144, 12);
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
	if (!compare_string(func, "jsp3_since")) return false;

	if (m_param_count == 1)
	{
		const auto date_string = get_string(params, 0);
		const auto wts = pfc::filetimestamp_from_string(date_string.c_str());

		if (wts != filetimestamp_invalid)
		{
			found_flag = true;
			const auto diff = init_time - pfc::fileTimeWtoU(wts);

			if (diff < day_in_seconds)
			{
				out->write(titleformat_inputtypes::unknown, "0d");
			}
			else
			{
				const auto dbl = static_cast<double>(diff);
				const auto since_string = pfc::format_time_ex(dbl, 0);
				const auto d = since_string.find_first('d') + 1;
				out->write(titleformat_inputtypes::unknown, since_string.subString(0, d));
			}
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
