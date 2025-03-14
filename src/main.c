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

/**
 * rotate midpoint vector
 */
static void
rotate_midpoint_vector(float ax, float ay,
		       float bx, float by,
		       double radangle,
		       double *x_out, double *y_out)
{
	double dx2; /* midpoint x coordinate */
	double dy2; /* midpoint y coordinate */
	double cosangle; /* cosine of rotation angle */
	double sinangle; /* sine of rotation angle */

	/* compute the sin and cos of the angle */
	cosangle = cos(radangle);
	sinangle = sin(radangle);

	/* compute the midpoint between start and end points */
	dx2 = (ax - bx) / 2.0;
	dy2 = (ay - by) / 2.0;

	/* rotate vector to remove angle */
	*x_out = ((cosangle * dx2) + (sinangle * dy2));
	*y_out = ((-sinangle * dx2) + (cosangle * dy2));
}

/**
 * ensure the arc radii are large enough and scale as appropriate
 *
 * the radii need to be large enough if they are not they must be
 *  adjusted. This allows for elimination of differences between
 *  implementations especialy with rounding.
 */
static void
ensure_radii_scale(double x1_sq, double y1_sq,
		   float *rx, float *ry,
		   double *rx_sq, double *ry_sq)
{
	double radiisum;
	double radiiscale;

	/* set radii square values */
	(*rx_sq) = (*rx) * (*rx);
	(*ry_sq) = (*ry) * (*ry);

	radiisum = (x1_sq / (*rx_sq)) + (y1_sq / (*ry_sq));
	if (radiisum > 0.99999) {
		/* need to scale radii */
		radiiscale = sqrt(radiisum) * 1.00001;
		*rx = (float)(radiiscale * (*rx));
		*ry = (float)(radiiscale * (*ry));
		/* update squares too */
		(*rx_sq) = (*rx) * (*rx);
		(*ry_sq) = (*ry) * (*ry);
	}
}

/**
 * compute the transformed centre point
 */
static void
compute_transformed_centre_point(double sign, float rx, float ry,
				 double rx_sq, double ry_sq,
				 double x1, double y1,
				 double x1_sq, double y1_sq,
				 double *cx1, double *cy1)
{
	double sq;
	double coef;
	sq = ((rx_sq * ry_sq) - (rx_sq * y1_sq) - (ry_sq * x1_sq)) /
		((rx_sq * y1_sq) + (ry_sq * x1_sq));
	sq = (sq < 0) ? 0 : sq;

	coef = (sign * sqrt(sq));

	*cx1 = coef * ((rx * y1) / ry);
	*cy1 = coef * -((ry * x1) / rx);
}

/**
 * compute untransformed centre point
 *
 * \param ax The first point x coordinate
 * \param ay The first point y coordinate
 * \param bx The second point x coordinate
 * \param ay The second point y coordinate
 */
static void
compute_centre_point(float ax, float ay,
		     float bx, float by,
		     double cx1, double cy1,
		     double radangle,
		     double *x_out, double *y_out)
{
	double sx2;
	double sy2;
	double cosangle; /* cosine of rotation angle */
	double sinangle; /* sine of rotation angle */

	/* compute the sin and cos of the angle */
	cosangle = cos(radangle);
	sinangle = sin(radangle);

	sx2 = (ax + bx) / 2.0;
	sy2 = (ay + by) / 2.0;

	*x_out = sx2 + (cosangle * cx1 - sinangle * cy1);
	*y_out = sy2 + (sinangle * cx1 + cosangle * cy1);
}

/**
 * compute the angle start and extent
 */
