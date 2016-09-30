#include "libs.h"

#include "PathMaker.h"

// from main.cpp
Eigen::Vector2d LocalCoords(const Osm::Node& n);

// from Quantize.cpp
double Quantize(double d);
Eigen::Vector2d Quantize(Eigen::Vector2d xy);


const char* const PathMaker::Segment::class_names[] = {
		"dont_draw",
		"landuse_begin",
		"lg",
		"landuse_end",
		"way_begin",
		"se",
		"pr",
		"mo",
		"st",
		"te",
		"se",
		"pr",
		"mo",
		"rw",
		"way_end"
};

void PathMaker::ImportSegments(OsmParser& osm_parser)
{
	for(const Osm::Way& way : osm_parser.ways) {
		Segment::Class cls = Segment::FindClass(way);
		if(cls == Segment::DONT_DRAW)
			continue;

		Segment seg;
		seg.id = way.id;
		seg.cls = Segment::Class(cls);
		seg.area = (way.Tag(Osm::AREA) == "yes");
		if(way.Tag(Osm::SHOWNAME) == "yes")
			seg.name = way.Tag(Osm::NAME);

		bool jump = true;
		for(uint64_t node_id : way.node_ids) {
			const Osm::Node& n = osm_parser.FindNode(node_id);

			const auto local_coords = LocalCoords(n);
			if(OutOfRange(local_coords, cls)){
				jump = true;
				continue;
			};

			seg.steps.push_back(Step());
			Step* last_step = &(seg.steps.back());

			if(jump == true) {
				new(last_step) MoveTo;
				jump = false;
			}
			else {
				new(last_step) LineTo;
			};
			// todo: knickerkennung

			last_step->end = local_coords;

			if(n.refs > 1 && seg.steps.size() > 1) {
				// Knotenpunkt gefunden
				segments.push_back(seg);

				// neu initialisieren mit Startpunkt
				seg.steps.clear();
				seg.steps.shrink_to_fit();
				seg.steps.push_back(Step());
				Step* next_step = &(seg.steps.back());
				new(next_step) MoveTo;
				next_step->end = LocalCoords(n);
			};
		};
		if(seg.steps.size() > 1)
			segments.push_back(seg);
	};
};

bool PathMaker::OutOfRange(Eigen::Vector2d local_coords, Segment::Class cls)
{
	const double dist = local_coords.norm();

	switch(cls)
	{
	case Segment::HIGHWAY_STREET:
	case Segment::HIGHWAY_TERTIARY:
		return (dist > 1300);
	case Segment::HIGHWAY_SECONDARY:
	case Segment::HIGHWAY_SECONDARY_LINK:
	case Segment::HIGHWAY_PRIMARY:
	case Segment::HIGHWAY_PRIMARY_LINK:
	case Segment::HIGHWAY_TRUNK_MOTORWAY:
	case Segment::HIGHWAY_TRUNK_MOTORWAY_LINK:
	case Segment::RAILWAY:
		return (dist > 2100);
	default:
		return (dist > 500);
	};
};

void PathMaker::Approximate()
{
	using namespace std;
	for(Segment& seg : segments) {

		std::vector<Step> new_steps;
		new_steps.push_back(seg.steps.front());
		for(size_t i = 1; i +1 < seg.steps.size(); ++i) {
			auto d1 = seg.steps[i].end - new_steps.back().end;
			auto d2 = seg.steps[i +1].end - seg.steps[i].end;

			if(d1.dot(d2) > 0.99 * (d1.norm() * d2.norm()))
				continue;

			new_steps.push_back(seg.steps[i]);
		};
		new_steps.push_back(seg.steps.back());

		swap(new_steps, seg.steps);
	}
}

PathMaker::Segment::Class
PathMaker::Segment::FindClass(const Osm::Way& way)
{
	const std::string& hw = way.Tag(Osm::HIGHWAY);
	const std::string& rw = way.Tag(Osm::RAILWAY);
	const std::string& lu = way.Tag(Osm::LANDUSE);

	if(hw == "motorway" || hw == "trunk")
		return HIGHWAY_TRUNK_MOTORWAY;
	if(hw == "motorway_link" || hw == "trunk_link")
		return HIGHWAY_TRUNK_MOTORWAY_LINK;
	if(hw == "primary")
		return HIGHWAY_PRIMARY;
	if(hw == "primary_link")
		return HIGHWAY_PRIMARY_LINK;
	if(hw == "secondary")
		return HIGHWAY_SECONDARY;
	if(hw == "secondary_link")
		return HIGHWAY_SECONDARY_LINK;
	if(hw == "tertiary" || hw == "tertiary_link")
		return HIGHWAY_TERTIARY;
	if(
		hw == "unclassified"  || hw == "residential" ||
		hw == "living_street" || hw == "pedestrian"
	)
		return HIGHWAY_STREET;
	if(rw == "rail" && way.Tag(Osm::SERVICE).empty())
		return RAILWAY;
	if(
		lu == "brownfield"   || lu == "commercial" ||
		lu == "construction" || lu == "farm" ||
		lu == "farmyard"     || lu == "industrial" ||
		lu == "railway"      || lu == "residential" ||
		lu == "retail"       || lu == "under construction"
	)
		return LANDUSE_GREY;
	// else
	return DONT_DRAW;
};

