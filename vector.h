#ifndef _VECTOR_H_
#define _VECTOR_H_


typedef float vector_t[3];

#define vector_copy(a, b) (a)[0] = (b)[0]; \
                          (a)[1] = (b)[1]; \
                          (a)[2] = (b)[2];

void vector_add(vector_t, vector_t, vector_t);
void vector_sub(vector_t, vector_t, vector_t);
void vector_dec(vector_t, vector_t);
void vector_inc(vector_t, vector_t);
void vector_neg(vector_t);
void vector_scale(vector_t, float);
void vector_linear(vector_t, float, vector_t, float, vector_t);
void vector_norm(vector_t);
void vector_cross(vector_t, vector_t, vector_t);
void vector_zero(vector_t);
float vector_dot(vector_t, vector_t);
float vector_len(vector_t);


#endif /* _VECTOR_H_*/
