#include <array>
#include <chrono>

#include "bsp_parser/key_values.h"
#include "bsp_parser/bsp_file.h"

using namespace bsp::detail;

std::array<std::string_view, VALUE_SIZE> value_strs = {"INT", "FLOAT", "STR",
                                                       "VEC"};

void print_value(const key_value_pair_t &pair)
{
	switch (pair.type)
	{
		case VALUE_INT: std::printf("%d", pair.int_val); break;
		case VALUE_FLOAT: std::printf("%.2f", pair.float_val); break;
		case VALUE_STRING:
			std::printf("%.*s", static_cast<int>(pair.str_val.size()),
			            pair.str_val.data());
			break;
		case VALUE_VECTOR:
			for (auto i = 0u; i < pair.vec_val.vec_len; ++i)
				std::printf("%.*s%.2f", (!i ? 0 : 1), " ", pair.vec_val.values[i]);
	}

	const auto type_name = value_strs[pair.type];
	if (pair.type != VALUE_VECTOR)
		std::printf(" (%.*s)", static_cast<int>(type_name.size()),
		            type_name.data());
	else
		std::printf(" (%.*s%d)", static_cast<int>(type_name.size()),
		            type_name.data(), pair.vec_val.vec_len);
}

static const auto file_path = R"(<Path to BSP>)";

int main()
{
	using namespace std::chrono;
	const auto ms = duration_cast<milliseconds>(high_resolution_clock::now().time_since_epoch()).count();
	
	bsp::file file{};
	const auto success = file.parse_map(file_path, true);
	const auto diff = duration_cast<milliseconds>(high_resolution_clock::now().time_since_epoch()).count() - ms;
	
	if (!success)
		std::printf("File parsing failed!\n");
	else
	{
		std::printf("File parsing successful!\n");
		std::printf("Dumping Entities...\n");

		for(const auto &entity : file.entities())
		{
			std::printf("------------------------\n");

			for(const auto &key_value : entity.values())
			{
				std::printf(" Key '%.*s': ", static_cast<int>(key_value.name.size()),
					key_value.name.data());
				print_value(key_value);
				std::printf("\n");
			}
			
			std::printf("------------------------\n");
		}
	}

	std::printf("Parsing took %llums", diff);
	
	std::getchar();

	return 0;
}