#include <stdio.h>
#include <string.h>
#include "svg_parser.h"
#include "svg_xml.h"
#include "svg_types.h"
#include "svg_string.h"

extern SomeStruct s_struct;
extern svgDrawing *p_ptDrawing;

extern svgItem* svgParseTitle( cxml_element_node *ptXmlNode );

extern void MakeSomeAction(SomeStruct *s_struct, svgDrawing *ptDrawing);

void MakeSomeItem2(SomeStruct *s_struct, char *ItemName, svgDrawing *ptDrawing, ItemCreateCallback callback){

	char name[64];
	memset(name, 0, 64);
	sprintf(name, "<%s>/", ItemName);

    cxml_list elem = new_cxml_list();
    cxml_find_all(s_struct->ptXmlNode, name, &elem);
	
    uint32_t val = 0;
    cxml_for_each(node2, &elem){
        cxml_element_node *d_elm = node2;

		if(d_elm != s_struct->ptXmlNode){
			s_struct->ptNewItem = callback( d_elm );
			s_struct->ptXmlNode = d_elm;
		
			MakeSomeAction(s_struct, ptDrawing);
		}
	}
    cxml_list_free(&elem);
}

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
	
	MakeSomeItem2(&s_struct, SVG_TAG_TITLE, p_ptDrawing, svgParseTitle);
	MakeSomeItem2(&s_struct, SVG_TAG_DESC, p_ptDrawing, svgParseDesc);
	MakeSomeItem2(&s_struct, SVG_TAG_GROUP, p_ptDrawing, svgParseGroup);
	MakeSomeItem2(&s_struct, SVG_TAG_PATH, p_ptDrawing, svgParsePath);
	MakeSomeItem2(&s_struct, SVG_TAG_RECT, p_ptDrawing, svgParseRect);
	MakeSomeItem2(&s_struct, SVG_TAG_CIRCLE, p_ptDrawing, svgParseCircle);
	MakeSomeItem2(&s_struct, SVG_TAG_ELLIPSE, p_ptDrawing, svgParseEllipse);
	MakeSomeItem2(&s_struct, SVG_TAG_LINE, p_ptDrawing, svgParseLine);
	MakeSomeItem2(&s_struct, SVG_TAG_POLYLINE, p_ptDrawing, svgParsePolyline);
	MakeSomeItem2(&s_struct, SVG_TAG_POLYGON, p_ptDrawing, svgParsePolygon);

	return ptItem;
}

