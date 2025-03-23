/*                                             */
/* Copyright (c) 2002 Mad Rain. Fenomen group. */
/* Marching cubes module.                      */
/*                                             */


#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mcubes.h"


#define HASH_TABLE_SIZE         16384
#define HASH_TABLE_MAX_ITEMS    16
#define QUEUE_MAX_SIZE          4096

// #define HASH_VALUE(x, y, z) ((x) * 1213 + (y) * 1217 + (z) * 1223)
#define HASH_VALUE(x, y, z) ((((x << 9) ^ (y << 5) ^ z ^ (y >> 5) ^ (x >> 9)) + 17 * (x + y + z)) & (HASH_TABLE_SIZE - 1))

#define forever for (;;)

typedef unsigned short face_t[3];

typedef struct {
    int coord;
    int axis;
} cube_edge_t;

typedef struct {
    int vertex[3];
    int label;
    int iso_pos; /* -1 inside, 0 outsize */
    int x, y, z;
    float value;
} cube_vertex_t;

typedef struct {
    int x, y, z;
    cube_vertex_t *vertex;
} queue_item_t;

static unsigned char face_table[256][32];
static short edge_table[256];

static float (*mcubes_func)(int, int, int);

static vector_t *vertex_list;
static vector_t *current_vertex;
static int vertices;
static int max_vertices;

static face_t *face_list;
static face_t *current_face;
static int faces;
static int max_faces;

static cube_vertex_t mcubes_hash_table[HASH_TABLE_SIZE][HASH_TABLE_MAX_ITEMS];
static int hash_table_items[HASH_TABLE_SIZE];

static queue_item_t queue_data[QUEUE_MAX_SIZE];
static queue_item_t *queue_head = queue_data;
static queue_item_t *queue_tail = queue_data;

static int queue_items = 0;
static int queue_max_items = 0;

static vector_t **vertex_list_ptr;
static int *vertices_ptr;
static int *max_vertices_ptr;

static face_t **face_list_ptr;
static int *faces_ptr;
static int *max_faces_ptr;

