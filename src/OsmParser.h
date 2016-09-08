#include "libs.h"

class OsmParser : public xmlpp::SaxParser
{
public:
	OsmParser();
	virtual ~OsmParser() {};

protected:
	// -- Types --
	enum Element
	{
		CLOSED, // no open tag
		NODE,
		WAY,
		RELATION,
		OTHER
	};
	enum TagKey // keys for OSM-Tags
	{
		// - general -
		NAME = 1,
		DESCRIPTION,
		NOTE,
		REF,
		// - ways -
		HIGHWAY,
		RAILWAY,
		WATERWAY,
		LANDUSE,
		AREA,
		ONEWAY,
		BRIDGE,
		TUNNEL,
		LAYER,
		TRACKTYPE,
		ROUTE,
		ACCESS,
		JUNCTION,
		// - node -
		PLACE,
		AMENITY,
		LEISURE,
		TOURISM,
		MAN_MADE,
		HISTORIC,
		SHOP,
		BARRIER,
		BICYCLE,
		FOOT,
		CROSSING,
		RELIGION,
		TAG_KEY_END
	};
	typedef std::vector<std::tuple<TagKey, std::string> > Tags;
	struct Node
	{
		Node() : id(0), lat(0), lon(0) {};
		void Output(std::ostream& out) const;

		uint64_t id;
		double lat;
		double lon;
		Tags tags;
	};

	// -- Functions --
	static std::vector<Glib::ustring> TagKeyNames();
	//overrides:
	virtual void on_end_document();
	virtual void on_start_element(
		const Glib::ustring& name,
		const AttributeList& properties
	);
	virtual void on_end_element(const Glib::ustring& name);

	// -- Variables --
	static const std::vector<Glib::ustring> tag_keys;
	Element open_element; ///< currently processed XML element
	Node open_node; ///< currently processed node (if valid)

	std::map<uint64_t, Node> nodes;
};

