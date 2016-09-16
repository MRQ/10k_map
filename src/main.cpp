#include "libs.h"

#include "OsmParser.h"

// in Quantize.cpp
double Quantize(double d);

using namespace Osm;

volatile double center_lon = 0;
volatile double center_lat = 0;

std::tuple<double, double> LocalCoords(const Node& n)
{
	return std::tuple<double, double>(
		// cochem
		//36000 *(n.lon -7.168),
		//-60750 *(n.lat  -50.147)
		// aachen
		//36000 * (n.lon - 6.0998001),
		//-60750 * (n.lat - 50.7795198)
		36000 * (n.lon - center_lon),
		-60750 * (n.lat - center_lat)
	);
};


int main(int argc, char *argv[])
{
	using std::cin;

	if(argc >=3)
	{
		center_lat = boost::lexical_cast<double>(argv[1]);
		center_lon = boost::lexical_cast<double>(argv[2]);
	};

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

		bool jump = true;
		bool wrapper_written = false;
		for(uint64_t node_id : way.node_ids) {
			const Node& n = parser.FindNode(node_id);
			if(n.lon == 0 && n.lat == 0) {
				jump = true;
				continue;
			};
			std::tuple<double, double> xy = LocalCoords(n);
			if(
				std::abs(std::get<0>(xy)) +
				std::abs(std::get<1>(xy)) > 2000
			) {
				jump = true;
				continue;
			};
			if(!wrapper_written) {
				std::cout << "<path class=\"";
				std::cout << tag_class << "\" d=\"";
				wrapper_written = true;
			};
			std::cout << (jump ? "M " : "L ");
			jump = false;
			std::cout << Quantize(std::get<0>(xy)) << " ";
			std::cout << Quantize(std::get<1>(xy)) << " ";
		}
		if(wrapper_written)
			std::cout << "\" />\n";
	}
};