void PathMaker::Output(std::ostream& out)
{
	out << "<g class=\"outline\"><use xlink:href=\"#snet\" /></g>";
	out << "<g class=\"fill\">\n";
	out << "<g id=\"snet\">\n";

	std::map<size_t, std::string> names;
	std::map<size_t, double> lengths;

	size_t id = 0;
	for(size_t cls = Segment::WAY_BEGIN; cls < Segment::WAY_END; ++cls)
	{
		double dummy = 0;
		double* length = &dummy;
		Segment last_seg;
		char cmd = '?';
		Eigen::Vector2d pos(0.0, 0.0);

		bool data_open = false;
		for(
			auto seg = segments.begin();
			seg != segments.end();
			seg++
		) {
			if(seg->cls != Segment::Class(cls))
				continue;

			if(last_seg.name != seg->name)
			{
				if(data_open)
					out << "\" />\n";
				data_open = false;
				length = &(lengths[id]);
				*length = 0;

				names[id] = seg->name;
			};

			static_assert(
				sizeof(Segment::class_names) / sizeof(char*)
				> Segment::WAY_END,
				"class_names has wrong size"
			);
			if(!data_open) {
				out << "<path class=\"" << Segment::class_names[cls] << "\" ";
				if(!seg->name.empty())
					out << "id=\"s" << id++ << "\" ";
				out << "d=\"";
				data_open = true;
			};
			for(size_t i = 0; i < seg->steps.size(); ++i)
			{
				auto prev_pos = seg->steps.front().end;
				if(i == 0 || i +1 == seg->steps.size())
					seg->steps[i].PathDataAbs(out, cmd, pos);
				else
					seg->steps[i].PathDataRel(out, cmd, pos);

				*length += (pos - prev_pos).norm();
			};

			last_seg = *seg;
		};
		if(data_open) {
			out << "\" />\n";
			data_open = false;
		};

		/*if(cls == Segment::LANDUSE_END) {
			out << "</g>\n<g id=\"snet\">\n";
		};*/
	};
	out << "</g>\n"; // id=snet
	out << "</g>\n"; // class=outline

	out << "<g id=\"names\">\n";
	for(auto name_id : names)
	{
		if(name_id.second.empty())
			continue;

		const double length = lengths[name_id.first];
		for(double offset = 30; offset + 500 < length; offset += 500) {
			out << "<text class=\"sn\" dy=\"5\">";
			out << "<textPath startOffset=\"";
			out << offset;
			out << "\" xlink:href=\"#s";
			out << name_id.first << "\">" << name_id.second;
			out << "</textPath></text>\n";
		};
	};
	out << "</g>\n";
};
/*
<text class="streetnames" dy="7">
    <textPath xlink:href="#s3" startOffset="100">Jülicherstraße</textPath>
</text>
*/

/* ------------------------------------------------------------------------- */

void PathMaker::Step::PathDataAbs(
	std::ostream& out,
	char& prev_,
	Eigen::Vector2d& prev_pos
) const
{
	throw std::runtime_error("soft-pure-virtual function called");
};

void PathMaker::Step::PathDataRel(
	std::ostream& out,
	char& prev_cmd,
	Eigen::Vector2d& prev_pos
) const
{
	throw std::runtime_error("soft-pure-virtual function called");
};

void PathMaker::MoveTo::PathDataAbs(
	std::ostream& out,
	char& prev_cmd,
	Eigen::Vector2d& prev_pos
) const
{
	const auto new_pos = Quantize(end);
	if(new_pos == prev_pos)
		return;

	out << "M ";
	out << new_pos(0) << " ";
	out << new_pos(1) << " ";
	prev_cmd = 'L';
	prev_pos = new_pos;
};

void PathMaker::MoveTo::PathDataRel(
	std::ostream& out,
	char& prev_cmd,
	Eigen::Vector2d& prev_pos
) const
{
	out << "m ";
	Eigen::Vector2d rel = Quantize(end - prev_pos);
	out << rel(0) << " ";
	out << rel(1) << " ";
	prev_cmd = 'l';
	prev_pos += rel;
};

void PathMaker::LineTo::PathDataAbs(
	std::ostream& out,
	char& prev_cmd,
	Eigen::Vector2d& prev_pos
) const
{
	const auto new_pos = Quantize(end);
	if(new_pos == prev_pos)
		return;

	if(prev_cmd != 'L')
		out << "L ";
	out << new_pos(0) << " ";
	out << new_pos(1) << " ";
	prev_cmd = 'L';
	prev_pos = new_pos;
};


void PathMaker::LineTo::PathDataRel(
	std::ostream& out,
	char& prev_cmd,
	Eigen::Vector2d& prev_pos
) const
{
	auto rel = Quantize(end - prev_pos);

	if(rel.squaredNorm() < 4*4)
		return;

	if(prev_cmd != 'l')
		out << "l ";
	out << rel(0) << " ";
	out << rel(1) << " ";
	prev_cmd = 'l';
	prev_pos += rel;
};

