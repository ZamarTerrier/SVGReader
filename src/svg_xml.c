#include <stdio.h>
#include <string.h>
#include "svg_xml.h"

cxml_element_node *GetChild( cxml_element_node *ptXmlNode, uint32_t tType, char *szChildName )
{
	cxml_element_node *ptXmlChild;

	/*if( ptXmlNode==NULL || szChildName==NULL )
		return NULL;

	ptXmlChild = ptXmlNode->children;
	while( ptXmlChild!=NULL )
	{
		if( ptXmlChild->type==tType && strcmp( ( char* )ptXmlChild->name, szChildName )==0 )
			break;
	}*/

	return ptXmlChild;
}
