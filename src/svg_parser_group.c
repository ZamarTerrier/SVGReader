#include <stdio.h>
#include <string.h>
#include "svg_parser.h"
#include "svg_xml.h"
#include "svg_types.h"
#include "svg_string.h"

extern SomeStruct s_struct;
extern svgDrawing *p_ptDrawing;

extern svgItem* svgParseTitle( cxml_element_node *ptXmlNode );

extern void MakeSomeItem(SomeStruct *s_struct, char *ItemName, svgDrawing *ptDrawing, ItemCreateCallback callback);

svgItem* svgParseGroup( cxml_element_node *ptXmlNode )
{
	svgItem *ptItem = NULL;

	if( ptXmlNode==NULL )
		return NULL;
	if( strcmp( ( char* )cxml_string_as_raw(&ptXmlNode->name.qname), SVG_TAG_GROUP )!=0 )
		return NULL;

	//	Read common values to all kind of item
	ptItem = svgNewItem( ptXmlNode );
	if( ptItem==NULL )
		return NULL;

	ptItem->tKind = SVG_ITEM_KIND_GROUP;

	s_struct.ptXmlNode = ptXmlNode;
	
	MakeSomeItem(&s_struct, SVG_TAG_TITLE, p_ptDrawing, svgParseTitle);
	MakeSomeItem(&s_struct, SVG_TAG_DESC, p_ptDrawing, svgParseDesc);
	MakeSomeItem(&s_struct, SVG_TAG_GROUP, p_ptDrawing, svgParseGroup);
	MakeSomeItem(&s_struct, SVG_TAG_PATH, p_ptDrawing, svgParsePath);
	MakeSomeItem(&s_struct, SVG_TAG_RECT, p_ptDrawing, svgParseRect);
	MakeSomeItem(&s_struct, SVG_TAG_CIRCLE, p_ptDrawing, svgParseCircle);
	MakeSomeItem(&s_struct, SVG_TAG_ELLIPSE, p_ptDrawing, svgParseEllipse);
	MakeSomeItem(&s_struct, SVG_TAG_LINE, p_ptDrawing, svgParseLine);
	MakeSomeItem(&s_struct, SVG_TAG_POLYLINE, p_ptDrawing, svgParsePolyline);
	MakeSomeItem(&s_struct, SVG_TAG_POLYGON, p_ptDrawing, svgParsePolygon);

	return ptItem;
}

