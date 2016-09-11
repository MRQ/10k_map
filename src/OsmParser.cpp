#include "OsmParser.h"

using namespace Osm;

OsmParser::OsmParser()
	: open_element(CLOSED)
{
	set_substitute_entities(true);
}

std::vector<std::string> OsmParser::TagKeyNames()
{
	std::vector<std::string> r(TAG_KEY_END);
	r[ACCESS]      = "access";
	r[AMENITY]     = "amenity";
	r[AREA]        = "area";
	r[BARRIER]     = "barrier";
	r[BICYCLE]     = "bicycle";
	r[BRIDGE]      = "bridge";
	r[CROSSING]    = "crossing";
	r[DESCRIPTION] = "description";
	r[FOOT]        = "foot";
	r[HIGHWAY]     = "highway";
	r[HISTORIC]    = "historic";
	r[JUNCTION]    = "junction";
	r[LANDUSE]     = "landuse";
	r[LAYER]       = "layer";
	r[LEISURE]     = "leisure";
	r[MAN_MADE]    = "man_made";
	r[NAME]        = "name";
	r[NOTE]        = "note";
	r[ONEWAY]      = "oneway";
	r[PLACE]       = "place";
	r[RAILWAY]     = "railway";
	r[REF]         = "ref";
	r[RELIGION]    = "religion";
	r[ROUTE]       = "route";
	r[SHOP]        = "shop";
	r[TOURISM]     = "tourism";
	r[TRACKTYPE]   = "tracktype";
	r[TUNNEL]      = "tunnel";
	r[WATERWAY]    = "waterway";
	return r;
}

const std::vector<std::string> OsmParser::tag_keys
	= OsmParser::TagKeyNames();

TagKey OsmParser::FindTagKey(const Glib::ustring& s)
{
	const std::string& raw = s.raw();
	if(raw.empty())
		return NOT_FOUND;

	const char first = raw[0];
	TagKey begin = NOT_FOUND;
	TagKey end = TAG_KEY_END;
	switch(first)
	{
	// This switch-case should be optimized to a lookup table by the compiler.
	case 'a': begin = ACCESS;      end = BARRIER;     break;
	case 'b': begin = BARRIER;     end = CROSSING;    break;
	case 'c': begin = CROSSING;    end = DESCRIPTION; break;
	case 'd': begin = DESCRIPTION; end = FOOT;        break;
	case 'f': begin = FOOT;        end = HIGHWAY;     break;
	case 'h': begin = HIGHWAY;     end = JUNCTION;    break;
	case 'j': begin = JUNCTION;    end = LANDUSE;     break;
	case 'l': begin = LANDUSE;     end = MAN_MADE;    break;
	case 'm': begin = MAN_MADE;    end = NAME;        break;
	case 'n': begin = NAME;        end = ONEWAY;      break;
	case 'o': begin = ONEWAY;      end = PLACE;       break;
	case 'p': begin = PLACE;       end = RAILWAY;     break;
	case 'r': begin = RAILWAY;     end = SHOP;        break;
	case 's': begin = SHOP;        end = TOURISM;     break;
	case 't': begin = TOURISM;     end = WATERWAY;    break;
	case 'w': begin = WATERWAY;    end = TAG_KEY_END; break;
	default:  begin = NOT_FOUND;   end = NOT_FOUND;   break;
	};
	for(size_t i = size_t(begin); i < size_t(end); ++i)
	{
		if(tag_keys[i] == raw)
			return TagKey(i);
	};
	return NOT_FOUND;
};

const Osm::Node& OsmParser::FindNode(uint64_t id)
{
	static const Node dummy;

	Node correct_id;
	correct_id.id = id;
	const auto found = std::equal_range(
		nodes.begin(),
		nodes.end(),
		correct_id,
		[](const Node& a, const Node& b) {return a.id < b.id;}
	);
	if(found.first == found.second) {
		std::cerr << "(not found " << id << ")";
		return dummy;
	}
	else
		return *found.first;
};

void OsmParser::on_start_element(
	const Glib::ustring& name,
	const AttributeList& properties
)
{
	using boost::lexical_cast;
	if(name.raw() == "node") {
		open_element = NODE;
		open_node = Node();
		for(SaxParser::Attribute attr : properties) {
			if(attr.name.raw() == "id")
				open_node.id = lexical_cast<uint64_t>(attr.value);
			else if(attr.name.raw() == "lat")
				open_node.lat = lexical_cast<double>(attr.value);
			else if(attr.name.raw() == "lon")
				open_node.lon = lexical_cast<double>(attr.value);
		}
	}
	else if(name.raw() == "way") {
		open_element = WAY;
		open_way = Way();
		for(SaxParser::Attribute attr : properties) {
			if(attr.name.raw() == "id")
				open_way.id = lexical_cast<uint64_t>(attr.value);
		}
	}
	else if(name.raw() == "relation") {
		open_element = RELATION;
	}
	else if(name.raw() == "tag" && open_element != CLOSED) {
		Tags& elem_tags = (
			open_element == NODE ?
			open_node.tags :
			open_way.tags
		);
		Glib::ustring k;
		Glib::ustring v;
		for(SaxParser::Attribute attr : properties) {
			if(attr.name.raw() == "k")
				k = attr.value;
			else if(attr.name.raw() == "v")
				v = attr.value;
		}
		const TagKey tk = FindTagKey(k);
		if(tk != NOT_FOUND)
			elem_tags.push_back(Tags::value_type(tk, v));
	}
	else if(name.raw() == "nd" && open_element == WAY) {
		for(SaxParser::Attribute attr : properties) {
			if(attr.name.raw() == "ref")
				open_way.node_ids.push_back(
					lexical_cast<uint64_t>(attr.value)
				);
		}
	};
}

void OsmParser::on_end_element(const Glib::ustring& name)
{
	if(name.raw() == "tag" || name.raw() == "nd")
		return;

	if(name.raw() == "osm") {
		std::sort(
			ways.begin(),
			ways.end(),
			[](const Way& a, const Way& b) {return a.id < b.id;}
		);
		std::sort(
			nodes.begin(),
			nodes.end(),
			[](const Node& a, const Node& b) {return a.id < b.id;}
		);
		return;
	};

	switch(open_element) {
	case NODE:
		nodes.push_back(open_node);
		break;
	case WAY:
		ways.push_back(open_way);
		break;
	}

	open_element = CLOSED;
}

const std::string Node::empty = "";
const std::string Way::empty = "";

void Node::Output(std::ostream& out) const
{
	out << "id = " << id << ", lat = " << lat << ", lon = " << lon;
	if(!tags.empty()) {
		out << ", tags = [";
		bool further = false;
		for(Tags::value_type tag : tags) {
			if(further) out << ", ";
			out << OsmParser::tag_keys.at(std::get<0>(tag)) << "=\"";
			out << std::get<1>(tag) << '"';
			further = true;
		};
		out << "]";
	}
	out << std::endl;
}

const std::string& Node::Tag(TagKey k) const
{
	for(const Tags::value_type& kv  : tags)
	{
		if(std::get<0>(kv) == k)
			return std::get<1>(kv);
	}
	return empty;
}

const std::string& Way::Tag(TagKey k) const
{
	for(const Tags::value_type& kv  : tags)
	{
		if(std::get<0>(kv) == k)
			return std::get<1>(kv);
	}
	return empty;
}