void
mcubes_init()
{
    static unsigned char iso_base[] = {
        1,   0x01, 0x8f,
        3,   0x92, 0x88, 0x21, 0xff,
        6,   0x23, 0xa2, 0xa9, 0x9a, 0x00, 0xa1, 0x20, 0x12, 0x13, 0xff,
        7,   0x3a, 0x89, 0x38, 0x92, 0x3f,
        15,  0x89, 0xaa, 0x9b, 0xff,
        22,  0x08, 0x1a, 0x53, 0x54, 0x33, 0x42, 0x24, 0x9f,
        23,  0x53, 0xa2, 0x35, 0x42, 0x59, 0x24, 0xff,
        24,  0x32, 0xb8, 0x54, 0xff,
        25,  0x24, 0x02, 0xb4, 0x31, 0x53, 0x5b, 0x54, 0xbf,
        27,  0x53, 0x14, 0x35, 0xb3, 0x49, 0xb4, 0xff,
        29,  0x02, 0x42, 0xb4, 0x4b, 0x55, 0xba, 0xff,
        30,  0x10, 0x84, 0x9b, 0x54, 0xb5, 0xba, 0xff,
        60,  0x98, 0x12, 0x91, 0x6a, 0x56, 0xba, 0xff,
        105, 0x02, 0x98, 0x45, 0x1a, 0x3b, 0x76, 0xff,
        0
    };
    static unsigned char iso_rotate[] = {
        2, 0, 3, 1, 6, 4, 7, 5,
        2, 3, 6, 7, 0, 1, 4, 5,
        1, 5, 3, 7, 0, 4, 2, 6
    };
    static unsigned char edge[12][2] = {
        {0, 1}, {0, 2}, {1, 3}, {2, 3},
        {4, 5}, {4, 6}, {5, 7}, {6, 7},
        {0, 4}, {1, 5}, {2, 6}, {3, 7}
    };
    static unsigned char iso_rotatetb[] = {
        0x40, 0x11, 0x42, 0x11, 0x40, 0x11,
        0x42, 0x10, 0x41, 0x20, 0x41, 0x00
    };
    int n1, n2, ph;
    int i, j, pos_size;
    unsigned char index;
    unsigned char *base = iso_base;
    unsigned char *iso_pos_src;
    unsigned char *iso_pos_dest;
    unsigned char incd[8][8];
    unsigned char iso_pos[32];

    memset(face_table, 0xff, sizeof(face_table));

    for (i = 0; i < 12; i ++) {
        incd[edge[i][0]][edge[i][1]] = i;
        incd[edge[i][1]][edge[i][0]] = i;
    }

    for (i = 0; i < 256; i ++) {
        int ed = 0;
        
        for (j = 0; j < 12; j ++)
            if ((i >> edge[j][0]) & 1 ^
                (i >> edge[j][1]) & 1)
                ed |= 1 << j;
                
        edge_table[i] = ed;
    }

    forever {
        if ((index = *base ++) == 0)
            break;

        iso_pos_src = &face_table[index][0];
        
        pos_size = 0;
        
        forever {
            n1 = *base >> 4;
            n2 = *base & 0x0f;
            base ++;
            
            if (n1 != 0x0f) iso_pos_src[pos_size ++] = n1;
                       else break;
                       
            if (n2 != 0x0f) iso_pos_src[pos_size ++] = n2;
                       else break;
        }

        ph = 0;

        do {
            unsigned char *rtt = iso_rotatetb;
            int rt, count;
            
            if (ph) {
                for (i = 0; i < pos_size; i += 3) {
                    iso_pos[i    ] = iso_pos_src[i    ];
                    iso_pos[i + 1] = iso_pos_src[i + 2];
                    iso_pos[i + 2] = iso_pos_src[i + 1];
                }

                iso_pos_src = iso_pos;
            }
            
            forever {           
                if ((rt = *rtt) == 0)
                    break;

                count = rt >> 4;
                
                do {
                    unsigned char *t = iso_rotate + (rt & 0xf) * 8;
                    unsigned char *t2 = t;
                    unsigned char new_index = 0;
                    
                    do {
                        if (index & 1)
                            new_index |= 1 << *t2;
                            
                        t2 ++;
                        index >>= 1;
                        
                    } while (index);
                    
                    index = new_index;
                    
                    iso_pos_dest = &face_table[index][0];
                    
                    for (i = 0; i < pos_size; i ++)
                        iso_pos_dest[i]=incd[t[edge[iso_pos_src[i]][0]]]
                                            [t[edge[iso_pos_src[i]][1]]];
                                            
                    iso_pos_src = iso_pos_dest;
                    
                } while (--count);

                rtt ++;
            }

            index = ~index;
            ph = ~ph;
            
        } while (ph);
    }
}

static cube_vertex_t *
compute_func_value(int x, int y, int z)
{
    cube_vertex_t *item;
    cube_vertex_t new_item, *new_item_dest;
    int hash_value = HASH_VALUE(x, y, z) & (HASH_TABLE_SIZE - 1);
    int items = hash_table_items[hash_value];

    item = &mcubes_hash_table[hash_value][0];

    if (items) {
        int count = items;
    
        do {
            if (item->x == x &&
                item->y == y &&
                item->z == z)
                return item;
                
            item ++;
            
        } while (--count);
    }
    
    new_item.x = x;
    new_item.y = y;
    new_item.z = z;
    new_item.iso_pos = (new_item.value = mcubes_func(x, y, z)) > 0 ? -1 : 0;
    new_item.label = 0;
    new_item.vertex[0] =
    new_item.vertex[1] =
    new_item.vertex[2] = -1;

    *item = new_item;
    
    hash_table_items[hash_value] ++;
    
    return item;
}

static void
queue_remove(int *x, int *y, int *z, cube_vertex_t **vertex)
{
    *x = queue_tail->x;
    *y = queue_tail->y;
    *z = queue_tail->z;
    *vertex = queue_tail->vertex;

    if (queue_tail == queue_data + QUEUE_MAX_SIZE - 1)
            queue_tail = queue_data;
    else    queue_tail ++;
    
    queue_items --;
}

static void
queue_insert(int x, int y, int z, cube_vertex_t *vertex)
{
    if (vertex->label == 0) {

        vertex->label = 1;
    
        queue_head->x = x;
        queue_head->y = y;
        queue_head->z = z;
        queue_head->vertex = vertex;

        if (queue_head == queue_data + QUEUE_MAX_SIZE - 1)
                queue_head = queue_data;
        else    queue_head ++;
        
        queue_items ++;
        
        /*
        if (queue_max_items < queue_items)
            queue_max_items = queue_items;
        */
    }
}

