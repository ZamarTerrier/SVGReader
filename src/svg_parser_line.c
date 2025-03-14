#include <stdio.h>
#include <string.h>
#include "svg_parser.h"
#include "svg_xml.h"
#include "svg_types.h"
#include "svg_string.h"

svgItem* svgParseLine( cxml_element_node *ptXmlNode )
{
	svgItem *ptItem = NULL;
	char *szValue;

	if( ptXmlNode==NULL )
		return NULL;
	if( strcmp( ( char* )cxml_string_as_raw(&ptXmlNode->name.qname), SVG_TAG_LINE )!=0 )
		return NULL;

	//	Read common values to all kind of item
	ptItem = svgNewItem( ptXmlNode );
	if( ptItem==NULL )
		return NULL;

	ptItem->tKind = SVG_ITEM_KIND_LINE;

	//	x1
	cxml_attribute_node *attribute = cxml_table_get(ptXmlNode->attributes, "x1");
	if( attribute != NULL ){
		szValue = cxml_string_as_raw(&attribute->value);
		svgStringToCoordinate( szValue, &ptItem->tParameters.tLine.tX1 );
	}
	//	Y1
	attribute = cxml_table_get(ptXmlNode->attributes, "y1");
	if( attribute != NULL ){
		szValue = cxml_string_as_raw(&attribute->value);
		svgStringToCoordinate( szValue, &ptItem->tParameters.tLine.tY1 );
	}
	//	X2
	attribute = cxml_table_get(ptXmlNode->attributes, "x2");
	if( attribute != NULL ){
		szValue = cxml_string_as_raw(&attribute->value);
		svgStringToCoordinate( szValue, &ptItem->tParameters.tLine.tX2 );
	}
	//	Y2
	attribute = cxml_table_get(ptXmlNode->attributes, "y2");
	if( attribute != NULL ){
		szValue = cxml_string_as_raw(&attribute->value);
		svgStringToCoordinate( szValue, &ptItem->tParameters.tLine.tY2 );
	}


	return ptItem;
}

