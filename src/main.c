//
//  main.c
//  Extension
//
//  Created by Carson S. on 9/10/24.
//

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "pd_api.h"
#include "linear_algebra.h"


static int update(void* userdata);
matrix4d* make_m(void);
void WUV(vector3d* W, vector3d* U, vector3d* V, float* f, float* a, float* u);
int* get_line(float* points);
void plot_point(float* p, PlaydateAPI* pd);
void to_2d(PlaydateAPI* pd);
void draw_square(PlaydateAPI* pd, int* square, bool edges, LCDSolidColor color);
void calc_normals(void);
bool isBackface(int* square);
void set_rotates(void);
void rotate_coords(matrix4d* rotater, vector4d** points_v);
void colorsInverted();
void solidFill();
void showBackFace();

const char* fontpath = "fonts/Asheville-Sans-14-Bold.pft";
LCDFont* font = NULL;



#define TEXT_WIDTH 86
#define TEXT_HEIGHT 16
#define T_ROWS 3
#define X 400
#define Y 240
#define PI 3.1415

float RADIAN_TO_DEGREE = 180 / PI;


#define NUM_POINTS 8
#define NUM_POLYGONS 6

bool inverted = false;
LCDSolidColor background_color = kColorWhite;
LCDSolidColor line_color = kColorBlack;
bool fill_squares = false;
bool backfaces = false;


float p1[3] = { -1, -1, -1 };
float p2[3] = { -1, 1, -1 };
float p3[3] = { 1, 1, -1 };
float p4[3] = {1, -1, -1};
float p5[3] = { -1, -1, 1 };
float p6[3] = { -1, 1, 1 };
float p7[3] = { 1, 1, 1 };
float p8[3] = { 1, -1, 1 };



float from[3] = {0,0.1,10.};
float at[3] =   {0.,0.,0.};
float up[3] =   {0.,0.,1.};
float spin = 0.0374533;

vector3d* W;
vector3d* U;
vector3d* V;

matrix4d* rotate_X;
matrix4d* rotate_Y;
matrix4d* rotate_Z;

matrix4d* rotate_X_n;
matrix4d* rotate_Y_n;
matrix4d* rotate_Z_n;


matrix4d* M;

float* points[NUM_POINTS] = { p1, p2, p3, p4, p5, p6, p7, p8 };
int* points2d[NUM_POINTS];

int square1[4] = { 3,2,1,0 };
int square2[4] = { 7,6,2,3 };
int square3[4] = { 4,5,6,7 };
int square4[4] = { 0,1,5,4 };
int square5[4] = { 5,1,2,6 };
int square6[4] = { 0,4,7,3 };

int* squares[NUM_POLYGONS] = { square1, square2, square3, square4, square5, square6 };
vector3d* normals[NUM_POLYGONS];


#ifdef _WINDLL
__declspec(dllexport)
#endif
int eventHandler(PlaydateAPI* pd, PDSystemEvent event, uint32_t arg)
{
	(void)arg; // arg is currently only used for event = kEventKeyPressed

	if ( event == kEventInit )
	{
		const char* err;
		font = pd->graphics->loadFont(fontpath, &err);
		
		if ( font == NULL )
			pd->system->error("%s:%i Couldn't load font %s: %s", __FILE__, __LINE__, fontpath, err);

		pd->system->addCheckmarkMenuItem("Invert", 0, colorsInverted, pd);
		pd->system->addCheckmarkMenuItem("Solid Fill", 0, solidFill, pd);
		pd->system->addCheckmarkMenuItem("Back Faces", 0, showBackFace, pd);
		// Note: If you set an update callback in the kEventInit handler, the system assumes the game is pure C and doesn't run any Lua code in the game
		pd->system->setUpdateCallback(update, pd);
		W = new_vector3d_zero();
		U = new_vector3d_zero();
		V = new_vector3d_zero();
		WUV(W, U, V, from, at, up);
		M = make_m();
		set_rotates();
		
		to_2d(pd);
		
	}
	else if (event == kEventTerminate)
	{
		free(W);
		free(U);
		free(V);
		free(rotate_X);
		free(rotate_Y);
		free(rotate_Z);
		free(rotate_X_n);
		free(rotate_Y_n);
		free(rotate_Z_n);
		free(M);
		for (int i = 0; i < NUM_POLYGONS; i++)
		{
			pd->system->realloc(points2d[i], 0);
		}
	}
	return 0;
}