#ifdef MCUBES_USE_STATIC_LISTS

void
mcubes_begin(vector_t *_vertex_list, int *_vertices_ptr,
             face_t *_face_list, int *_faces_ptr,
             float (*_mcubes_func)(int, int, int))
{
    current_vertex = _vertex_list;
    vertices_ptr = _vertices_ptr;

    current_face = _face_list;
    faces_ptr = _faces_ptr;

    mcubes_func = _mcubes_func;

    vertices = 0;
    faces = 0;
    
    memset(hash_table_items, 0, sizeof(hash_table_items));
}

void
mcubes_end()
{
    *vertices_ptr = vertices;
    
    *faces_ptr = faces;

#if 0    
    {
    	int i;
    	FILE *fout = fopen("hash", "wt");

    	for (i = 0; i < HASH_TABLE_SIZE; i ++)
    		fprintf(fout, "%d\n", hash_table_items[i]);
    	fclose(fout);
    }
#endif
}

#else

void
mcubes_begin(vector_t **_vertex_list_ptr, int *_vertices_ptr, int *_max_vertices_ptr,
             face_t **_face_list_ptr, int *_faces_ptr, int *_max_faces_ptr,
             float (*_mcubes_func)(int, int, int))
{
    vertex_list_ptr = _vertex_list_ptr;
    max_vertices_ptr = _max_vertices_ptr;
    vertices_ptr = _vertices_ptr;

    if (*max_vertices_ptr == 0) {
        vertex_list = (vector_t *) malloc(sizeof(vector_t) * (max_vertices = 128));
    } else {
        max_vertices = *max_vertices_ptr;
        vertex_list = *vertex_list_ptr;
    }

    face_list_ptr = _face_list_ptr;
    max_faces_ptr = _max_faces_ptr;
    faces_ptr = _faces_ptr;

    if (*max_faces_ptr == 0) {
        face_list = (face_t *) malloc(sizeof(face_t) * (max_faces = 128));
    } else {
        max_faces = *max_faces_ptr;
        face_list = *face_list_ptr;
    }
    
    mcubes_func = _mcubes_func;

    vertices = 0;
    faces = 0;
    
    current_vertex = vertex_list;
    current_face = face_list;
    
    memset(hash_table_items, 0, sizeof(hash_table_items));
}

void
mcubes_end()
{
    *vertex_list_ptr = vertex_list;
    *vertices_ptr = vertices;
    *max_vertices_ptr = max_vertices;
    
    *face_list_ptr = face_list;
    *faces_ptr = faces;
    *max_faces_ptr = max_faces;
}

#endif /* MCUBES_USE_STATIC_LISTS */

