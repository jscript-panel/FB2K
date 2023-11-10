#pragma once

class TrackPropertyCallback : public track_property_callback_v2
{
public:
	bool is_group_wanted(const char*) final
	{
		return true;
	}

	std::string to_string()
	{
		return m_data.dump(4, ' ', false, JSON::error_handler_t::ignore);
	}

	void set_property(const char* group, double, const char* name, const char* value) final
	{
		m_data[group][name] = value;
	}

private:
	JSON m_data;
};
