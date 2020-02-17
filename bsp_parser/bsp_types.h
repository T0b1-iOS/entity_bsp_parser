#pragma once
#include <cstdint>

namespace bsp
{
	// constants
	constexpr auto HEADER_LUMPS = 64u;

	// enums
	enum LUMP_TYPE
	{
		LUMP_ENTITIES,
		// ...
	};

	// structs
	struct lump_t
	{
		int32_t offset;  // byte offset into file
		int32_t length;  // length in bytes
		int32_t version;
		char ident_code[4];
	};

	struct file_header_t
	{
		char magic[4];
		int32_t version;
		lump_t lumps[HEADER_LUMPS];
		int32_t map_revision;
	};
}  // namespace bsp
