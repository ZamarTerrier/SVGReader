#ifndef __svg_parser_desc_h__
#define __svg_parser_desc_h__

#include "svg.h"
#include "svg_types.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define SVG_TAG_DESC		"desc"

svgItem* svgParseDesc( cxml_element_node *ptXmlNode );


#ifdef __cplusplus
}
#endif


#endif	//	 __svg_parser_desc_h__
