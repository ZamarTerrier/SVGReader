#include <stdio.h>
#include <string.h>
#include "svg_parser.h"
#include "svg_xml.h"
#include "svg_types.h"
#include "svg_string.h"

svgItem* svgParseCircle( cxml_element_node *ptXmlNode )
{
	svgItem *ptItem = NULL;
	char *szValue;

	if( ptXmlNode==NULL )
		return NULL;
	if( strcmp( ( char* )ptXmlNode->name.lname, SVG_TAG_CIRCLE )!=0 )
		return NULL;

	//	Read common values to all kind of item
	ptItem = svgNewItem( ptXmlNode );
	if( ptItem==NULL )
		return NULL;

	ptItem->tKind = SVG_ITEM_KIND_CIRCLE;

	//	x
    cxml_attribute_node *attribute = cxml_table_get(ptXmlNode->attributes, "cx");
	if( attribute != NULL ){
		szValue = cxml_string_as_raw(&attribute->value);
		svgStringToCoordinate( szValue, &ptItem->tParameters.tCircle.tX );
	}
	//	Y
	attribute = cxml_table_get(ptXmlNode->attributes, "cy");
	if( attribute != NULL ){
		szValue = cxml_string_as_raw(&attribute->value);
		svgStringToCoordinate( szValue, &ptItem->tParameters.tCircle.tY );
	}
	//	radius
	attribute = cxml_table_get(ptXmlNode->attributes, "r");
	if( attribute != NULL ){
		szValue = cxml_string_as_raw(&attribute->value);
		svgStringToLength( szValue, &ptItem->tParameters.tCircle.tRadius );
	}

	return ptItem;
}

