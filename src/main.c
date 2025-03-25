#include "svg.h"

#include <TigorEngine.h>
#include <TigorGUI.h>

#include <GUI/GUIManager.h>

#include <Core/e_camera.h>

#include <Tools/e_math.h>

#include <math.h>

#include "svg.h"

#define TAU             6.28318530717958647692

#ifndef M_PI
#define M_PI		3.14159265358979323846
#endif

#ifndef M_PI_2
#define M_PI_2          1.57079632679489661923
#endif

#define KAPPA		0.5522847498

#define degToRad(angleInDegrees) ((angleInDegrees) * M_PI / 180.0)
#define radToDeg(angleInRadians) ((angleInRadians) * 180.0 / M_PI)

Camera2D cam2D;
Camera3D cam3D;

typedef struct{
    float x;
    float y;
    float x1;
    float y1;
    float x2;
    float y2;
} SomePathStruct;

SomePathStruct positions[256];
uint32_t num_positions = 0;

vec2 points[256];
uint32_t num_points = 0;

int scale = 4;

void ParceSvgItems(svgItem *item){

    memset(&positions, -1, sizeof(positions));

    while(item != NULL){    
        svgPathCommand *comm = NULL;
        svgPathCommand *prev_comm = NULL;
        vec3 color_fill = vec3_f(svgtiny_RED(item->state.fill) / 255.0, svgtiny_GREEN(item->state.fill) / 255.0, svgtiny_BLUE(item->state.fill) / 255.0);
        vec3 color_stroke = vec3_f(svgtiny_RED(item->state.stroke) / 255.0, svgtiny_GREEN(item->state.stroke) / 255.0, svgtiny_BLUE(item->state.stroke) / 255.0);
        if(item->tKind == SVG_ITEM_KIND_PATH){  
            comm = item->tParameters.tPath.ptFirstCommand;  
            float last_x = 0, last_y = 0;
            float last_cubic_x = 0, last_cubic_y = 0;
            float last_quad_x = 0, last_quad_y = 0;
            float subpath_first_x = 0, subpath_first_y = 0;
            while(comm != NULL)
            {
                float x, y, x1, y1, x2, y2, rx, ry, rotation, large_arc, sweep; 
                //	MoveTo //	LineTo
                if((comm->tId == SVG_PATH_CMD_ID_MOVETO_REL) || (comm->tId == SVG_PATH_CMD_ID_MOVETO_ABS) || (comm->tId == SVG_PATH_CMD_ID_LINETO_ABS) || (comm->tId == SVG_PATH_CMD_ID_LINETO_REL)){   
                    x = comm->tParameters.tMoveTo.tX.fValue;
                    y = comm->tParameters.tMoveTo.tY.fValue; 

                    if((comm->tId == SVG_PATH_CMD_ID_MOVETO_REL) || (comm->tId == SVG_PATH_CMD_ID_LINETO_REL)){
                        x += last_x;
                        y += last_y;
                    }   
                    if((comm->tId == SVG_PATH_CMD_ID_MOVETO_REL) || (comm->tId == SVG_PATH_CMD_ID_MOVETO_ABS)){
                        subpath_first_x = x;
                        subpath_first_y = y;
                        
                        if(prev_comm != NULL)
                            PathStroke(color_stroke, true, item->state.stroke_width, false);
                    }   
                    last_cubic_x = last_quad_x = last_x = x;
                    last_cubic_y = last_quad_y = last_y = y;

                    PathLineTo(vec2_f(x * scale, y * scale));   
                }
                //	Vertical LineTo
                else if((comm->tId == SVG_PATH_CMD_ID_VERTICAL_LINETO_REL) || (comm->tId == SVG_PATH_CMD_ID_VERTICAL_LINETO_ABS)){

                    x = last_x;
                    y = comm->tParameters.tLineTo.tY.fValue;    
                    if(comm->tId == SVG_PATH_CMD_ID_VERTICAL_LINETO_REL)
                        y += last_y;    
                    last_cubic_x = last_quad_x = last_x;
                    last_cubic_y = last_quad_y = last_y = y;

                    PathLineTo(vec2_f(x * scale, y * scale));
                }
                //	Horizontal LineTo
                else if((comm->tId == SVG_PATH_CMD_ID_HORIZONTAL_LINETO_REL) || (comm->tId == SVG_PATH_CMD_ID_HORIZONTAL_LINETO_ABS)){

                    x = comm->tParameters.tLineTo.tX.fValue;
                    y = last_y;  
                    if(comm->tId == SVG_PATH_CMD_ID_HORIZONTAL_LINETO_REL)
                        x += last_x;    
                    last_cubic_x = last_quad_x = last_x = x;
                    last_cubic_y = last_quad_y = last_y;

                    PathLineTo(vec2_f(x * scale, y * scale));
                }
                //	Cubic CurveTo
                else if((comm->tId == SVG_PATH_CMD_ID_CUBIC_CURVETO_REL) || (comm->tId == SVG_PATH_CMD_ID_CUBIC_CURVETO_ABS)){

                    x1  = comm->tParameters.tCubicCurveTo.tX1.fValue;
                    y1  = comm->tParameters.tCubicCurveTo.tY1.fValue;
                    x2  = comm->tParameters.tCubicCurveTo.tX2.fValue;
                    y2  = comm->tParameters.tCubicCurveTo.tY2.fValue;
                    x   = comm->tParameters.tCubicCurveTo.tX.fValue;
                    y   = comm->tParameters.tCubicCurveTo.tY.fValue;    
                    if(comm->tId == SVG_PATH_CMD_ID_CUBIC_CURVETO_REL){                        
                        x1 += last_x;
                        y1 += last_y;
                        x2 += last_x;
                        y2 += last_y;
                        x += last_x;
                        y += last_y;
                    }   
                    last_cubic_x = last_x = x;
                    last_cubic_y = last_y = y;   

                    PathBezierCubicCurveTo(vec2_f(x1 * scale, y1 * scale), vec2_f(x2 * scale, y2 * scale), vec2_f(x * scale, y * scale), 10);
                }
                //	Smooth Cubic CurveTo
                else if((comm->tId == SVG_PATH_CMD_ID_SMOOTH_CUBIC_CURVETO_REL) || (comm->tId == SVG_PATH_CMD_ID_SMOOTH_CUBIC_CURVETO_ABS)){

                    x2  = comm->tParameters.tSmoothCubicCurveTo.tX2.fValue;
                    y2  = comm->tParameters.tSmoothCubicCurveTo.tY2.fValue;
                    x   = comm->tParameters.tSmoothCubicCurveTo.tX.fValue;
                    y   = comm->tParameters.tSmoothCubicCurveTo.tY.fValue;  
                    x1 = last_x + (last_x - last_cubic_x);
                    y1 = last_y + (last_y - last_cubic_y);
                    if(comm->tId == SVG_PATH_CMD_ID_SMOOTH_CUBIC_CURVETO_REL){    
                        x2 += last_x;
                        y2 += last_y;
                        x += last_x;
                        y += last_y;
                    }   
                    last_cubic_x    = x2;
                    last_cubic_y    = y2;
                    last_quad_x     = last_x = x;
                    last_quad_y     = last_y = y;    

                    PathBezierCubicCurveTo(vec2_f(x1 * scale, y1 * scale), vec2_f(x2 * scale, y2 * scale), vec2_f(x * scale, y * scale), 10);
                }
                //	Quadratic CurveTo
                else if((comm->tId == SVG_PATH_CMD_ID_QUADRATIC_CURVETO_REL) || (comm->tId == SVG_PATH_CMD_ID_QUADRATIC_CURVETO_ABS)){

                    x1  = comm->tParameters.tQuadraticCurveTo.tX1.fValue;
                    y1  = comm->tParameters.tQuadraticCurveTo.tY1.fValue;
                    x   = comm->tParameters.tQuadraticCurveTo.tX.fValue;
                    y   = comm->tParameters.tQuadraticCurveTo.tY.fValue;    
                    last_quad_x = x1;
                    last_quad_y = y1;
                    if(comm->tId == SVG_PATH_CMD_ID_QUADRATIC_CURVETO_REL){    
                        x1 += last_x;
                        y1 += last_y;
                        x += last_x;
                        y += last_y;
                    }

                    PathBezierCubicCurveTo(vec2_f((1./3 * last_x + 2./3 * x1) * scale, (1./3 * last_y + 2./3 * y1) * scale), vec2_f((2./3 * x1 + 1./3 * x) * scale, (2./3 * y1 + 1./3 * y) * scale), vec2_f(x * scale, y * scale), 10); 
                    last_cubic_x = last_x = x;
                    last_cubic_y = last_y = y; 
                }
                //	Quadratic Cubic CurveTo
                else if((comm->tId == SVG_PATH_CMD_ID_SMOOTH_QUADRATIC_CURVETO_REL) || (comm->tId == SVG_PATH_CMD_ID_SMOOTH_QUADRATIC_CURVETO_ABS)){

                    x   = comm->tParameters.tSmoothQuadraticCurveTo.tX.fValue;
                    y   = comm->tParameters.tSmoothQuadraticCurveTo.tY.fValue;  
                    x1 = last_x + (last_x - last_quad_x);
                    y1 = last_y + (last_y - last_quad_y);
                    last_quad_x = x1;
                    last_quad_y = y1;
                    if(comm->tId == SVG_PATH_CMD_ID_SMOOTH_QUADRATIC_CURVETO_REL){    
                        x1 += last_x;
                        y1 += last_y;
                        x += last_x;
                        y += last_y;
                    }   
                    PathBezierCubicCurveTo(vec2_f((1./3 * last_x + 2./3 * x1) * scale, (1./3 * last_y + 2./3 * y1) * scale), vec2_f((2./3 * x1 + 1./3 * x) * scale, (2./3 * y1 + 1./3 * y) * scale), vec2_f(x * scale, y * scale), 10);

                    last_cubic_x = last_x = x;
                    last_cubic_y = last_y = y;
                }
                //	ArcTo
                else if((comm->tId == SVG_PATH_CMD_ID_ARCTO_REL) || (comm->tId == SVG_PATH_CMD_ID_ARCTO_ABS)){

                    x   = comm->tParameters.tArcTo.tX.fValue;
                    y   = comm->tParameters.tArcTo.tY.fValue;   
                    rx = comm->tParameters.tArcTo.tRadiusX.fValue;
                    ry = comm->tParameters.tArcTo.tRadiusY.fValue;

                    rotation = comm->tParameters.tArcTo.tXAxisAngle.fValue;
                    large_arc = comm->tParameters.tArcTo.tLargeArcFlag.fValue;
                    sweep = comm->tParameters.tArcTo.tSweepFlag.fValue; 
                    int num_segments = _CalcCircleAutoSegmentCount(e_max(rx, ry)); // A bit pessimistic, maybe there's a better computation to do here. 
                    // Because we are filling a closed shape we remove 1 from the count of segments/points
                    const float a_max = M_PI * 2.0f * ((float)num_segments - 1.0f) / (float)num_segments;
                    PathArcTo(vec2_f(x * scale, y * scale), rx, 0, a_max, 0);
                }else if(comm->tId == SVG_PATH_CMD_ID_CLOSEPATH){
                    last_cubic_x = last_quad_x = last_x = subpath_first_x;
                    last_cubic_y = last_quad_y = last_y = subpath_first_y;                        
                }   
                prev_comm = comm;
                comm = comm->ptNextCommand;
            }
            
            bool save = item->state.stroke != svgtiny_TRANSPARENT;          
            if(item->state.fill != svgtiny_TRANSPARENT)
                PathFillConvex(color_fill, save);      

            if(item->state.stroke != svgtiny_TRANSPARENT)
                PathStroke(color_stroke, true, item->state.stroke_width, false);

        }else if(item->tKind == SVG_ITEM_KIND_RECT){

            vec2 pos = vec2_f(item->tParameters.tRect.tX.fValue * scale, item->tParameters.tRect.tY.fValue * scale);
            vec2 size = vec2_f(item->tParameters.tRect.tWidth.fValue  * scale, item->tParameters.tRect.tHeight.fValue  * scale);    
            if(item->state.fill != svgtiny_TRANSPARENT)
                GUIAddRectFilled(pos, v2_add(pos, size), color_fill, item->tParameters.tRect.tRadiusY.fValue * scale, GUIDrawFlags_RoundCornersAll);

            if(item->state.stroke != svgtiny_TRANSPARENT)
                GUIAddRect(pos, v2_add(pos, size), color_stroke, item->tParameters.tRect.tRadiusY.fValue * scale, GUIDrawFlags_RoundCornersAll, item->state.stroke_width);                    

        }else if(item->tKind == SVG_ITEM_KIND_ELLIPSE){

            vec2 pos = vec2_f(item->tParameters.tEllipse.tX.fValue * scale, item->tParameters.tEllipse.tY.fValue * scale);
            vec2 size = vec2_f(item->tParameters.tEllipse.tRadiusX.fValue  * scale, item->tParameters.tEllipse.tRadiusY.fValue  * scale);

            if(item->state.fill != svgtiny_TRANSPARENT)
                GUIAddEllipseFilled(pos, size, color_fill, 0, 0);   
            if(item->state.stroke != svgtiny_TRANSPARENT)
                GUIAddEllipse(pos, size, color_stroke, 0, 0, item->state.stroke_width);

        }else if(item->tKind == SVG_ITEM_KIND_CIRCLE){  
            if(item->state.fill != svgtiny_TRANSPARENT)
                GUIAddCircleFilled(vec2_f(item->tParameters.tCircle.tX.fValue * scale, item->tParameters.tCircle.tY.fValue * scale), item->tParameters.tCircle.tRadius.fValue * scale, color_fill, 0);  
            if(item->state.fill != svgtiny_TRANSPARENT)
                GUIAddCircle(vec2_f(item->tParameters.tCircle.tX.fValue * scale, item->tParameters.tCircle.tY.fValue * scale), item->tParameters.tCircle.tRadius.fValue * scale, color_stroke, 0, item->state.stroke_width);
        }   
        item = item->ptNextItem;
    }
}

extern void AddConvexPolyFilled(const vec2 *points, const int points_count, vec3 col);

int main(){

    TEngineInitSystem(800, 600, "Test");

    Camera2DInit(&cam2D);
    Camera3DInit(&cam3D);

    svgDrawing* temp = svgOpenFile("temp.svg");

    ParceSvgItems(temp->tItemList.ptItem);


    while(!TEngineWindowIsClosed()){

        TEnginePoolEvents();

        svgItem *item = temp->tItemList.ptItem;
        
        ParceSvgItems(item);
        
        TEngineRender();
    }

    TEngineCleanUp();

    return 0;
}