static void
compute_angle_start_extent(float rx, float ry,
			   double x1, double y1,
			   double cx1, double cy1,
			   double *start, double *extent)
{
	double sign;
	double ux;
	double uy;
	double vx;
	double vy;
	double p, n;
	double actmp;

	/*
	 * Angle betwen two vectors is +/- acos( u.v / len(u) * len(v))
	 * Where:
	 *  '.' is the dot product.
	 *  +/- is calculated from the sign of the cross product (u x v)
	 */

	ux = (x1 - cx1) / rx;
	uy = (y1 - cy1) / ry;
	vx = (-x1 - cx1) / rx;
	vy = (-y1 - cy1) / ry;

	/* compute the start angle */
	/* The angle between (ux, uy) and the 0 angle */

	/* len(u) * len(1,0) == len(u) */
	n = sqrt((ux * ux) + (uy * uy));
	/* u.v == (ux,uy).(1,0) == (1 * ux) + (0 * uy) == ux */
	p = ux;
	/* u x v == (1 * uy - ux * 0) == uy */
	sign = (uy < 0) ? -1.0 : 1.0;
	/* (p >= n) so safe */
	*start = sign * acos(p / n);

	/* compute the extent angle */
	n = sqrt(((ux * ux) + (uy * uy)) * ((vx * vx) + (vy * vy)));
	p = (ux * vx) + (uy * vy);
	sign = ((ux * vy) - (uy * vx) < 0) ? -1.0f : 1.0f;

	/* arc cos must operate between -1 and 1 */
	actmp = p / n;
	if (actmp < -1.0) {
		*extent = sign * M_PI;
	} else if (actmp > 1.0) {
		*extent = 0;
	} else {
		*extent = sign * acos(actmp);
	}
}

/**
 * converts a circle centered unit circle arc to a series of bezier curves
 *
 * Each bezier is stored as six values of three pairs of coordinates
 *
 * The beziers are stored without their start point as that is assumed
 *   to be the preceding elements end point.
 *
 * \param start The start angle of the arc (in radians)
 * \param extent The size of the arc (in radians)
 * \param bzpt The array to store the bezier values in
 * \return The number of bezier segments output (max 4)
 */
static int
circle_arc_to_bezier(double start, double extent, double *bzpt)
{
	int bzsegments;
	double increment;
	double controllen;
	int pos = 0;
	int segment;
	double angle;
	double dx, dy;

	bzsegments = (int) ceil(fabs(extent) / M_PI_2);
	increment = extent / bzsegments;
	controllen = 4.0 / 3.0 * sin(increment / 2.0) / (1.0 + cos(increment / 2.0));

	for (segment = 0; segment < bzsegments; segment++) {
		/* first control point */
		angle = start + (segment * increment);
		dx = cos(angle);
		dy = sin(angle);
		bzpt[pos++] = dx - controllen * dy;
		bzpt[pos++] = dy + controllen * dx;
		/* second control point */
		angle+=increment;
		dx = cos(angle);
		dy = sin(angle);
		bzpt[pos++] = dx + controllen * dy;
		bzpt[pos++] = dy - controllen * dx;
		/* endpoint */
		bzpt[pos++] = dx;
		bzpt[pos++] = dy;

	}
	return bzsegments;
}

