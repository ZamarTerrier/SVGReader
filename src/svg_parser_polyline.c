#include <stdio.h>
#include <string.h>
#include "svg_parser.h"
#include "svg_xml.h"
#include "svg_types.h"
#include "svg_string.h"

svgItem* svgParsePolyline( cxml_element_node *ptXmlNode )
{
	svgItem *ptItem = NULL;
	char *szValue, szField[ 16 ];
	const char *szFieldStart;
	svgPoint *ptNewPoint = NULL, *ptLastPoint = NULL;

	if( ptXmlNode==NULL )
		return NULL;
	if( strcmp( ( char* )cxml_string_as_raw(&ptXmlNode->name.qname), SVG_TAG_POLYLINE )!=0 )
		return NULL;

	//	Read common values to all kind of item
	ptItem = svgNewItem( ptXmlNode );
	if( ptItem==NULL )
		return NULL;

	ptItem->tKind = SVG_ITEM_KIND_POLYLINE;

	//	Points
	cxml_attribute_node *attribute = cxml_table_get(ptXmlNode->attributes, "points");
	if( attribute != NULL ) {
		szValue = cxml_string_as_raw(&attribute->value);

		szFieldStart = svgGetNextPointField( szValue, szField, 16 );

		while( szFieldStart!=NULL ) {
			//	First point ?
			if( ptNewPoint==NULL )
				ptNewPoint = &ptItem->tParameters.tPolyline.tFirstPoint;
			else
				ptNewPoint = ( svgPoint* )malloc( sizeof( *ptNewPoint ) );
			memset( ptNewPoint, 0, sizeof( *ptNewPoint ) );

			//	X
			svgStringToCoordinate( szField, &ptNewPoint->tX );

			//	Y
			szFieldStart += strlen( szField );
			szFieldStart = svgGetNextPointField( szFieldStart, szField, 16 );
			svgStringToCoordinate( szField, &ptNewPoint->tY );

			//	Link with last point
			if( ptLastPoint!=NULL )
				ptLastPoint->ptNextPoint = ptNewPoint;
			ptLastPoint = ptNewPoint;

			//	Next X
			szFieldStart += strlen( szField );
			szFieldStart = svgGetNextPointField( szFieldStart, szField, 16 );
		}
	}

	return ptItem;
}
