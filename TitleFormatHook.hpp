#pragma once

class TitleFormatHook : public titleformat_hook
{
public:
	bool process_field(titleformat_text_out*, const char*, size_t, bool& found_flag) final;
	bool process_function(titleformat_text_out* out, const char* name, size_t, titleformat_hook_function_params* params, bool& found_flag) final;
};
