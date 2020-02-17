#pragma once
#include <cstdint>
#include <vector>

#include "key_values.h"

namespace bsp
{
	struct file
	{
		file() = default;

		/**
		* \brief Parses a BSP file
		* \param map_file Path to file
		* \param optimized_read speeds up parsing by only reading the portions of the file that actually get parsed
		* \return whether the parsing was successful
		*/
		auto parse_map(const char *map_file, const bool optimized_read) -> bool;

		/**
		* \brief Parse a BSP file from data that's in memory.
		* IMPORTANT: Keep this buffer alive till u finished since the underlying code uses pointers into that buffer
		* \param data 
		* \param size 
		* \return whether the parsing was successful
		*/
		auto parse_map(const uint8_t *data, const size_t size) -> bool;

		void clear_internal_buffers()
		{
			_entities.clear();
			_file_data.clear();
		}

		[[nodiscard]] auto entities() const -> const auto & { return _entities; }

		protected:
		void parse_entities(const uint8_t *lump_data, const size_t lump_size);

		std::vector<uint8_t> _file_data{};
		std::vector<detail::key_values> _entities{};
	};
}  // namespace bsp
