#ifndef __svg_parser_h__
#define __svg_parser_h__

#define CXML_USE_QUERY_MOD
#include <cxml/cxml.h>

#include "svg.h"
#include "svg_types.h"
#include "svg_parser_circle.h"
#include "svg_parser_desc.h"
#include "svg_parser_group.h"
#include "svg_parser_line.h"
#include "svg_parser_path.h"
#include "svg_parser_rect.h"
#include "svg_parser_title.h"
#include "svg_parser_ellipse.h"
#include "svg_parser_polyline.h"
#include "svg_parser_polygon.h"

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct{
    svgItem *ptNewItem; /* New parsed item */
    svgItem *ptLastItem; /* Last created item */
    svgItem *ptCurrentParent; /* Current parent, NULL if root level */
    svgItem *ptLastBrother; /* Last item that should be the brother of the new parsed item */
    svgItem *ptLastRootLevelItem; /* Last item on the root level */
    cxml_element_node *ptXmlNode;
} SomeStruct;

typedef svgItem *(*ItemCreateCallback)(cxml_element_node *ptXmlNode);

svgItem* svgNewItem( cxml_element_node *ptXmlNode );
svgError svgParseFile( cxml_element_node *ptXmlNode, svgDrawing *ptDrawing );

#ifdef __cplusplus
}
#endif


#endif	//	 __svg_parser_h__
