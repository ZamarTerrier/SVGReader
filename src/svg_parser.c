#include <stdio.h>
#include <string.h>
#include "svg_parser.h"
#include "svg_types.h"
#include "svg_xml.h"
#include "svg_string.h"


#ifdef __cplusplus
extern "C" {
#endif

#ifndef UNUSED
#define UNUSED(x) ((void) (x))
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


/**
 * Find a gradient by id and parse it.
 */

void svgtiny_find_gradient(const char *id,
		struct svgtiny_parse_state_gradient *grad,
		struct svgtiny_parse_state *state)
{
}

/**
 * Parse a colour.
 */

static void _svgtiny_parse_color(const char *s, svgtiny_colour *c,
		struct svgtiny_parse_state_gradient *grad,
		struct svgtiny_parse_state *state)
{
	unsigned int r, g, b;
	float rf, gf, bf;
	size_t len = strlen(s);
	char *id = 0, *rparen;

	if (len == 4 && s[0] == '#') {
		if (sscanf(s + 1, "%1x%1x%1x", &r, &g, &b) == 3)
			*c = svgtiny_RGB(r | r << 4, g | g << 4, b | b << 4);

	} else if (len == 7 && s[0] == '#') {
		if (sscanf(s + 1, "%2x%2x%2x", &r, &g, &b) == 3)
			*c = svgtiny_RGB(r, g, b);

	} else if (10 <= len && s[0] == 'r' && s[1] == 'g' && s[2] == 'b' &&
			s[3] == '(' && s[len - 1] == ')') {
		if (sscanf(s + 4, "%u,%u,%u", &r, &g, &b) == 3)
			*c = svgtiny_RGB(r, g, b);
		else if (sscanf(s + 4, "%f%%,%f%%,%f%%", &rf, &gf, &bf) == 3) {
			b = bf * 255 / 100;
			g = gf * 255 / 100;
			r = rf * 255 / 100;
			*c = svgtiny_RGB(r, g, b);
		}

	} else if (len == 4 && strcmp(s, "none") == 0) {
		*c = svgtiny_TRANSPARENT;

	} else if (5 < len && s[0] == 'u' && s[1] == 'r' && s[2] == 'l' &&
			s[3] == '(') {
		if (grad == NULL) {
			*c = svgtiny_RGB(0, 0, 0);
		} else if (s[4] == '#') {
			id = strdup(s + 5);
			if (!id)
				return;
			rparen = strchr(id, ')');
			if (rparen)
				*rparen = 0;
			svgtiny_find_gradient(id, grad, state);
			free(id);
			if (grad->linear_gradient_stop_count == 0)
				*c = svgtiny_TRANSPARENT;
			else if (grad->linear_gradient_stop_count == 1)
				*c = grad->gradient_stop[0].color;
			else
				*c = svgtiny_LINEAR_GRADIENT;
		}

	} else {
		const struct svgtiny_named_color *named_color;
		named_color = svgtiny_color_lookup(s, strlen(s));
		if (named_color)
			*c = named_color->color;
	}
}

char *strndup(const char *s, size_t n)
{
	size_t len;
	char *s2;

	for (len = 0; len != n && s[len]; len++)
		continue;

	s2 = malloc(len + 1);
	if (s2 == NULL)
		return NULL;

	memcpy(s2, s, len);
	s2[len] = '\0';

	return s2;
}

static float _svgtiny_parse_length(const char *s, int viewport_size,
				   const struct svgtiny_parse_state state)
{
	int num_length = strspn(s, "0123456789+-.");
	const char *unit = s + num_length;
	float n = atof((const char *) s);
	float font_size = 20; /*css_len2px(&state.style.font_size.value.length, 0);*/

	UNUSED(state);

	if (unit[0] == 0) {
		return n;
	} else if (unit[0] == '%') {
		return n / 100.0 * viewport_size;
	} else if (unit[0] == 'e' && unit[1] == 'm') {
		return n * font_size;
	} else if (unit[0] == 'e' && unit[1] == 'x') {
		return n / 2.0 * font_size;
	} else if (unit[0] == 'p' && unit[1] == 'x') {
		return n;
	} else if (unit[0] == 'p' && unit[1] == 't') {
		return n * 1.25;
	} else if (unit[0] == 'p' && unit[1] == 'c') {
		return n * 15.0;
	} else if (unit[0] == 'm' && unit[1] == 'm') {
		return n * 3.543307;
	} else if (unit[0] == 'c' && unit[1] == 'm') {
		return n * 35.43307;
	} else if (unit[0] == 'i' && unit[1] == 'n') {
		return n * 90;
	}

	return 0;
}

void StyleParcing(cxml_element_node *ptXmlNode, struct svgtiny_parse_state *state){
    cxml_attribute_node *attribute = cxml_table_get(ptXmlNode->attributes, "style");

	if( attribute != NULL ){
		char *style = cxml_string_as_raw(&attribute->value);

		const char *s;
		char *value;
		if ((s = strstr(style, "fill:"))) {
			s += 5;
			while (*s == ' ')
				s++;
			value = strndup(s, strcspn(s, "; "));
			_svgtiny_parse_color(value, &state->fill, &state->fill_grad, state);
			free(value);
		}
		if ((s = strstr(style, "stroke:"))) {
			s += 7;
			while (*s == ' ')
				s++;
			value = strndup(s, strcspn(s, "; "));
			_svgtiny_parse_color(value, &state->stroke, &state->stroke_grad, state);
			free(value);
		}
		if ((s = strstr(style, "stroke-width:"))) {
			s += 13;
			while (*s == ' ')
				s++;
			value = strndup(s, strcspn(s, "; "));
			state->stroke_width = _svgtiny_parse_length(value,
						state->viewport_width, *state);
			free(value);
		}
	}
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
		s_struct->ptNewItem->state.viewport_height = ptDrawing->tHeight.fValue;
		s_struct->ptNewItem->state.viewport_width = ptDrawing->tWidth.fValue;
		StyleParcing(d_elm, &s_struct->ptNewItem->state);
	
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



