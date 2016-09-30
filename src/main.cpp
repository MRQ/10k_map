#include "libs.h"

#include "OsmParser.h"
#include "PathMaker.h"

// in Quantize.cpp
double Quantize(double d);

using namespace Osm;

volatile double center_lon = 0;
volatile double center_lat = 0;

Eigen::Vector2d LocalCoords(const Node& n)
{
	return Eigen::Vector2d(
		// cochem
		//36000 *(n.lon -7.168),
		//-60750 *(n.lat  -50.147)
		// aachen
		//36000 * (n.lon - 6.0998001),
		//-60750 * (n.lat - 50.7795198)
		68312 * (n.lon - center_lon),
		-103748 * (n.lat - center_lat)
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

	PathMaker path_maker;
	path_maker.ImportSegments(parser);
	path_maker.Approximate();
	path_maker.Output(std::cout);

	return 0;
};
