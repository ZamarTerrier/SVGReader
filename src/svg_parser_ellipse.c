#include <stdio.h>
#include <string.h>
#include "svg_parser.h"
#include "svg_xml.h"
#include "svg_types.h"
#include "svg_string.h"

svgItem* svgParseEllipse( cxml_element_node *ptXmlNode )
{
	svgItem *ptItem = NULL;
	char *szValue;

	if( ptXmlNode==NULL )
		return NULL;
	if( strcmp( ( char* )cxml_string_as_raw(&ptXmlNode->name.qname), SVG_TAG_ELLIPSE )!=0 )
		return NULL;

	//	Read common values to all kind of item
	ptItem = svgNewItem( ptXmlNode );
	if( ptItem==NULL )
		return NULL;

	ptItem->tKind = SVG_ITEM_KIND_ELLIPSE;

	//	cx
	cxml_attribute_node *attribute = cxml_table_get(ptXmlNode->attributes, "cx");
	if( attribute != NULL ){
		szValue = cxml_string_as_raw(&attribute->value);
		svgStringToCoordinate( szValue, &ptItem->tParameters.tEllipse.tX );
	}
	//	cY
	attribute = cxml_table_get(ptXmlNode->attributes, "cy");
	if( attribute !=NULL ){
		szValue = cxml_string_as_raw(&attribute->value);
		svgStringToCoordinate( szValue, &ptItem->tParameters.tEllipse.tY );
	}
	//	RadiusX
	attribute = cxml_table_get(ptXmlNode->attributes, "rx");
	if( attribute != NULL ){
		szValue = cxml_string_as_raw(&attribute->value);
		svgStringToLength( szValue, &ptItem->tParameters.tEllipse.tRadiusX );
	}
	//	RadiusY
	attribute = cxml_table_get(ptXmlNode->attributes, "ry");
	if( attribute != NULL ){
		szValue = cxml_string_as_raw(&attribute->value);
		svgStringToLength( szValue, &ptItem->tParameters.tEllipse.tRadiusY );
	}


	return ptItem;
}
