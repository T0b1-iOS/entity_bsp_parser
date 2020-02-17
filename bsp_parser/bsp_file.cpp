#include <fstream>

#include "bsp_file.h"
#include "bsp_types.h"

using namespace bsp;

auto file::parse_map(const char *map_file, const bool optimized_read) -> bool
{
	auto file = std::ifstream{
	  map_file,
	  std::ios::binary | (optimized_read ? std::ios::beg : std::ios::ate)};
	if (!file.is_open())
	{
		std::printf("Failed to open File '%s'\n", map_file);
		return false;
	}

	if (!optimized_read)
	{
		const auto size = size_t(file.tellg());
		file.seekg(0u, std::ios::beg);

		_file_data.resize(size);
		file.read(reinterpret_cast<char *>(_file_data.data()), size);

		return parse_map(_file_data.data(), _file_data.size());
	}

	file_header_t file_header{};
	file.read(reinterpret_cast<char *>(&file_header), sizeof(file_header));

	if (std::string_view{file_header.magic, 4u} != "VBSP")
		return false;

	std::printf(
	  "Attempting to parse BSP File with Version %d and Map Revision %d\n",
	  file_header.version, file_header.map_revision);

	const auto ent_lump = file_header.lumps[LUMP_ENTITIES];

	_file_data.resize(ent_lump.length);
	file.seekg(ent_lump.offset, std::ios::beg);
	file.read(reinterpret_cast<char *>(_file_data.data()), ent_lump.length);

	parse_entities(_file_data.data(), ent_lump.length);

	return true;
}

auto file::parse_map(const uint8_t *data, const size_t size) -> bool
{
	const auto &file_header = *reinterpret_cast<const file_header_t *>(data);
	if (std::string_view{file_header.magic, 4u} != "VBSP")
		return false;

	std::printf(
	  "Attempting to parse BSP File with Version %d and Map Revision %d\n",
	  file_header.version, file_header.map_revision);

	const auto ent_lump = file_header.lumps[LUMP_ENTITIES];
	parse_entities(data + ent_lump.offset, ent_lump.length);

	return true;
}

void file::parse_entities(const uint8_t *lump_data, const size_t lump_size)
{
	auto lump_view = std::string_view{reinterpret_cast<const char *>(lump_data)};
	if (lump_view.size() > lump_size)
		return;

	size_t block_start;
	while ((block_start = lump_view.find('{')) != std::string::npos)
	{
		// let's hope there are no nested blocks
		const auto block_end = lump_view.find('}');

		const auto block_view =
		  lump_view.substr(block_start, block_end - block_start + 1u);
		auto key_values = detail::key_values{};
		if (!key_values.parse_from_string(block_view))
			continue;

		_entities.push_back(key_values);

		lump_view.remove_prefix(block_end + 1u);
	}

	std::printf("Parsed %d Entities\n", _entities.size());
}
