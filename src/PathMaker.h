#include "libs.h"
#ifndef PathMaker_h_LVoC2gALP6pM
#define PathMaker_h_LVoC2gALP6pM

#include "OsmParser.h"

class PathMaker
{
public:
	void ImportSegments(OsmParser& osm_parser);
	//void Approximate();
	//void CombinePaths();
	void Output(std::ostream& out);

protected:

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
		Segment() : id(0), cls(DONT_DRAW) {};
		/// Order in this enum determines drawing order
		enum Class
		{
			DONT_DRAW,

			LANDUSE_BEGIN,
			LANDUSE_GREY,
			LANDUSE_END,

			WAY_BEGIN,
			RAILWAY,
			HIGHWAY_STREET,
			HIGHWAY_TERTIARY,
			HIGHWAY_SECONDARY,
			HIGHWAY_PRIMARY,
			HIGHWAY_TRUNK_MOTORWAY,
			WAY_END,
		};
		static const char* const class_names[];

		static Class FindClass(const Osm::Way& way);

		uint64_t id; ///< not unique because of segmentation
		Class cls;
		std::vector<Step> steps;
	};

	std::list<Segment> segments;
};

#endif // PathMaker_h_LVoC2gALP6pM
