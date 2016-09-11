#include "libs.h"

#include "OsmParser.h"

using namespace Osm;

std::tuple<double, double> LocalCoords(const Node& n)
{
	return std::tuple<double, double>(
//		500 + 24000 *(n.lon -7.1809),
//		500 - 40500 *(n.lat  -50.1513)
		500 + 36000 *(n.lon -7.168),
		500 - 60750 *(n.lat  -50.147)
	);
};

int main()
{
	using std::cin;

	OsmParser parser;
	char buffer[512];
	const size_t buffer_size = sizeof(buffer) / sizeof(char);

	while(cin.good()) {
		std::memset(buffer, 0, buffer_size);
		cin.read(buffer, buffer_size -1);
		Glib::ustring chunk(buffer, buffer+cin.gcount());
		parser.parse_chunk(chunk);
	};

	for(const Way& way : parser.ways) {
		const std::string& hw = way.Tag(HIGHWAY);
		if(hw.empty()) // short cut
			continue;

		std::string tag_class;
		if(hw == "motorway" || hw == "motorway_link") {
			tag_class = "m";
		}
		else if(hw == "trunk" || hw == "trunk_link") {
			tag_class = "t";
		}
		else if(hw == "primary" || hw == "primary_link") {
			tag_class = "p";
		}
		else if(hw == "secondary" || hw == "secondary_link") {
			tag_class = "s";
		}
		else if(hw == "tertiary" || hw == "tertiary_link") {
			tag_class = "d";
		}
		else if(
			hw == "unclassified"  || hw == "residential" ||
			hw == "living_street" || hw == "pedestrian"
		) {
			tag_class = "r";
		};

		if(tag_class.empty())
			continue;

		std::cout << "<path class=\"" << tag_class << "\" d=\"";
		bool jump = true;
		for(uint64_t node_id : way.node_ids) {
			const Node& n = parser.FindNode(node_id);
			if(n.lon == 0 && n.lat == 0) {
				jump = true;
				continue;
			};
			std::tuple<double, double> xy = LocalCoords(n);
			std::cout << (jump ? "M " : "L ");
			jump = false;
			std::cout << std::floor(std::get<0>(xy)) << " ";
			std::cout << std::floor(std::get<1>(xy)) << " ";
		}
		std::cout << "\" />\n";
	}
};
