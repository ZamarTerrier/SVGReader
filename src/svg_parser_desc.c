#include <stdio.h>
#include <string.h>
#include "svg_parser.h"
#include "svg_xml.h"
#include "svg_types.h"
#include "svg_string.h"

svgItem* svgParseDesc( cxml_element_node *ptXmlNode )
{
	svgItem *ptItem = NULL;

	if( ptXmlNode==NULL )
		return NULL;
	if( strcmp( ( char* )cxml_string_as_raw(&ptXmlNode->name.qname), SVG_TAG_DESC )!=0 )
		return NULL;

	//	Read common values to all kind of item
	ptItem = svgNewItem( ptXmlNode );
	if( ptItem==NULL )
		return NULL;

	ptItem->tKind = SVG_ITEM_KIND_DESC;

    cxml_text_node *text = NULL;
    text = cxml_list_get(&ptXmlNode->children, 0);
    cxml_string *value = &text->value;
    char *str = cxml_string_as_raw(value);
	ptItem->tParameters.tTitle.szText = strdup( str );
	
	return ptItem;
}