static int update(void* userdata)
{
	PlaydateAPI* pd = userdata;
	
	pd->graphics->clear(background_color);
	pd->graphics->setFont(font);
	char output[25];
	char* a = "Rotation Speed: ";
	int speed = (int)(spin * RADIAN_TO_DEGREE);
	sprintf(output, "%s %d", a, speed);

	pd->graphics->setDrawMode(kDrawModeNXOR);
	pd->graphics->drawText(output, strlen(output), kASCIIEncoding, 5, 220);



	float crank = pd->system->getCrankChange();
	if (crank != 0)
	{
		spin += (crank > 0) ? (float).005 : (float) - .005;
		if (spin < (float)0.0174533)
			spin = (float)0.0174533;
		if (spin > (float)17.4527)
			spin = (float)17.4527;
		set_rotates();
	}

	PDButtons curr;
	pd->system->getButtonState(&curr, NULL, NULL);
	if (curr != 0)
	{
		vector4d* v_points[NUM_POINTS];
		for (int i = 0; i < NUM_POINTS; i++)
		{
			float h[4] = { points[i][0],points[i][1],points[i][2],1 };
			v_points[i] = new_vector4d(h);
		}

		//rotate coords based on buttons
		if (curr & kButtonUp)
			rotate_coords(rotate_X,v_points);
		if (curr & kButtonDown)
			rotate_coords(rotate_X_n,v_points);
		if (curr & kButtonRight)
			rotate_coords(rotate_Y_n,v_points);
		if (curr & kButtonLeft)
			rotate_coords(rotate_Y,v_points);
		if (curr & kButtonA)
			rotate_coords(rotate_Z_n,v_points);
		if (curr & kButtonB)
			rotate_coords(rotate_Z,v_points);

		for (int i = 0; i < NUM_POINTS; i++)
			free(v_points[i]);

		//convert 3d points to 2d space
		to_2d(pd);
	}
	
	LCDColor c = line_color;
	if (fill_squares)
	{
		for (int i = 0; i < NUM_POLYGONS; i++)
			draw_square(pd, squares[i],false,line_color);
		c = background_color;
	}
	for (int i = 0; i < NUM_POLYGONS; i++)
		draw_square(pd, squares[i],true,c);

        
	pd->system->drawFPS(0,0);

	return 1;
}

void WUV(vector3d* W, vector3d* U, vector3d* V, float* f, float* a, float* u)
{
	//Calculate W
	subtract(a, f, W->data, 3);
	normalize_vector3d(W);

	//Calculate U
	vector3d* up_v = new_vector3d(u);
	cross_vector3d(up_v, W, U);
	normalize_vector3d(U);
	free(up_v);

	//Calculate V
	cross_vector3d(W, U, V);
	normalize_vector3d(V);
}

