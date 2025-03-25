#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <malloc.h>

#include "svg.h"
#include "svg_parser.h"


svgDrawing* svgOpenFile( const char *szFile )
{
	svgDrawing *ptDrawing = NULL;
	cxml_root_node *ptXmlRoot = NULL;
    cxml_element_node *ptXml;
    char *szValue;

	//	check parameters
	if( szFile==NULL ) {
		svgSetLastError( SVG_ERR_INVALID_PARAMETER, "szFile can't be NULL."  );
		goto _exit;
	}
	if( access( szFile, F_OK )!=0 ) {
		svgSetLastError( SVG_ERR_NO_SUCH_FILE, "No such file." );
		goto _exit;
	}

	//	Open SVG file
    ptXmlRoot = cxml_load_file( szFile , false);
	if( ptXmlRoot==NULL ) {
		svgSetLastError( SVG_ERR_BAD_FILE_FORMAT, "Can't parse XML file." );
		goto _exit;
	}

    ptXml = cxml_find(ptXmlRoot, "<svg>/");
    if( ptXml==NULL ) {
		svgSetLastError( SVG_ERR_BAD_FILE_FORMAT, "Can't get XML root node." );
		goto _exit;
    }

	//	Initialize SVG drawing
	ptDrawing = ( svgDrawing* )malloc( sizeof( *ptDrawing ) );
	if( ptDrawing==NULL ) {
		svgSetLastError( SVG_ERR_NOT_ENOUGH_RAM, "Not enough RAM to create SVG drawing." );
		goto _exit;
	}
	memset( ptDrawing, 0, sizeof( *ptDrawing ) );

	//	SVG's version number
    cxml_attribute_node *attribute = cxml_table_get(ptXml->attributes, "version");
	if( attribute !=NULL ){
		szValue = cxml_string_as_raw(&attribute->value);
		ptDrawing->szVersion = strdup( szValue );
	}
	attribute = cxml_table_get(ptXml->attributes, "id");
	if( attribute !=NULL ){
		szValue = cxml_string_as_raw(&attribute->value);
		ptDrawing->szId = strdup( szValue );
	}
	attribute = cxml_table_get(ptXml->attributes, "x");
	if( attribute !=NULL ){
		szValue = cxml_string_as_raw(&attribute->value);
		svgStringToCoordinate( szValue, &ptDrawing->tX );
	}
	attribute = cxml_table_get(ptXml->attributes, "y");
	if( attribute !=NULL ){
		szValue = cxml_string_as_raw(&attribute->value);
		svgStringToCoordinate( szValue, &ptDrawing->tY );
	}
	ptDrawing->tWidth.fValue = 100;
	ptDrawing->tWidth.tUnit = SVG_LENGTH_UNIT_PERCENT;
	attribute = cxml_table_get(ptXml->attributes, "width");
	if( attribute !=NULL ){
		szValue = cxml_string_as_raw(&attribute->value);
		svgStringToLength( szValue, &ptDrawing->tWidth );
	}
	//	SVG's Height
	ptDrawing->tHeight.fValue = 100;
	ptDrawing->tHeight.tUnit = SVG_LENGTH_UNIT_PERCENT;
	attribute = cxml_table_get(ptXml->attributes, "height");
	if(attribute !=NULL ){
		szValue = cxml_string_as_raw(&attribute->value);
		svgStringToLength( szValue, &ptDrawing->tHeight );
	}

	//	Parse the svg objects
	svgParseFile( ptXml, ptDrawing );

_exit:
	//	Free memory
	if( ptXml!=NULL )
    	cxml_destroy(ptXmlRoot);

	return ptDrawing;
}

void svgFreeItem( svgItem *ptItem )
{
	svgPathCommand *ptPathCmd, *ptNextPathCmd;
	svgPoint *ptPoint, *ptNextPoint;
	if( ptItem==NULL )
		return;

	if( ptItem->szId!=NULL )
		free( ptItem->szId );

	switch( ptItem->tKind )
	{
		case SVG_ITEM_KIND_GROUP:
			break;
		case SVG_ITEM_KIND_PATH:
			ptPathCmd = ptItem->tParameters.tPath.ptFirstCommand;
			while( ptPathCmd!=NULL ) {
				ptNextPathCmd = ptPathCmd->ptNextCommand;
				free( ptPathCmd );
				ptPathCmd = ptNextPathCmd;
			}
			break;
		case SVG_ITEM_KIND_RECT:
			break;
		case SVG_ITEM_KIND_CIRCLE:
			break;
		case SVG_ITEM_KIND_ELLIPSE:
			break;
		case SVG_ITEM_KIND_LINE:
			break;
		case SVG_ITEM_KIND_POLYLINE:
			ptPoint = ptItem->tParameters.tPolyline.tFirstPoint.ptNextPoint;
			while( ptPoint!=NULL ) {
				ptNextPoint = ptPoint->ptNextPoint;
				free( ptPoint );
				ptPoint = ptNextPoint;
			}
			break;
		case SVG_ITEM_KIND_POLYGON:
			ptPoint = ptItem->tParameters.tPolygon.tFirstPoint.ptNextPoint;
			while( ptPoint!=NULL ) {
				ptNextPoint = ptPoint->ptNextPoint;
				free( ptPoint );
				ptPoint = ptNextPoint;
			}
			break;
		case SVG_ITEM_KIND_TITLE:
			if( ptItem->tParameters.tTitle.szText!=NULL )
				free( ptItem->tParameters.tTitle.szText );
			break;
		case SVG_ITEM_KIND_DESC:
			if( ptItem->tParameters.tTitle.szText!=NULL )
				free( ptItem->tParameters.tDesc.szText );
			break;
	}

	free( ptItem );
}

void svgFreeDrawing( svgDrawing *ptDrawing )
{
	svgItem *ptItem, *ptNextItem;
	if( ptDrawing==NULL )
		return;

	//	Common attributes
	if( ptDrawing->szId!=NULL )
		free( ptDrawing->szId );
	if( ptDrawing->szVersion!=NULL )
		free( ptDrawing->szVersion );

	//	Free items...We use the unsorted list to make it simpler
	ptItem = ptDrawing->tItemList.ptItem;
	while( ptItem!=NULL ){
		//	since we'll free the current item we must store the location to the next one
		ptNextItem = ptItem->ptNextUnsortedItem;
		svgFreeItem( ptItem );

		//	Next
		ptItem = ptNextItem;
	}

	free( ptDrawing );
}

