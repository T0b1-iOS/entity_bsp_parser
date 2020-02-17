#include <algorithm>
#include <array>
#include <charconv>

#include "key_values.h"

using namespace bsp::detail;

namespace
{
	std::vector<std::string_view> line_buffer{};

	void trim_whitespaces(std::string_view &str)
	{
		const auto begin = str.find_first_not_of(" \t");
		if (begin == std::string::npos)
		{
			str = {};
			return;
		}
		const auto end = str.find_last_not_of(" \t");

		str = str.substr(begin, (end - begin + 1u));
	}
}  // namespace

auto key_values::parse_from_string(const std::string_view str) -> bool
{
	split_lines(str);

	// trim lines
	std::for_each(line_buffer.begin(), line_buffer.end(), trim_whitespaces);

	line_buffer.erase( std::remove_if(line_buffer.begin(), line_buffer.end(), [](const auto &str_view) { return str_view.empty(); }), line_buffer.end());
	
	if (line_buffer[0u][0u] != '{')
		return false;

	if (line_buffer.back()[0u] != '}')
		return false;

	for (auto i = 1u; i < (line_buffer.size() - 1u); ++i)
		parse_line(line_buffer[i]);

	return true;
}

void key_values::split_lines(std::string_view str)
{
	line_buffer.clear();

	// TODO: windows line endings

	size_t pos;
	while ((pos = str.find('\n')) != std::string::npos)
	{
		const auto line = str.substr(0u, pos);
		str             = str.substr(pos + 1u);

		if (!line.empty())
			line_buffer.push_back(line);
	}

	if (!str.empty())
		line_buffer.push_back(str);
}

void key_values::parse_line(std::string_view line)
{
	if (line.empty())
		return;

	// Line layout: '"<key>" "<value>"'
	// parse key
	auto key = std::string_view{};
	{
		if (line[0u] != '"')
			return;

		line.remove_prefix(1u);
		const auto key_end_pos =
		  line.find('"');  // TODO: idk if u can escape " like \"
		key = line.substr(0u, key_end_pos);
		line.remove_prefix(key_end_pos + 1u);
	}

	if (key.empty())
		return;

	// now what's left is '<whitespaces>"<value>"'
	trim_whitespaces(
	  line);  // this also trims the back which is unnecessary but o well

	// now it's '"<value>"' (hopefully)
	if (line[0u] != '"' || line.back() != '"')
		return;

	line.remove_prefix(1u);
	line.remove_suffix(1u);

	// now it's '<value>'

	auto key_value_pair = key_value_pair_t{};
	key_value_pair.name = key;
	parse_value(line, key_value_pair);

	_values.push_back(key_value_pair);
}

namespace
{
	template<typename type>
	std::optional<type> parse_str(const std::string_view value)
	{
		type val;
		const auto first = value.data(), last = first + value.size();

		const auto res = std::from_chars(first, last, val);
		if (res.ec != static_cast<std::errc>(0) || res.ptr != last)
			return {};

		return val;
	}

	void parse_non_vec(const std::string_view value, key_value_pair_t &pair)
	{
		// check if it's an int
		const auto int_val = parse_str<int>(value);
		if (int_val.has_value())
		{
			pair.type    = VALUE_INT;
			pair.int_val = *int_val;
			return;
		}

		// check if it's a float
		const auto float_val = parse_str<float>(value);
		if (float_val.has_value())
		{
			pair.type      = VALUE_FLOAT;
			pair.float_val = *float_val;
			return;
		}

		// it's a string then
		pair.type    = VALUE_STRING;
		pair.str_val = value;
	}

	void parse_vec(const std::string_view value,
	               const std::array<std::string_view, 4u> &parts,
	               const size_t parts_len, key_value_pair_t &pair)
	{
		auto vec_val = vector_t{};

		// iterate all parts, check if they are all floats
		auto all_float = true;
		for (auto i = 0u; i < parts_len; ++i)
		{
			const auto float_val = parse_str<float>(parts[i]);
			if (!float_val.has_value())
			{
				all_float = false;
				break;
			}

			vec_val.values[i] = *float_val;
			vec_val.vec_len++;
		}

		if (all_float)
		{
			pair.type    = VALUE_VECTOR;
			pair.vec_val = vec_val;
			return;
		}

		// otherwise it's a string
		pair.type    = VALUE_STRING;
		pair.str_val = value;
	}
}  // namespace

void key_values::parse_value(std::string_view value, key_value_pair_t &pair)
{
	// first check for spaces in case it's a vec
	auto parts     = std::array<std::string_view, 4u>{};
	auto parts_len = 0u;
	{
		size_t pos;
		while ((pos = value.find(' ')) != std::string::npos)
		{
			// TODO: trim part? otherwise parsing of vectors may fail
			const auto part = value.substr(0u, pos);
			value           = value.substr(pos + 1u);

			parts[parts_len] = part;
			parts_len++;

			if (parts_len == 4u)
				break;
		}

		if (parts_len > 0 && !value.empty() && parts_len != 4u)
		{
			parts[parts_len] = value;
			parts_len++;
		}
	}

	// is it not a vector?
	if (!parts_len)
		parse_non_vec(value, pair);
	else
		parse_vec(value, parts, parts_len, pair);
}