matrix4d* make_m(void)
{
	float m_vp[16] = { X / 2., 0, 0, (X - 1) / 2.,
					  0,Y / 2., 0, (Y - 1) / 2.,
					  0,0,1,0,
					  0,0,0,1 };
	matrix4d* vp = new_matrix4d(m_vp);
	float cam1[16] = { U->data[0],U->data[1],U->data[2],0,
					  V->data[0],V->data[1],V->data[2],0,
					  W->data[0],W->data[1],W->data[2],0,
					  0,0,0,1 };

	float cam2[16] = { 1,0,0,-1 * from[0],
					  0,1,0,-1 * from[1],
					  0,0,1,-1 * from[2],
					  0,0,0,1 };

	matrix4d* c1 = new_matrix4d(cam1);
	matrix4d* c2 = new_matrix4d(cam2);
	matrix4d* cam = new_matrix4d_zero();
	multiply_matrix4d(c1, c2, cam);
	free(c1);
	free(c2);

	float n, f, l, r, t, b;
	float radian = 2 * PI * (45 / 360.);
	n = -1;
	f = -1000;
	l = -1 * (tan(radian / 2));
	r = -l;
	t = r * (float).6;
	b = l * (float).6;
	float per_m[16] = { (2 * n) / (r - l), 0, (l + r) / (l - r), 0,
						0, (2 * n) / (t - b), (t + b) / (b - t), 0,
						0, 0, (f + n) / (n - f), (2 * f * n) / (f - n),
						0, 0, 1, 0 };
	matrix4d* per = new_matrix4d(per_m);
	matrix4d* m = new_matrix4d_zero();
	matrix4d* hold = new_matrix4d_zero();

	multiply_matrix4d(vp, per, hold);
	multiply_matrix4d(hold, cam, m);
	free(vp);
	free(per);
	free(cam);
	free(hold);
	return m;
}

//this was an early test function (i dont use it anymore) (should delete)
int* get_line(float* points)
{
	float p1f[4] = { points[0],points[1],points[2],1 };
	float p2f[4] = { points[3], points[4], points[5], 1 };
	int * r = malloc(sizeof(int[4]));
	vector4d* p1 = new_vector4d(p1f);
	vector4d* p2 = new_vector4d(p2f);
	vector4d* h1 = new_vector4d_zero();
	vector4d* h2 = new_vector4d_zero();
	multiply_matrix_vector_4d(M, p1, h1);
	multiply_matrix_vector_4d(M, p2, h2);
	if (r != NULL)
	{
		r[0] = (int)(h1->data[0] / ((h1->data[3] != 0) ? h1->data[3] : 1));
		r[1] = (int)(h1->data[1] / ((h1->data[3] != 0) ? h1->data[3] : 1));

		r[2] = (int)(h2->data[0] / ((h2->data[3] != 0) ? h2->data[3] : 1));
		r[3] = (int)(h2->data[1] / ((h2->data[3] != 0) ? h2->data[3] : 1));
	}
	free(p1);
	free(p2);
	free(h1);
	free(h2);
	return r;
}

//this was an early test function (i dont use it anymore) (should delete)
void plot_point(float* p, PlaydateAPI *pd)
{
	float pf[4] = { p[0], p[1], p[2], 1 };
	vector4d* p1 = new_vector4d(pf);
	vector4d* h1 = new_vector4d_zero();
	multiply_matrix_vector_4d(M, p1, h1);
	int x = (int)(h1->data[0] / h1->data[3]);
	int y = (int)(h1->data[1] / h1->data[3]);
	pd->graphics->drawEllipse(x, y, 5, 5, 5, 0, 0, kColorBlack);
}

void to_2d(PlaydateAPI *pd)
{
	for (int i = 0; i < NUM_POINTS; i++)
	{
		float pf[4] = { points[i][0], points[i][1], points[i][2], 1 };
		vector4d* p1 = new_vector4d(pf);
		vector4d* h1 = new_vector4d_zero();
		multiply_matrix_vector_4d(M, p1, h1);
		int x = (int)(h1->data[0] / h1->data[3]);
		int y = (int)(h1->data[1] / h1->data[3]);
		int* twod;
		twod = pd->system->realloc(NULL, sizeof(int[2]));
		if (twod != NULL)
		{
			memcpy(twod, &x, sizeof(int));
			memcpy(twod + 1, &y, sizeof(int));
		}

		if (points2d[i] != NULL)
			pd->system->realloc(points2d[i], 0);
		points2d[i] = twod;
		free(p1);
		free(h1);
	}
}

