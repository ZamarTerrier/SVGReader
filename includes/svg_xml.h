#ifndef __svg_xml_h__
#define __svg_xml_h__

#define CXML_USE_QUERY_MOD
#include <cxml/cxml.h>


#ifdef __cplusplus
extern "C"
{
#endif

cxml_element_node *GetChild( cxml_element_node *ptXmlNode, uint32_t tType, char *szChildName );


#ifdef __cplusplus
}
#endif


#endif	//	 __svg_xml_h__