void
mcubes_polygonize(int xb, int yb, int zb)
{
    cube_vertex_t *cube_vertex[2][2][2];
    cube_vertex_t *cube_vertex_p00;
    cube_vertex_t *cube_vertex_0p0;
    cube_vertex_t *cube_vertex_00p;
    int object_vertex[12];
    int edge_set;
    int x, y, z;
    int cube_index;
    int xs, ys, zs;

    static cube_edge_t cube_edge[12] = {
        {0, 0}, {0, 1}, {1, 1}, {2, 0},
        {4, 0}, {4, 1}, {5, 1}, {6, 0},
        {0, 2}, {1, 2}, {2, 2}, {3, 2}
    };

    {
        cube_vertex_t *start = compute_func_value(xb, yb, zb);
        
        queue_insert(xb, yb, zb, start);

        if (queue_items == 0)
            return;
    }

    do {
        queue_remove(&x, &y, &z, &cube_vertex[0][0][0]);
            
        {
            cube_vertex_t **vertex = &cube_vertex[0][0][0];
            int vertices = 8;
            int mask = 1;
            int i = 0;
            
            cube_index = 0;

            do {
                if (i > 0)
                    *vertex = compute_func_value(x + ( i       & 1),
                                                 y + ((i >> 1) & 1),
                                                 z +  (i >> 2)     );
                                                 
                cube_index |= (*vertex)->iso_pos & mask;
                mask <<= 1;
                vertex ++;

            } while (++i != 8);
        }
        
        if (cube_index != 0xff && cube_index != 0) {
            int cube_index_inv = cube_index ^ 0xff;

            edge_set = edge_table[cube_index];

#ifndef MCUBES_USE_STATIC_LISTS

            if (vertices + 12 >= max_vertices) {
                max_vertices *= 2;
                vertex_list = (vector_t *) realloc(vertex_list,
                               sizeof(vector_t) * max_vertices);
                current_vertex = vertex_list + vertices;
            }
            
#endif /* !MCUBES_USE_STATIC_LISTS */

            {
                int edge_mask = 1;
                cube_edge_t *cur_cube_edge = cube_edge;
                int *cur_object_vertex = object_vertex;
                int i = 12;

                do {
                    if (edge_set & edge_mask) {
                        int vertex;
                        int axis = cur_cube_edge->axis;
                        int coord = cur_cube_edge->coord;
                        cube_vertex_t **v0 = &cube_vertex[0][0][0] + coord;
                        int *v = v0[0]->vertex + axis;

                        if ((vertex = *v) < 0) {
                            cube_vertex_t **v1 = v0 + (1 << axis);
                            float value0 = fabs(v0[0]->value);
                            float value1 = fabs(v1[0]->value);
                            vertex = vertices ++;
                            *v = vertex;
                        
                            current_vertex[0][0] = x + ( coord       & 1);
                            current_vertex[0][1] = y + ((coord >> 1) & 1);
                            current_vertex[0][2] = z + ( coord >> 2     );

                            current_vertex[0][axis] += value0 / (value0 + value1);
                        
                            current_vertex ++;
                        }
                        
                        *cur_object_vertex = vertex;
                    }

                    cur_object_vertex ++;
                    cur_cube_edge ++;
                    edge_mask <<= 1;
                    
                } while (--i);
            }

#ifndef MCUBES_USE_STATIC_LISTS

            if (faces + 6 >= max_faces) {
                max_faces *= 2;
                face_list = (face_t *) realloc(face_list, sizeof(face_t) * max_faces);
                current_face = face_list + faces;
            }
            
#endif /* !MCUBES_USE_STATIC_LISTS */

            {
                unsigned char *src = &face_table[cube_index][0];
                int v0;

                while ((v0 = *src) != 0xff) {
                    current_face[0][0] = object_vertex[v0];
                    current_face[0][1] = object_vertex[src[1]];
                    current_face[0][2] = object_vertex[src[2]];
                    current_face ++;
                    faces ++;
                    src += 3;
                }
            }

            cube_vertex_p00 = compute_func_value(x - 1, y    , z    );
            cube_vertex_0p0 = compute_func_value(x    , y - 1, z    );
            cube_vertex_00p = compute_func_value(x    , y    , z - 1);

            if (((0x01 | 0x04 | 0x10 | 0x40) & cube_index) &&
                ((0x01 | 0x04 | 0x10 | 0x40) & cube_index_inv))
                queue_insert(x - 1, y    , z    , cube_vertex_p00);

            if (((0x01 | 0x02 | 0x10 | 0x20) & cube_index) &&
                ((0x01 | 0x02 | 0x10 | 0x20) & cube_index_inv))
                queue_insert(x    , y - 1, z    , cube_vertex_0p0);
                
            if (((0x01 | 0x02 | 0x04 | 0x08) & cube_index) &&
                ((0x01 | 0x02 | 0x04 | 0x08) & cube_index_inv))
                queue_insert(x    , y    , z - 1, cube_vertex_00p);

            if (((0x02 | 0x08 | 0x20 | 0x80) & cube_index) &&
                ((0x02 | 0x08 | 0x20 | 0x80) & cube_index_inv))
                queue_insert(x + 1, y    , z    , cube_vertex[0][0][1]);
                
            if (((0x04 | 0x08 | 0x40 | 0x80) & cube_index) &&
                ((0x04 | 0x08 | 0x40 | 0x80) & cube_index_inv))
                queue_insert(x    , y + 1, z    , cube_vertex[0][1][0]);
            
            if (((0x10 | 0x20 | 0x40 | 0x80) & cube_index) &&
                ((0x10 | 0x20 | 0x40 | 0x80) & cube_index_inv))
                queue_insert(x    , y    , z + 1, cube_vertex[1][0][0]);        
        }
        
    } while (queue_items);
}