/**
 * transform coordinate list
 *
 * perform a scale, rotate and translate on list of coordinates
 *
 * scale(rx,ry)
 * rotate(an)
 * translate (cx, cy)
 *
 * homogeneous transforms
 *
 * scaling
 *     |   rx        0        0   |
 * S = |    0       ry        0   |
 *     |    0        0        1   |
 *
 * rotate
 *     | cos(an)  -sin(an)    0   |
 * R = | sin(an)   cos(an)    0   |
 *     |    0        0        1   |
 *
 * {{cos(a), -sin(a) 0}, {sin(a), cos(a),0}, {0,0,1}}
 *
 * translate
 *     |    1        0       cx   |
 * T = |    0        1       cy   |
 *     |    0        0        1   |
 *
 * note order is significat here and the combined matrix is
 * M = T.R.S
 *
 *       | cos(an)  -sin(an)    cx   |
 * T.R = | sin(an)   cos(an)    cy   |
 *       |    0        0        1    |
 *
 *         | rx * cos(an)  ry * -sin(an)   cx  |
 * T.R.S = | rx * sin(an)  ry * cos(an)    cy  |
 *         | 0             0               1   |
 *
 * {{Cos[a], -Sin[a], c}, {Sin[a], Cos[a], d}, {0, 0, 1}} . {{r, 0, 0}, {0, s, 0}, {0, 0, 1}}
 *
 * Each point
 *     | x1 |
 * P = | y1 |
 *     |  1 |
 *
 * output
 * | x2 |
 * | y2 | = M . P
 * | 1  |
 *
 * x2 = cx + (rx * x1 * cos(a)) + (ry * y1 * -1 * sin(a))
 * y2 = cy + (ry * y1 * cos(a)) + (rx * x1 * sin(a))
 *
 *
 * \param rx X scaling to apply
 * \param ry Y scaling to apply
 * \param radangle rotation to apply (in radians)
 * \param cx X translation to apply
 * \param cy Y translation to apply
 * \param points The size of the bzpoints array
 * \param bzpoints an array of x,y values to apply the transform to
 */
static void
scale_rotate_translate_points(double rx, double ry,
			      double radangle,
			      double cx, double cy,
			      int pntsize,
			      double *points)
{
	int pnt;
	double cosangle; /* cosine of rotation angle */
	double sinangle; /* sine of rotation angle */
	double rxcosangle, rxsinangle, rycosangle, rynsinangle;
	double x2,y2;

	/* compute the sin and cos of the angle */
	cosangle = cos(radangle);
	sinangle = sin(radangle);

	rxcosangle = rx * cosangle;
	rxsinangle = rx * sinangle;
	rycosangle = ry * cosangle;
	rynsinangle = ry * -1 * sinangle;

	for (pnt = 0; pnt < pntsize; pnt+=2) {
		x2 = cx + (points[pnt] * rxcosangle) + (points[pnt + 1] * rynsinangle);
		y2 = cy + (points[pnt + 1] * rycosangle) + (points[pnt] * rxsinangle);
		points[pnt] = x2;
		points[pnt + 1] = y2;
	}
}

static int
svgarc_to_bezier(float start_x,
		 float start_y,
		 float end_x,
		 float end_y,
		 float rx,
		 float ry,
		 float angle,
		 bool largearc,
		 bool sweep,
		 double *bzpoints)
{
	double radangle; /* normalised elipsis rotation angle in radians */
	double rx_sq; /* x radius squared */
	double ry_sq; /* y radius squared */
	double x1, y1; /* rotated midpoint vector */
	double x1_sq, y1_sq; /* x1 vector squared */
	double cx1,cy1; /* transformed circle center */
	double cx,cy; /* circle center */
	double start, extent;
	int bzsegments;

	if ((start_x == end_x) && (start_y == end_y)) {
		/*
		 * if the start and end coordinates are the same the
		 *  svg spec says this is equivalent to having no segment
		 *  at all
		 */
		return 0;
	}

	if ((rx == 0) || (ry == 0)) {
		/*
		 * if either radii is zero the specified behaviour is a line
		 */
		return -1;
	}

	/* obtain the absolute values of the radii */
	rx = fabsf(rx);
	ry = fabsf(ry);

	/* convert normalised angle to radians */
	radangle = degToRad(fmod(angle, 360.0));

	/* step 1 */
	/* x1,x2 is the midpoint vector rotated to remove the arc angle */
	rotate_midpoint_vector(start_x, start_y, end_x, end_y, radangle, &x1, &y1);

	/* step 2 */
	/* get squared x1 values */
	x1_sq = x1 * x1;
	y1_sq = y1 * y1;

	/* ensure radii are correctly scaled  */
	ensure_radii_scale(x1_sq, y1_sq, &rx, &ry, &rx_sq, &ry_sq);

	/* compute the transformed centre point */
	compute_transformed_centre_point(largearc == sweep?-1:1,
					 rx, ry,
					 rx_sq, ry_sq,
					 x1, y1,
					 x1_sq, y1_sq,
					 &cx1, &cy1);

	/* step 3 */
	/* get the untransformed centre point */
	compute_centre_point(start_x, start_y,
			     end_x, end_y,
			     cx1, cy1,
			     radangle,
			     &cx, &cy);

	/* step 4 */
	/* compute anglestart and extent */
	compute_angle_start_extent(rx,ry,
				   x1,y1,
				   cx1, cy1,
				   &start, &extent);

	/* extent of 0 is a straight line */
	if (extent == 0) {
		return -1;
	}

	/* take account of sweep */
	if (!sweep && extent > 0) {
		extent -= TAU;
	} else if (sweep && extent < 0) {
		extent += TAU;
	}

	/* normalise start and extent */
	extent = fmod(extent, TAU);
	start = fmod(start, TAU);

	/* convert the arc to unit circle bezier curves */
	bzsegments = circle_arc_to_bezier(start, extent, bzpoints);

	/* transform the bezier curves */
	scale_rotate_translate_points(rx, ry,
				      radangle,
				      cx, cy,
				      bzsegments * 6,
				      bzpoints);

	return bzsegments;
}

