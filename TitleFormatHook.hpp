#pragma once

class TitleFormatHook : public titleformat_hook
{
public:
	bool process_field(titleformat_text_out*, const char*, size_t, bool& found_flag) final;
	bool process_function(titleformat_text_out* out, const char* func, size_t, titleformat_hook_function_params* params, bool& found_flag) final;

private:
	bool process_font(titleformat_text_out* out, const char* func, titleformat_hook_function_params* params, bool& found_flag);
	bool process_country_flag(titleformat_text_out* out, const char* func, titleformat_hook_function_params* params, bool& found_flag);
	std::string get_string(titleformat_hook_function_params* params, size_t index);
	size_t get_num(titleformat_hook_function_params* params, size_t index, size_t min, size_t max, size_t fallback);

	size_t m_param_count{};
};
