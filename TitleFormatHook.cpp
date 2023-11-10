#include "stdafx.hpp"
#include "TitleFormatHook.hpp"

bool TitleFormatHook::process_field(titleformat_text_out*, const char*, size_t, bool& found_flag)
{
	found_flag = false;
	return false;
}

bool TitleFormatHook::process_function(titleformat_text_out* out, const char* name, size_t, titleformat_hook_function_params* params, bool& found_flag)
{
	found_flag = false;
	if (stricmp_utf8(name, "font") != 0) return false;

	static constexpr fmt::string_view bel = "\x7";
	static constexpr fmt::string_view tab = "\t";
	const size_t count = params->get_param_count();

	if (count == 0)
	{
		found_flag = true;
		out->write(titleformat_inputtypes::unknown, fmt::format("{0}{0}", bel).c_str());
	}
	else if (count >= 2 && count <= 6)
	{
		found_flag = true;

		const auto get_value_checked = [params, count](size_t index, size_t min, size_t max, size_t fallback)
			{
				if (index < count)
				{
					const size_t value = params->get_param_uint(index);
					if (value >= min && value <= max) return value;
				}
				return fallback;
			};

		const char* font;
		size_t font_length{};
		params->get_param(0, font, font_length);

		const size_t size = get_value_checked(1, 8, 144, 12);
		const size_t weight = get_value_checked(2, 100, 950, 400);
		const size_t style = get_value_checked(3, 0, 2, 0);
		const size_t underline = get_value_checked(4, 0, 1, 0);
		const size_t strikethrough = get_value_checked(5, 0, 1, 0);
		const std::array nums = { size, weight, style, underline, strikethrough };

		out->write(titleformat_inputtypes::unknown, fmt::format("{}{}{}{}{}", bel, font, tab, fmt::join(nums, tab), bel).c_str());
	}

	return true;
}
