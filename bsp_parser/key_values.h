#pragma once

#include <string_view>
#include <vector>
#include <optional>

namespace bsp::detail
{
	enum VALUE_TYPE
	{
		VALUE_INT,
		VALUE_FLOAT,
		VALUE_STRING,
		VALUE_VECTOR,

		VALUE_SIZE
	};

	struct vector_t
	{
		union
		{
			struct
			{
				float x, y, z, w;
			};
			float values[4] = {0.f};
		};
		uint8_t vec_len = 0u;
	};

	struct key_value_pair_t
	{
		// string_views into the file buffer to not allocate that much
		std::string_view name;

		VALUE_TYPE type;
		union
		{
			int32_t int_val;
			float float_val;
			std::string_view str_val;
			vector_t vec_val;
		};
	};

	struct key_values
	{
		key_values() = default;

		/**
		* \brief Parses a KeyValue block (encapsulated in '{}')
		* \param str buffer that holds one(!) KeyValue block
		* \return whether the string contained a valid KeyValue block
		*/
		auto parse_from_string(std::string_view str) -> bool;

		[[nodiscard]] auto find_pair(const std::string_view key) const
		  -> std::optional<key_value_pair_t>
		{
			for (const auto &pair : _values)
			{
				if (pair.name == key)
					return pair;
			}

			return {};
		}

		[[nodiscard]] auto values() const -> const auto & { return _values; }

		protected:
		void split_lines(std::string_view str);

		void parse_line(std::string_view line);
		void parse_value(std::string_view value, key_value_pair_t &pair);

		std::vector<key_value_pair_t> _values{};
	};
}  // namespace bsp::detail
