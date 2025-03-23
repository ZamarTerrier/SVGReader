#pragma once

#ifndef __svg_h__
#define __svg_h__

#include <stdbool.h>

#include "svg_types.h"
#include "svg_errors.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef int svgtiny_colour;
#define svgtiny_TRANSPARENT 0x1000000
#ifdef __riscos__
#define svgtiny_RGB(r, g, b) ((b) << 16 | (g) << 8 | (r))
#define svgtiny_RED(c) ((c) & 0xff)
#define svgtiny_GREEN(c) (((c) >> 8) & 0xff)
#define svgtiny_BLUE(c) (((c) >> 16) & 0xff)
#else
#define svgtiny_RGB(r, g, b) ((r) << 16 | (g) << 8 | (b))
#define svgtiny_RED(c) (((c) >> 16) & 0xff)
#define svgtiny_GREEN(c) (((c) >> 8) & 0xff)
#define svgtiny_BLUE(c) ((c) & 0xff)
#endif

struct svgtiny_named_color {
	const char *name;
	svgtiny_colour color;
};

typedef struct svgtiny_gradient_stop {
	float offset;
	svgtiny_colour color;
} svgtiny_gradient_stop;

#define svgtiny_MAX_STOPS 10
#define svgtiny_LINEAR_GRADIENT 0x2000000

typedef struct svgtiny_parse_state_gradient {
	unsigned int linear_gradient_stop_count;
	svgtiny_gradient_stop gradient_stop[svgtiny_MAX_STOPS];
	bool gradient_user_space_on_use;
	struct {
		float a, b, c, d, e, f;
	} gradient_transform;
} svgtiny_parse_state_gradient;

typedef struct svgtiny_parse_state {
	float viewport_width;
	float viewport_height;

	/* paint attributes */
	svgtiny_colour fill;
	svgtiny_colour stroke;
	float stroke_width;

	/* gradients */
	svgtiny_parse_state_gradient fill_grad;
	svgtiny_parse_state_gradient stroke_grad;	
} svgtiny_parse_state;

typedef enum _svgItemKind {
	SVG_ITEM_KIND_GROUP = 0,
	SVG_ITEM_KIND_PATH,
	SVG_ITEM_KIND_RECT,
	SVG_ITEM_KIND_CIRCLE,
	SVG_ITEM_KIND_ELLIPSE,
	SVG_ITEM_KIND_LINE,
	SVG_ITEM_KIND_POLYLINE,
	SVG_ITEM_KIND_POLYGON,
	SVG_ITEM_KIND_TITLE,
	SVG_ITEM_KIND_DESC,
} svgItemKind ;

typedef struct _svgItem {
	svgItemKind tKind;
	char *szId;
	struct _svgItem *ptParent;				/* Parent item */
	struct _svgItem *ptFirstChild;			/* Next child, usually when item is a group */
	struct _svgItem *ptLastChild;			/* Last child, usually when item is a group */
	struct _svgItem *ptNextItem;			/* Next item stored as a tree */
	struct _svgItem *ptNextUnsortedItem;	/* Next item but not stored as a tree */

	union {
		svgTitle tTitle;		/* tKind==SVG_ITEM_KIND_TITLE */
		svgDesc tDesc;			/* tKind==SVG_ITEM_KIND_DESC */
		svgLine tLine;			/* tKind==SVG_ITEM_KIND_LINE */
		svgRect tRect;			/* tKind==SVG_ITEM_KIND_RECT */
		svgCircle tCircle;		/* tKind==SVG_ITEM_KIND_CIRCLE */
		svgPath tPath;			/* tKind==SVG_ITEM_KIND_PATH */
		svgEllipse tEllipse;	/* tKind==SVG_ITEM_KIND_ELLIPSE */
		svgPolyline tPolyline;	/* tKind==SVG_ITEM_KIND_POLYLINE */
		svgPolygon tPolygon;	/* tKind==SVG_ITEM_KIND_POLYGON */
	} tParameters ;
	svgtiny_parse_state state;
} svgItem ;

typedef struct _svgItemList {
	svgItem *ptItem;	/* First item */
	uint32 ui32Count;	/* Number of items */
} svgItemList ;

typedef struct _svgDrawing {
	char *szVersion;
	char *szId;
	svgCoordinate tX;
	svgCoordinate tY;
	svgLength tWidth;
	svgLength tHeight;
	svgItemList tItemList;
} svgDrawing ;

svgDrawing* svgOpenFile( const char *szFile );
void svgFreeDrawing( svgDrawing *ptDrawing );

#ifdef __cplusplus
}
#endif

#endif	//	__svg_h__
