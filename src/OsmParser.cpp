#include "OsmParser.h"

OsmParser::OsmParser()
	: open_element(CLOSED)
{
	set_substitute_entities(true);
}

std::vector<Glib::ustring> OsmParser::TagKeyNames()
{
	std::vector<Glib::ustring> r(TAG_KEY_END);
	r[NAME]        = "name";
	r[DESCRIPTION] = "description";
	r[NOTE]        = "note";
	r[REF]         = "ref";
	r[HIGHWAY]     = "highway";
	r[RAILWAY]     = "railway";
	r[WATERWAY]    = "waterway";
	r[LANDUSE]     = "landuse";
	r[AREA]        = "area";
	r[ONEWAY]      = "oneway";
	r[BRIDGE]      = "bridge";
	r[TUNNEL]      = "tunnel";
	r[LAYER]       = "layer";
	r[TRACKTYPE]   = "tracktype";
	r[ROUTE]       = "route";
	r[ACCESS]      = "access";
	r[JUNCTION]    = "junction";
	r[PLACE]       = "place";
	r[AMENITY]     = "amenity";
	r[LEISURE]     = "leisure";
	r[TOURISM]     = "tourism";
	r[MAN_MADE]    = "man_made";
	r[HISTORIC]    = "historic";
	r[SHOP]        = "shop";
	r[BARRIER]     = "barrier";
	r[BICYCLE]     = "bicycle";
	r[FOOT]        = "foot";
	r[CROSSING]    = "crossing";
	r[RELIGION]    = "religion";
	return r;
}

const std::vector<Glib::ustring> OsmParser::tag_keys
	= OsmParser::TagKeyNames();

void OsmParser::on_end_document()
{
	std::cout << "done!\n";
}

void OsmParser::on_start_element(
	const Glib::ustring& name,
	const AttributeList& properties
)
{
	using boost::lexical_cast;
	if(name == "node") {
		open_element = NODE;
		open_node = Node();
		for(SaxParser::Attribute attr : properties) {
			if(attr.name == "id")
				open_node.id = lexical_cast<uint64_t>(attr.value);
			else if(attr.name == "lat")
				open_node.lat = lexical_cast<double>(attr.value);
			else if(attr.name == "lon")
				open_node.lon = lexical_cast<double>(attr.value);
		}
	}
	else if(name == "way") {
		open_element = WAY;
	}
	else if(name == "relation") {
		open_element = RELATION;
	}
	else if(name == "tag" && open_element != CLOSED) {
		Tags& elem_tags = open_node.tags; // für ways erweitern
		Glib::ustring k;
		Glib::ustring v;
		for(SaxParser::Attribute attr : properties) {
			if(attr.name == "k")
				k = attr.value;
			else if(attr.name == "v")
				v = attr.value;
		}
		for(size_t i = 0; i < TAG_KEY_END; ++i) {
			if(tag_keys[i] == k) {
				elem_tags.push_back(Tags::value_type(TagKey(i), v));
			};
		}
	}; // if tag
}

void OsmParser::on_end_element(const Glib::ustring& name)
{
	if(name == "tag")
		return;

	switch(open_element) {
	case NODE:
		nodes.insert(std::pair<uint64_t, Node>(
			open_node.id, open_node
		));
		open_node.Output(std::cout);
		break;
	}

	open_element = CLOSED;
}

void OsmParser::Node::Output(std::ostream& out) const
{
	out << "id = " << id << ", lat = " << lat << ", lon = " << lon;
	if(!tags.empty()) {
		out << " tags = [";
		bool further = false;
		for(Tags::value_type tag : tags) {
			if(further) out << ", ";
			out << tag_keys.at(std::get<0>(tag)) << "=\"";
			out << std::get<1>(tag) << '"';
			further = true;
		};
		out << "]";
	}
	out << std::endl;
}
