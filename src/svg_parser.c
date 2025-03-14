#include <stdio.h>
#include <string.h>
#include "svg_parser.h"
#include "svg_types.h"
#include "svg_xml.h"
#include "svg_string.h"


#ifdef __cplusplus
extern "C" {
#endif

SomeStruct s_struct;
svgDrawing *p_ptDrawing;

svgItem* svgNewItem( cxml_element_node *ptXmlNode )
{
	svgItem *ptItem = NULL;
	char *szValue;

	if( ptXmlNode==NULL )
		return NULL;

	ptItem = ( svgItem* )malloc( sizeof( *ptItem ) );
	if( ptItem==NULL )
	{
		svgSetLastError( SVG_ERR_NOT_ENOUGH_RAM, "Not enough RAM to create SVG drawing." );
		return NULL;
	}
	memset( ptItem, 0, sizeof( *ptItem ) );

	//	ID
    cxml_attribute_node *attribute = cxml_table_get(ptXmlNode->attributes, "id");
	if( attribute != NULL ){
		szValue = cxml_string_as_raw(&attribute->value);
		ptItem->szId = strdup( szValue );
	}

	return ptItem;
}

void MakeSomeAction(SomeStruct *s_struct, svgDrawing *ptDrawing){
	//	Is it the first item ?
	if( ptDrawing->tItemList.ptItem==NULL )
		ptDrawing->tItemList.ptItem = s_struct->ptNewItem;	
	//	Add it to the parent
	if( s_struct->ptCurrentParent!=NULL ) {
		//	Link it to its brother "same level"
		s_struct->ptLastBrother = s_struct->ptCurrentParent->ptLastChild;	
		if( s_struct->ptCurrentParent->ptFirstChild==NULL )
			s_struct->ptCurrentParent->ptFirstChild = s_struct->ptNewItem;	
		s_struct->ptCurrentParent->ptLastChild = s_struct->ptNewItem;
	}
	else {
		s_struct->ptLastBrother = s_struct->ptLastRootLevelItem;
		s_struct->ptLastRootLevelItem = s_struct->ptNewItem;
	}	
	if( s_struct->ptLastBrother!=NULL && s_struct->ptLastBrother->ptParent == s_struct->ptCurrentParent )
		s_struct->ptLastBrother->ptNextItem = s_struct->ptNewItem;	
	s_struct->ptNewItem->ptParent = s_struct->ptCurrentParent;	
	//	Link current item with previous one
	if( s_struct->ptLastItem!=NULL )
		s_struct->ptLastItem->ptNextUnsortedItem = s_struct->ptNewItem;	
	s_struct->ptLastItem = s_struct->ptNewItem;
	ptDrawing->tItemList.ui32Count ++;	
}

void MakeSomeItem(SomeStruct *s_struct, char *ItemName, svgDrawing *ptDrawing, ItemCreateCallback callback){

	char name[64];
	memset(name, 0, 64);
	sprintf(name, "<%s>/", ItemName);

    cxml_list elem = new_cxml_list();
    cxml_find_all(s_struct->ptXmlNode, name, &elem);
	
    uint32_t val = 0;
    cxml_for_each(node, &elem){
        cxml_element_node *d_elm = node;

    	s_struct->ptNewItem = callback( d_elm );
		s_struct->ptXmlNode = d_elm;
	
		MakeSomeAction(s_struct, ptDrawing);
	}
    cxml_list_free(&elem);
}

svgError svgParseFile( cxml_element_node *ptXmlRoot, svgDrawing *ptDrawing )
{
	p_ptDrawing = ptDrawing;

    if( ptXmlRoot==NULL )
    	return svgSetLastError( SVG_ERR_INVALID_PARAMETER, "ptXmlRoot can't be NULL" );
    if( ptDrawing==NULL )
    	return svgSetLastError( SVG_ERR_INVALID_PARAMETER, "ptDrawing can't be NULL" );

	//	Parse SVG file
    s_struct.ptCurrentParent = NULL;
    s_struct.ptLastItem = NULL;
    s_struct.ptLastBrother = NULL;
    s_struct.ptLastRootLevelItem = NULL;
	s_struct.ptXmlNode = ptXmlRoot;

	MakeSomeItem(&s_struct, SVG_TAG_TITLE, ptDrawing, svgParseTitle);
	s_struct.ptXmlNode = ptXmlRoot;
	MakeSomeItem(&s_struct, SVG_TAG_DESC, ptDrawing, svgParseDesc);
	s_struct.ptXmlNode = ptXmlRoot;
	MakeSomeItem(&s_struct, SVG_TAG_GROUP, ptDrawing, svgParseGroup);
	s_struct.ptXmlNode = ptXmlRoot;
	MakeSomeItem(&s_struct, SVG_TAG_PATH, ptDrawing, svgParsePath);
	s_struct.ptXmlNode = ptXmlRoot;
	MakeSomeItem(&s_struct, SVG_TAG_RECT, ptDrawing, svgParseRect);
	s_struct.ptXmlNode = ptXmlRoot;
	MakeSomeItem(&s_struct, SVG_TAG_CIRCLE, ptDrawing, svgParseCircle);
	s_struct.ptXmlNode = ptXmlRoot;
	MakeSomeItem(&s_struct, SVG_TAG_ELLIPSE, ptDrawing, svgParseEllipse);
	s_struct.ptXmlNode = ptXmlRoot;
	MakeSomeItem(&s_struct, SVG_TAG_LINE, ptDrawing, svgParseLine);
	s_struct.ptXmlNode = ptXmlRoot;
	MakeSomeItem(&s_struct, SVG_TAG_POLYLINE, ptDrawing, svgParsePolyline);
	s_struct.ptXmlNode = ptXmlRoot;
	MakeSomeItem(&s_struct, SVG_TAG_POLYGON, ptDrawing, svgParsePolygon);

    return svgSetLastError( SVG_ERR_SUCCESS, NULL );
}



