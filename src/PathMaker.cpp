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
		"rw",
		"st",
		"te",
		"se",
		"pr",
		"mo",
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

		for(uint64_t node_id : way.node_ids) {
			const Osm::Node& n = osm_parser.FindNode(node_id);
			seg.steps.push_back(Step());
			Step* last_step = &(seg.steps.back());

			if(seg.steps.size() == 1) {
				new(last_step) MoveTo;
			}
			else {
				new(last_step) LineTo;
			};
			// todo: knickerkennung

			last_step->end = LocalCoords(n);

			if(n.refs > 1 && seg.steps.size() > 1) {
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
}

PathMaker::Segment::Class
PathMaker::Segment::FindClass(const Osm::Way& way)
{
	const std::string& hw = way.Tag(Osm::HIGHWAY);
	const std::string& rw = way.Tag(Osm::RAILWAY);
	const std::string& lu = way.Tag(Osm::LANDUSE);

	if(
		hw == "motorway" || hw == "motorway_link" ||
		hw == "trunk"    || hw == "trunk_link"
	)
		return HIGHWAY_TRUNK_MOTORWAY;
	if(hw == "primary"   || hw == "primary_link")
		return HIGHWAY_PRIMARY;
	if(hw == "secondary" || hw == "secondary_link")
		return HIGHWAY_SECONDARY;
	if(hw == "tertiary" || hw == "tertiary_link")
		return HIGHWAY_TERTIARY;
	if(
		hw == "unclassified"  || hw == "residential" ||
		hw == "living_street" || hw == "pedestrian"
	)
		return HIGHWAY_STREET;
	if(rw == "rail")
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
	// todo: landuse
	// todo: names
	for(size_t cls = Segment::WAY_BEGIN; cls < Segment::WAY_END; ++cls)
	{
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

			if(!data_open) {
				out << "<path class=\"" << Segment::class_names[cls];
				out << "\" d=\"";
				data_open = true;
			};
			for(size_t i = 0; i < seg->steps.size(); ++i)
			{
				if(i == 0 || i +1 == seg->steps.size())
					seg->steps[i].PathDataAbs(out, cmd, pos);
				else
					seg->steps[i].PathDataRel(out, cmd, pos);
			};

			last_seg = *seg;
		};
		if(data_open) {
			out << "\" />\n";
		};
	};
};

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
	auto new_pos = Quantize(end);
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
	if(prev_cmd != 'L')
		out << "L ";
	prev_pos = Quantize(end);
	out << prev_pos(0) << " ";
	out << prev_pos(1) << " ";
	prev_cmd = 'L';
};


void PathMaker::LineTo::PathDataRel(
	std::ostream& out,
	char& prev_cmd,
	Eigen::Vector2d& prev_pos
) const
{
	if(prev_cmd != 'l')
		out << "l ";
	Eigen::Vector2d rel = Quantize(end - prev_pos);
	out << rel(0) << " ";
	out << rel(1) << " ";
	prev_cmd = 'l';
	prev_pos += rel;
};