void draw_square(PlaydateAPI* pd, int* square, bool edges, LCDSolidColor color)
{
	if (backfaces || isBackface(square)) //should rename this func to isnotBackface
	{
		int sides[8];
		for (int i = 0; i < 4; i++)
			memcpy(sides + (i * 2), *(points2d + *(square + i)), sizeof(int[2]));
		if (edges)
		{
			pd->graphics->drawLine(sides[0], sides[1], sides[2], sides[3], 3, color);
			pd->graphics->drawLine(sides[2], sides[3], sides[4], sides[5], 3, color);
			pd->graphics->drawLine(sides[4], sides[5], sides[6], sides[7], 3, color);
			pd->graphics->drawLine(sides[6], sides[7], sides[0], sides[1], 3, color);
		}
		else
			pd->graphics->fillPolygon(4, sides, color, kPolygonFillNonZero);
	}	
}

void calc_normals(void)
{
	for (int i = 0; i < NUM_POLYGONS; i++)
	{
		//N = ((points[1] - points[0]).cross(points[2] - points[1])).normalized();
		//float *normal = malloc(sizeof(float[3]));
		vector3d* normal = new_vector3d_zero();
		float p10[3], p21[3];
		subtract(points[squares[i][1]], points[squares[i][0]], p10, 3);
		subtract(points[squares[i][2]], points[squares[i][1]], p21, 3);
		vector3d* a = new_vector3d(p10);
		vector3d* b = new_vector3d(p21);
		cross_vector3d(a, b, normal);
		normalize_vector3d(normal);
		normals[i] = normal;
	}
}

bool isBackface(int* square)
{
	int* a, * b, * c, * d;
	a = points2d[square[0]];
	b = points2d[square[1]];
	c = points2d[square[2]];
	d = points2d[square[3]];
	int sum = 0;
	sum += (a[0] - b[0]) * (a[1] + b[1]);
	sum += (b[0] - c[0]) * (b[1] + c[1]);
	sum += (c[0] - d[0]) * (c[1] + d[1]);
	sum += (d[0] - a[0]) * (d[1] + a[1]);
	
	return (sum > 0) ? true : false;	
}

void set_rotates(void)
{
	float c = cos(spin);
	float s = sin(spin);
	float rx[16] = { 1, 0, 0, 0,
					0, c, -s, 0,
					0, s, c, 0,
					0, 0, 0, 1 };

	float ry[16] = { c, 0, s, 0,
					0, 1, 0, 0,
					-s, 0, c, 0,
					0, 0, 0, 1 };

	float rz[16] = { c, -s, 0, 0,
					 s, c, 0, 0,
					 0, 0, 1, 0,
					 0, 0, 0, 1 };

	float rxn[16] = { 1, 0, 0, 0,
					  0, c, s, 0,
					  0, -s, c, 0,
					  0, 0, 0, 1 };

	float ryn[16] = { c, 0, -s, 0,
					0, 1, 0, 0,
					s, 0, c, 0,
					0, 0, 0, 1 };

	float rzn[16] = { c, s, 0, 0,
					-s, c, 0, 0,
					0, 0, 1, 0,
					0, 0, 0, 1 };
	rotate_X = new_matrix4d(rx);
	rotate_Y = new_matrix4d(ry);
	rotate_Z = new_matrix4d(rz);
	rotate_X_n = new_matrix4d(rxn);
	rotate_Y_n = new_matrix4d(ryn);
	rotate_Z_n = new_matrix4d(rzn);
}

void rotate_coords(matrix4d* rotater, vector4d** points_v)
{
	for (int i = 0; i < NUM_POINTS; i++)
	{
		vector4d* out = new_vector4d_zero();
		multiply_matrix_vector_4d(rotater, points_v[i], out);
		memcpy(points[i], out->data, sizeof(float[3]));
		memcpy(points_v[i]->data, out->data, sizeof(float[4]));
		free(out);
	}
}

void colorsInverted()
{
	inverted = !inverted;
	if (inverted)
	{
		background_color = kColorBlack;
		line_color = kColorWhite;
	}
	else
	{
		background_color = kColorWhite;
		line_color = kColorBlack;
	}
}

void solidFill()
{
	fill_squares = !fill_squares;
}

void showBackFace()
{
	backfaces = !backfaces;
}