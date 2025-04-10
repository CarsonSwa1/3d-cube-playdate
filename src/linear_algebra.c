#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#include "linear_algebra.h"


////****Helper Functions *****//////

//gets a 2x2 "chunk" from a 4x4 matrix
//index is the starting index of the 2x2 matrix
void chunk(float a[16], float a1[4], int index) {
    a1[0] = a[index];
    a1[1] = a[index + 1];
    a1[2] = a[index + 4];
    a1[3] = a[index + 5];
}

//product of two 2 x 2 matrices
void two_multiply(float a[4], float b[4], float c[4]) {
    float p1, p2, p3, p4, p5, p6, p7;
    p1 = (a[0] + a[3]) * (b[0] + b[3]);
    p2 = a[3] * (b[2] - b[0]);
    p3 = b[3] * (a[0] + a[1]);
    p4 = (a[1] - a[3]) * (b[2] + b[3]);
    p5 = a[0] * (b[1] - b[3]);
    p6 = b[0] * (a[2] + a[3]);
    p7 = (a[0] - a[2]) * (b[0] + b[1]);
    c[0] += p1 + p2 - p3 + p4;
    c[1] += p5 + p3;
    c[2] += p6 + p2;
    c[3] += p5 + p1 - p6 - p7;
}

////////////////////////////////////



//***** Generic Matrix/Vector Functions
void add(float* a, float* b, float* c, int size)
{
    for (int i = 0; i < size; i++)
        c[i] = a[i] + b[i];
}
void subtract(float* a, float* b, float* c, int size)
{
    for (int i = 0; i < size; i++)
        c[i] = a[i] - b[i];
}

//***** Matrix4d Functions *****
matrix4d* new_matrix4d(float data[16])
{
    matrix4d* p = malloc(sizeof(matrix4d));
    memcpy(p->data, data, sizeof(float[16]));
    return p;
}
matrix4d* new_matrix4d_zero()
{
    matrix4d* p = malloc(sizeof(matrix4d));
    memset(p->data, 0, sizeof(float[16]));
    return p;
}
void multiply_matrix4d(matrix4d* a, matrix4d* b, matrix4d* dest)
{
    float cl[16] = { 0.,0.,0.,0.,0.,0.,0.,0.,0.,0.,0.,0.,0.,0.,0.,0. };
    float al[16], bl[16];
    memcpy(al, a->data, sizeof(float[16]));
    memcpy(bl, b->data, sizeof(float[16]));

    float a1[4], b1[4];
    float c1[4] = { 0., 0., 0., 0. };
    chunk(al, a1, 0);
    chunk(bl, b1, 0);
    two_multiply(a1, b1, c1);
    chunk(al, a1, 2);
    chunk(bl, b1, 8);
    two_multiply(a1, b1, c1);
    memcpy(cl, c1, FLOAT2);
    memcpy(cl + 4, c1 + 2, FLOAT2);

    memset(c1, 0, sizeof(float[4]));
    chunk(al, a1, 0);
    chunk(bl, b1, 2);
    two_multiply(a1, b1, c1);
    chunk(al, a1, 2);
    chunk(bl, b1, 10);
    two_multiply(a1, b1, c1);
    memcpy(cl + 2, c1, FLOAT2);
    memcpy(cl + 6, c1 + 2, FLOAT2);

    memset(c1, 0, sizeof(float[4]));
    chunk(al, a1, 8);
    chunk(bl, b1, 0);
    two_multiply(a1, b1, c1);
    chunk(al, a1, 10);
    chunk(bl, b1, 8);
    two_multiply(a1, b1, c1);
    memcpy(cl + 8, c1, FLOAT2);
    memcpy(cl + 12, c1 + 2, FLOAT2);

    memset(c1, 0, sizeof(float[4]));
    chunk(al, a1, 8);
    chunk(bl, b1, 2);
    two_multiply(a1, b1, c1);
    chunk(al, a1, 10);
    chunk(bl, b1, 10);
    two_multiply(a1, b1, c1);
    memcpy(cl + 10, c1, FLOAT2);
    memcpy(cl + 14, c1 + 2, FLOAT2);

    memcpy(dest->data, cl, sizeof(float[16]));
}

//***** Vector3d Functions *****
vector3d* new_vector3d(float data[3])
{
    vector3d* p = malloc(sizeof(vector3d));
    memcpy(p->data, data, sizeof(float[3]));
    return p;
}
vector3d* new_vector3d_zero()
{
    vector3d* p = malloc(sizeof(vector3d));
    memset(p->data, 0, sizeof(float[3]));
    return p;
}
void normalize_vector3d(vector3d* a)
{
    float* d = a->data;
    float w = sqrt(d[0] * d[0] + d[1] * d[1] + d[2] * d[2]);
    if (w != 0.)
    {
        d[0] /= w;
        d[1] /= w;
        d[2] /= w;
    }
}
void cross_vector3d(vector3d* a, vector3d* b, vector3d* dest)
{
    float* aa = a->data;
    float* bb = b->data;
    dest->data[0] = aa[1] * bb[2] - aa[2] * bb[1];
    dest->data[1] = 0 - (aa[0] * bb[2] - aa[2] * bb[0]);
    dest->data[2] = aa[0] * bb[1] - aa[1] * bb[0];
}

//***** Vector4d Functions *****
vector4d* new_vector4d(float data[4])
{
    vector4d* p = malloc(sizeof(vector4d));
    memcpy(p->data, data, sizeof(float[4]));
    return p;
}
vector4d* new_vector4d_zero()
{
    vector4d* p = malloc(sizeof(vector4d));
    memset(p->data, 0, sizeof(float[4]));
    return p;
}
void multiply_matrix_vector_4d(matrix4d* a, vector4d* b, vector4d* dest)
{
    float* aa = a->data;
    float* bb = b->data;
    int ctr = 0;
    for (int i = 0; i < 4; i++)
    {
        dest->data[i] = aa[ctr] * bb[0] + aa[ctr + 1] * bb[1] + aa[ctr + 2] * bb[2] + aa[ctr + 3] * bb[3];
        ctr += 4;
    }
}
