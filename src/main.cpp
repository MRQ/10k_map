#include "libs.h"

#include "OsmParser.h"

int main()
{
	using std::cin;

	OsmParser parser;
	char buffer[128];
	const size_t buffer_size = sizeof(buffer) / sizeof(char);

	while(cin.good())
	{
		std::memset(buffer, 0, buffer_size);
		cin.read(buffer, buffer_size -1);
		Glib::ustring chunk(buffer, buffer+cin.gcount());
		parser.parse_chunk(chunk);
	};
};
