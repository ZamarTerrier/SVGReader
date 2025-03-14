#include <stdio.h>
#include <string.h>
#include "svg_parser.h"
#include "svg_xml.h"
#include "svg_types.h"
#include "svg_string.h"

svgItem* svgParseRect( cxml_element_node *ptXmlNode )
{
	svgItem *ptItem = NULL;
	char *szValue;

	if( ptXmlNode==NULL )
		return NULL;
	if( strcmp( ( char* )cxml_string_as_raw(&ptXmlNode->name.qname), SVG_TAG_RECT )!=0 )
		return NULL;

	//	Read common values to all kind of item
	ptItem = svgNewItem( ptXmlNode );
	if( ptItem==NULL )
		return NULL;

	ptItem->tKind = SVG_ITEM_KIND_RECT;

	//	x
	cxml_attribute_node *attribute = cxml_table_get(ptXmlNode->attributes, "x");
	if( attribute != NULL ){
		szValue = cxml_string_as_raw(&attribute->value);
		svgStringToCoordinate( szValue, &ptItem->tParameters.tRect.tX );
	}
	//	y
	attribute = cxml_table_get(ptXmlNode->attributes, "y");
	if( attribute != NULL ){
		szValue = cxml_string_as_raw(&attribute->value);
		svgStringToCoordinate( szValue, &ptItem->tParameters.tRect.tY );
	}
	//	width
	attribute = cxml_table_get(ptXmlNode->attributes, "width");
	if( attribute != NULL ){
		szValue = cxml_string_as_raw(&attribute->value);
		svgStringToLength( szValue, &ptItem->tParameters.tRect.tWidth );
	}
	//	height
	attribute = cxml_table_get(ptXmlNode->attributes, "height");
	if( attribute != NULL ){
		szValue = cxml_string_as_raw(&attribute->value);
		svgStringToLength( szValue, &ptItem->tParameters.tRect.tHeight );
	}
	//	rx
	attribute = cxml_table_get(ptXmlNode->attributes, "rx");
	if( attribute != NULL ){
		szValue = cxml_string_as_raw(&attribute->value);
		svgStringToLength( szValue, &ptItem->tParameters.tRect.tRadiusX );
	}
	//	ry
	attribute = cxml_table_get(ptXmlNode->attributes, "ry");
	if( attribute != NULL ){
		szValue = cxml_string_as_raw(&attribute->value);
		svgStringToLength( szValue, &ptItem->tParameters.tRect.tRadiusY );
	}

	return ptItem;
}
