#include "libs.h"

namespace Osm
{
enum TagKey // keys for OSM-Tags
{
	NOT_FOUND = 0,
	ACCESS,
	AMENITY,
	AREA,
	BARRIER,
	BICYCLE,
	BRIDGE,
	CROSSING,
	DESCRIPTION,
	FOOT,
	HIGHWAY,
	HISTORIC,
	JUNCTION,
	LANDUSE,
	LAYER,
	LEISURE,
	MAN_MADE,
	NAME,
	NOTE,
	ONEWAY,
	PLACE,
	RAILWAY,
	REF,
	RELIGION,
	ROUTE,
	SHOP,
	TOURISM,
	TRACKTYPE,
	TUNNEL,
	WATERWAY,
	TAG_KEY_END
};
typedef std::vector<std::tuple<TagKey, std::string> > Tags;
struct Node
{
	Node() : id(0), lat(0), lon(0) {};
	void Output(std::ostream& out) const;
	const std::string& Tag(TagKey) const;

	uint64_t id;
	double lat;
	double lon;
	Tags tags;

private:
	static const std::string empty;
};
struct Way
{
	Way() : id(0) {};
	const std::string& Tag(TagKey) const;

	uint64_t id;
	std::vector<uint64_t> node_ids;
	Tags tags;

private:
	static const std::string empty;
};

} // namespace OSM

class OsmParser : public xmlpp::SaxParser
{
public:
	OsmParser();
	virtual ~OsmParser() {};
	const Osm::Node& FindNode(uint64_t id);

	std::vector<Osm::Way> ways;
	std::vector<Osm::Node> nodes;

protected:
	friend class Osm::Node;
	enum Element
	{
		CLOSED, // no open tag
		NODE,
		WAY,
		RELATION,
		OTHER
	};


	// -- Functions --
	static std::vector<std::string> TagKeyNames();
	Osm::TagKey FindTagKey(const Glib::ustring& s);

	//overrides:
	virtual void on_start_element(
		const Glib::ustring& name,
		const AttributeList& properties
	);
	virtual void on_end_element(const Glib::ustring& name);

	// -- Variables --
	static const std::vector<std::string> tag_keys;

	Element open_element; ///< currently processed XML element
	Osm::Node open_node; ///< currently processed node (if valid)
	Osm::Way open_way; ///< currently processed way
};