void ParceSvgItems(svgItem *item){

    memset(&positions, -1, sizeof(positions));

    while(item != NULL){

        svgPathCommand *comm = NULL;
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
                        positions[num_positions].x = last_cubic_x = last_quad_x = last_x = x;
                        positions[num_positions].y = last_cubic_y = last_quad_y = last_y = y;
                        num_positions ++;

                }
                //	Vertical LineTo
                else if((comm->tId == SVG_PATH_CMD_ID_VERTICAL_LINETO_REL) || (comm->tId == SVG_PATH_CMD_ID_VERTICAL_LINETO_ABS)){
                    
                    x = comm->tParameters.tLineTo.tX.fValue;
                    y = comm->tParameters.tLineTo.tY.fValue; 

                    if(comm->tId == SVG_PATH_CMD_ID_VERTICAL_LINETO_REL)
					    y += last_y;

                    positions[num_positions].x = last_cubic_x = last_quad_x = last_x;
                    positions[num_positions].y = last_cubic_y = last_quad_y = last_y = y;
                    num_positions ++;
                }
                //	Horizontal LineTo
                else if((comm->tId == SVG_PATH_CMD_ID_HORIZONTAL_LINETO_REL) || (comm->tId == SVG_PATH_CMD_ID_HORIZONTAL_LINETO_ABS)){
                    
                    x = comm->tParameters.tLineTo.tX.fValue;
                    y = comm->tParameters.tLineTo.tY.fValue; 

                    if(comm->tId == SVG_PATH_CMD_ID_HORIZONTAL_LINETO_REL)
					    x += last_y;

                    positions[num_positions].x = last_cubic_x = last_quad_x = last_x = x;
                    positions[num_positions].y = last_cubic_y = last_quad_y = last_y;
                    num_positions ++;
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

                    positions[num_positions].x1 = x1;
                    positions[num_positions].y1 = y1;   
                    positions[num_positions].x2 = x2;
                    positions[num_positions].y2 = y2;    
                    positions[num_positions].x  = last_cubic_x = last_x = x;
                    positions[num_positions].y  = last_cubic_y = last_y = y;   
                    num_positions ++;
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

                    positions[num_positions].x1 = x1;
                    positions[num_positions].y1 = y1;
                    positions[num_positions].x2 = last_cubic_x =  x2;
                    positions[num_positions].y2 = last_cubic_y = y2;
                    positions[num_positions].x  = last_quad_x = last_x = x;
                    positions[num_positions].y  = last_quad_y = last_y = y;                        
                    num_positions ++;
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

                    positions[num_positions].x1 = 1./3 * last_x + 2./3 * x1;
                    positions[num_positions].y1 = 1./3 * last_y + 2./3 * y1;
                    positions[num_positions].x2 = 2./3 * x1 + 1./3 * x;
                    positions[num_positions].y2 = 2./3 * y1 + 1./3 * y;
                    positions[num_positions].x  = last_cubic_x = last_x = x;
                    positions[num_positions].y  = last_cubic_y = last_y = y;                        
                    num_positions ++;
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

                    positions[num_positions].x1 = 1./3 * last_x + 2./3 * x1;
                    positions[num_positions].y1 = 1./3 * last_y + 2./3 * y1;
                    positions[num_positions].x2 = 2./3 * x1 + 1./3 * x;
                    positions[num_positions].y2 = 2./3 * y1 + 1./3 * y;
                    positions[num_positions].x  = last_cubic_x = last_x = x;
                    positions[num_positions].y  = last_cubic_y = last_y = y;                        
                    num_positions ++;
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

                    
                    int bzsegments;
                    double bzpoints[6*4]; /* allow for up to four bezier segments per arc */

                    bzsegments = svgarc_to_bezier(last_x, last_y,
							      x, y,
							      rx, ry,
							      rotation,
							      large_arc,
							      sweep,
							      bzpoints);

                    if(bzsegments == -1){ 
                        positions[num_positions].x = x;
                        positions[num_positions].y = y;
                        num_positions ++;
				    } else if (bzsegments > 0) {
					    int bzpnt;
                        for (bzpnt = 0;bzpnt < (bzsegments * 6); bzpnt+=6) {
                            positions[num_positions].x1 = bzpoints[bzpnt];
                            positions[num_positions].y1 = bzpoints[bzpnt+1];
                            positions[num_positions].x2 = bzpoints[bzpnt+2];
                            positions[num_positions].y2 = bzpoints[bzpnt+3];
                            positions[num_positions].x  = bzpoints[bzpnt+4];
                            positions[num_positions].y  = bzpoints[bzpnt+5];
                            num_positions +=3;
                        }
                    }
				    if (bzsegments != 0) {
                        last_cubic_x = last_quad_x = last_x = positions[num_positions-1].x;
                        last_cubic_y = last_quad_y = last_y = positions[num_positions-1].y;
                    }
                }else if(comm->tId == SVG_PATH_CMD_ID_CLOSEPATH){
                    last_cubic_x = last_quad_x = last_x = subpath_first_x;
                    last_cubic_y = last_quad_y = last_y = subpath_first_y;
                }

                comm = comm->ptNextCommand;
            }

        }

        item = item->ptNextItem;
    }
}

