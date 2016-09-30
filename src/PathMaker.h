#include "libs.h"
#ifndef PathMaker_h_LVoC2gALP6pM
#define PathMaker_h_LVoC2gALP6pM

#include "OsmParser.h"

class PathMaker
{
public:
	void ImportSegments(OsmParser& osm_parser);
	void Approximate();
	//void CombinePaths();
	void Output(std::ostream& out);

protected:
	friend class OsmParser;

	class Step
	{
	public:
		Step() {};
		Step(const Step& other)
		{
			std::memcpy(this, &other, sizeof(*this));
		}

		Eigen::Vector2d end;

		virtual void PathDataAbs(
			std::ostream&,
			char& prev_cmd,
			Eigen::Vector2d& prev_pos
		) const;
		virtual void PathDataRel(
			std::ostream&,
			char& prev_cmd,
			Eigen::Vector2d& prev_pos
		) const;

	private:
	};

	class MoveTo : public Step
	{
	public:
		virtual void PathDataAbs(
			std::ostream&,
			char& prev_cmd,
			Eigen::Vector2d& prev_pos
		) const;
		virtual void PathDataRel(
			std::ostream&,
			char& prev_cmd,
			Eigen::Vector2d& prev_pos
		) const;
	};

	class LineTo : public Step
	{
	public:
		virtual void PathDataAbs(
			std::ostream&,
			char& prev_cmd,
			Eigen::Vector2d& prev_pos
		) const;
		virtual void PathDataRel(
			std::ostream&,
			char& prev_cmd,
			Eigen::Vector2d& prev_pos
		) const;
	};

	struct Segment
	{
		Segment() : id(0), cls(DONT_DRAW), area(false) {};
		/// Order in this enum determines drawing order
		enum Class
		{
			DONT_DRAW,
			LANDUSE_BEGIN,
			LANDUSE_GREY,
			LANDUSE_END,
			WAY_BEGIN,
			HIGHWAY_SECONDARY_LINK,
			HIGHWAY_PRIMARY_LINK,
			HIGHWAY_TRUNK_MOTORWAY_LINK,
			HIGHWAY_STREET,
			HIGHWAY_TERTIARY,
			HIGHWAY_SECONDARY,
			HIGHWAY_PRIMARY,
			HIGHWAY_TRUNK_MOTORWAY,
			RAILWAY,
			WAY_END,
		};
		static const char* const class_names[];

		static Class FindClass(const Osm::Way& way);

		uint64_t id; ///< not unique because of segmentation
		Class cls;
		bool area;
		std::string name;
		std::vector<Step> steps;
	};

	static bool OutOfRange(Eigen::Vector2d local_coords, Segment::Class cls);

	std::list<Segment> segments;
};

#endif // PathMaker_h_LVoC2gALP6pM
