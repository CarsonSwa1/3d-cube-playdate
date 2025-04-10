#pragma once

#define FLOAT2 sizeof(float[2])

// Generic Matrix/Vector
void add(float* a, float* b, float* c, int size);
void subtract(float* a, float* b, float* c, int size);

//Matrix4d (4 x 4 matrix)
typedef struct { float data[16]; } matrix4d;
matrix4d* new_matrix4d(float data[16]);
matrix4d* new_matrix4d_zero();
void multiply_matrix4d(matrix4d* a, matrix4d* b, matrix4d* dest);

//Vector3d (3 x 1 matrix)
typedef struct { float data[3]; } vector3d;
vector3d* new_vector3d(float data[3]);
vector3d* new_vector3d_zero();
void normalize_vector3d(vector3d* a);
void cross_vector3d(vector3d* a, vector3d* b, vector3d* dest);

//Vector4d (4 x 1 matrix)
typedef struct { float data[4]; } vector4d;
vector4d* new_vector4d(float data[4]);
vector4d* new_vector4d_zero();
void multiply_matrix_vector_4d(matrix4d* a, vector4d* b, vector4d* dest);