int scale = 4;

float interpolate( float from , float to , float percent )
{
    float difference = to - from;
    return from + ( difference * percent );
}    

extern void AddConvexPolyFilled(const vec2 *points, const int points_count, vec3 col);

vec2 points[256];
uint32_t num_points = 0;
int main(){

    TEngineInitSystem(800, 600, "Test");

    Camera2DInit(&cam2D);
    Camera3DInit(&cam3D);

    svgDrawing* temp = svgOpenFile("temp.svg");

    ParceSvgItems(temp->tItemList.ptItem);

    float xa, ya, xb, yb, xc, yc, xm, ym, xn, yn, x, y, x1, y1, x2, y2, x3, y3, x4, y4;

    while(!TEngineWindowIsClosed()){

        TEnginePoolEvents();

        /*for(int i=0;i < num_positions;i++){
            
            if(i > 0){
                if(positions[i].x1 != -1){

                    points[num_points] = vec2_f(positions[i - 1].x * scale, positions[i - 1].y * scale);
                    num_points++;
                    for( float j = 0 ; j < 1 ; j += 0.1 )
                    {
                        x1 = positions[i - 1].x;
                        y1 = positions[i - 1].y;
                        x2 = positions[i - 0].x1;
                        y2 = positions[i - 0].y1;
                        x3 = positions[i - 0].x2;
                        y3 = positions[i - 0].y2;
                        x4 = positions[i - 0].x;
                        y4 = positions[i - 0].y;
                        
                        // The Green Lines
                        xa = interpolate( x1 , x2 , j );
                        ya = interpolate( y1 , y2 , j );
                        xb = interpolate( x2 , x3 , j );
                        yb = interpolate( y2 , y3 , j );
                        xc = interpolate( x3 , x4 , j );
                        yc = interpolate( y3 , y4 , j );

                        // The Blue Line
                        xm = interpolate( xa , xb , j );
                        ym = interpolate( ya , yb , j );
                        xn = interpolate( xb , xc , j );
                        yn = interpolate( yb , yc , j );

                        // The Black Dot
                        x = interpolate( xm , xn , j );
                        y = interpolate( ym , yn , j );
                        
                        points[num_points] = vec2_f(x * scale, y * scale);
                        num_points++;

                    }
                    
                    points[num_points] = vec2_f(positions[i].x * scale, positions[i].y * scale);
                    num_points++;
                }else{
                    
                    points[num_points + 0] = vec2_f(positions[i - 1].x * scale, positions[i - 1].y * scale);
                    points[num_points + 0] = vec2_f(positions[i].x * scale, positions[i].y * scale);
                    num_points += 2;
                }
            }
        }*/

        svgItem *item = temp->tItemList.ptItem;

        while(item != NULL){

            svgPathCommand *comm = NULL;
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
                        last_cubic_x = last_quad_x = last_x = x;
                        last_cubic_y = last_quad_y = last_y = y;
                        PathLineTo(vec2_f(x * scale, y * scale));

                    }
                    //	Vertical LineTo
                    else if((comm->tId == SVG_PATH_CMD_ID_VERTICAL_LINETO_REL) || (comm->tId == SVG_PATH_CMD_ID_VERTICAL_LINETO_ABS)){
                        
                        x = comm->tParameters.tLineTo.tX.fValue;
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
                        y = comm->tParameters.tLineTo.tY.fValue; 

                        if(comm->tId == SVG_PATH_CMD_ID_HORIZONTAL_LINETO_REL)
                            x += last_y;

                        last_cubic_x = last_quad_x = last_x = x;
                        last_cubic_y = last_quad_y = last_y;
                            
                        PathLineTo(vec2_f(x* scale, y* scale));
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
                        
                        PathBezierCubicCurveTo(vec2_f(x, y), vec2_f(x1, y1), vec2_f(x2, y2), 0);
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
                        
                        PathBezierCubicCurveTo(vec2_f(x, y), vec2_f(1./3 * last_x + 2./3 * x1, 1./3 * last_y + 2./3 * y1), vec2_f(2./3 * x1 + 1./3 * x, 2./3 * y1 + 1./3 * y), 0);

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

                        PathBezierCubicCurveTo(vec2_f(x, y), vec2_f(1./3 * last_x + 2./3 * x1, 1./3 * last_y + 2./3 * y1), vec2_f(2./3 * x1 + 1./3 * x, 2./3 * y1 + 1./3 * y), 0);
                        
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
                        PathArcTo(vec2_f(x, y), rx, 0, a_max, 0);
                    }else if(comm->tId == SVG_PATH_CMD_ID_CLOSEPATH){
                        last_cubic_x = last_quad_x = last_x = subpath_first_x;
                        last_cubic_y = last_quad_y = last_y = subpath_first_y;
                    }

                    comm = comm->ptNextCommand;
                }

                //PathFillConvex(vec3_f(1, 1, 1));
                PathStroke(vec3_f(1, 1, 1), true, 0.6);
                
            }

            item = item->ptNextItem;
        }

        
        TEngineRender();
    }

    TEngineCleanUp();

    return 0;
}