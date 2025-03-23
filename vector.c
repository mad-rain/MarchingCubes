#include <math.h>
#include "vector.h"

/* vector zeroring */
void
vector_zero(vector_t a)
{
        a[0] = 0.0f;
        a[1] = 0.0f;
        a[2] = 0.0f;
}

/* dot product of two vectors */
float
vector_dot(vector_t a, vector_t b)
{
        return a[0] * b[0] + a[1] * b[1] + a[2] * b[2];
}

/* cross product of two vectors */
void
vector_cross(vector_t a, vector_t b, vector_t c)
{
        c[0] = a[1] * b[2] - a[2] * b[1];
        c[1] = a[2] * b[0] - a[0] * b[2];
        c[2] = a[0] * b[1] - a[1] * b[0];
}

/* vector negation */
void
vector_neg(vector_t a)
{
        a[0] = -a[0];
        a[1] = -a[1];
        a[2] = -a[2];
}

/* vector scaling */
void
vector_scale(vector_t a, float s)
{
        a[0] *= s;
        a[1] *= s;
        a[2] *= s;
}

/* vector increment */
void
vector_inc(vector_t a, vector_t b)
{
        a[0] += b[0];
        a[1] += b[1];
        a[2] += b[2];
}

/* vector decrement */
void
vector_dec(vector_t a, vector_t b)
{
        a[0] -= b[0];
        a[1] -= b[1];
        a[2] -= b[2];
}

/* vector addition */
void
vector_add(vector_t a, vector_t b, vector_t c)
{
        c[0] = a[0] + b[0];
        c[1] = a[1] + b[1];
        c[2] = a[2] + b[2];
}

/* vector substraction */
void
vector_sub(vector_t a, vector_t b, vector_t c)
{
        c[0] = a[0] - b[0];
        c[1] = a[1] - b[1];
        c[2] = a[2] - b[2];
}

/* linear combination of two vectors */
void
vector_linear(vector_t a, float as, vector_t b, float bs, vector_t c)
{
        c[0] = a[0] * as + b[0] * bs;
        c[1] = a[1] * as + b[1] * bs;
        c[2] = a[2] * as + b[2] * bs;
}

/* length of a vector */
float
vector_len(vector_t a)
{
        return sqrt(a[0] * a[0] + a[1] * a[1] + a[2] * a[2]);
}

/* squared length of a vector */
float
vector_len2(vector_t a)
{
        return a[0] * a[0] + a[1] * a[1] + a[2] * a[2];
}

/* normalization of a vector */
void
vector_norm(vector_t a)
{
        float len = sqrt(a[0] * a[0] + a[1] * a[1] + a[2] * a[2]);

        if (len != 0.0f) {
                float inv_len = 1.0f / len;
        
                a[0] *= inv_len;
                a[1] *= inv_len;
                a[2] *= inv_len;
        }
}

float
vector_cos(vector_t a, vector_t b)
{
        float l0 = vector_len(a);
        float l1 = vector_len(b);

        return (a[0] * b[0] + a[1] * b[1] + a[2] * b[2]) / (l0 * l1);
}

void
vector_lerp(vector_t a, vector_t b, float t1, vector_t c)
{
        float t0 = 1.0f - t1;

        c[0] = a[0] * t0 + b[0] * t1;
        c[1] = a[1] * t0 + b[1] * t1;
        c[2] = a[2] * t0 + b[2] * t1;
}

