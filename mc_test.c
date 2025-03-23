
#include <GL/glut.h>

#include "mcubes.h"
#include <stdio.h>
#include <math.h>

#ifndef M_PI
#define M_PI 3.141592
#endif

static int vertices;
static int faces;

#ifdef MCUBES_USE_STATIC_LISTS

static vector_t vertex_list[32768];
static vector_t normal_list[32768];
static unsigned short face_list[32768][3];

#else

static vector_t *vertex_list = NULL;
static int max_vertices = 0;

static vector_t *normal_list = NULL;
static int max_normals = 0;

static unsigned short (*face_list)[3] = NULL;
static int max_faces = 0;

#endif /* MCUBES_USE_STATIC_LISTS */

static float alpha = 0.0;
static float beta = 0.0;

static void
create_normals(vector_t *vertex_list, int vertices,
               unsigned short *face, int faces,
               vector_t *normal_list)
{
    vector_t v1v0, v2v0;
    vector_t face_normal;

    if (vertices == 0 || faces == 0)
        return;

    memset(normal_list, 0, sizeof(vector_t) * vertices);
    
    do {
        int iv0 = face[0];
        int iv1 = face[1];
        int iv2 = face[2];
        
        vector_t *v0 = vertex_list + iv0;
        vector_t *v1 = vertex_list + iv1;
        vector_t *v2 = vertex_list + iv2;
        
        face += 3;
    
        vector_sub(*v1, *v0, v1v0);
        vector_sub(*v2, *v0, v2v0);
        vector_cross(v1v0, v2v0, face_normal);
        vector_norm(face_normal);

        vector_inc(normal_list[iv0], face_normal);
        vector_inc(normal_list[iv1], face_normal);
        vector_inc(normal_list[iv2], face_normal);

    } while (--faces);

    do {
        vector_norm(normal_list[0]);
        normal_list ++;

    } while (--vertices);
}

static float sphere_shift = 4.0f;

static float
func0(int x, int y, int z)
{
    z += 16;
    
    return 1.0f / (x * x + y * y + z * z) 
           + 1.0f / (x * x + y * y + (z - 32) * (z - 32)) 
           + 0.25f / (x * x + (y - sphere_shift) * (y - sphere_shift)
                  + (z - 16) * (z - 16))
           - 1.0f / (15.5 * 15.5) 
           - 1.0f / (15.5 * 15.5 + 32 * 32)
           - 0.25f / (15.5 * 15.5 + 16 * 16);
}

/*
int cube_ind;

static float
func0(int x, int y, int z)
{
    if (x >= 0 && x <= 1 &&
        y >= 0 && y <= 1 &&
        z >= 0 && z <= 1)
            return ((1 << ((z << 2) | (y << 1) | x)) & cube_ind) ? 1.0 : -3.0;
    else    return -3.0;
}
*/

static void
idle_func()
{
    sphere_shift = cos(alpha / 580.0 * M_PI * 4.0) * 26.0;
    
#ifdef MCUBES_USE_STATIC_LISTS

    mcubes_begin(vertex_list, &vertices,
                 face_list, &faces,
                 func0);
                 
#else

    mcubes_begin(&vertex_list, &vertices, &max_vertices,
                 &face_list, &faces, &max_faces,
                 func0);

#endif /* MCUBES_USE_STATIC_LISTS */
                 
    mcubes_polygonize(15, 0, -16);
    /* mcubes_polygonize(0, 0, 0); */
    /* mcubes_polygonize(15, 0, 16); */
        
    mcubes_end();

#ifndef MCUBES_USE_STATIC_LISTS

    if (vertices > max_normals) {
        normal_list = (vector_t *) realloc(normal_list, sizeof(vector_t) * vertices);
        max_normals = vertices;
    }

#endif /* !MCUBES_USE_STATIC_LISTS */
        
    create_normals(vertex_list, vertices,
                   face_list, faces,
                   normal_list);

    alpha += 5.0 / 5.0f;
    beta += 4.0 / 5.0f;
    
    glutPostRedisplay();
}

static void
draw_scene()
{
    /* glDisable(GL_LIGHTING); */
        
    glClearColor(0.5, 0.5, 0.5, 0.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    {
        float pos[4] = {-0.9, -0.3, -2.0, 1.0f};
        float amb[4] = {0.0, 0.0, 0.0, 1.0};
        float dif[4] = {1.0, 1.0, 0.0, 1.0};
        float spc[4] = {1.0, 1.0, 1.0, 1.0};

        glLightfv(GL_LIGHT0, GL_POSITION, pos);
        glLightfv(GL_LIGHT0, GL_AMBIENT, amb);
        glLightfv(GL_LIGHT0, GL_DIFFUSE, dif);
        glLightfv(GL_LIGHT0, GL_SPECULAR, spc);
        
        glEnable(GL_LIGHT0);
        
        /*
        glPointSize(6.0);
        glEnable(GL_POINT_SMOOTH);
        glColor3f(dif[0], dif[1], dif[2]);
        glBegin(GL_POINTS);
            glVertex3f(pos[0], pos[1], pos[2]);
        glEnd();
        */
    }
    
#if 1
    
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(0, 0, -40);
    /* glScalef(20.0, 20.0, 20.0); */
    glRotatef(alpha, 0.0, 1.0, 0.0);
    glRotatef(beta, 1.0, 0.0, 0.0);

    {
        float pos[4] = {3.0, -8.0, 23.0, 0.0};
        float amb[4] = {0.0, 0.0, 0.0, 1.0};
        float dif[4] = {0.0, 0.0, 1.0, 1.0};
        float spc[4] = {1.0, 1.0, 1.0, 1.0};

        glLightfv(GL_LIGHT1, GL_POSITION, pos);
        glLightfv(GL_LIGHT1, GL_AMBIENT, amb);
        glLightfv(GL_LIGHT1, GL_DIFFUSE, dif);
        glLightfv(GL_LIGHT1, GL_SPECULAR, spc);
        
        glEnable(GL_LIGHT1);
        
        /*
        glPointSize(6.0);
        glEnable(GL_POINT_SMOOTH);
        glColor3f(dif[0], dif[1], dif[2]);
        glBegin(GL_POINTS);
            glVertex3f(pos[0], pos[1], pos[2]);
        glEnd();
        */
    }

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK); /* default */
    

    /*
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glLineWidth(0.4);
    glEnable(GL_LINE_SMOOTH);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    */

    {
        float amb[4] = {0, 0, 0, 1.0};
        float dif[4] = {0.8, 0.8, 0.8, 1.0};
        float spc[4] = {1, 1, 1, 1.0};
    
        glMaterialfv(GL_FRONT, GL_AMBIENT, amb);
        glMaterialfv(GL_FRONT, GL_DIFFUSE, dif);
        /* lMaterialfv(GL_FRONT, GL_SPECULAR, spc);    */
        /* glMaterialf(GL_FRONT, GL_SHININESS, 40.0f); */
    }

    glEnable(GL_LIGHTING);

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_NORMAL_ARRAY);
    
    glNormalPointer(GL_FLOAT, 0, normal_list);
    glVertexPointer(3, GL_FLOAT, 0, vertex_list);
    
    /*
    glDisable(GL_LIGHTING);

    glColor3f(0.0, 0.0, 1.0);
    glCullFace(GL_FRONT);
    glDrawElements(GL_TRIANGLES, faces * 3, GL_UNSIGNED_SHORT, face_list);
    glColor3f(0.0, 1.0, 0.0);
    glCullFace(GL_BACK);
    glDrawElements(GL_TRIANGLES, faces * 3, GL_UNSIGNED_SHORT, face_list);
    */

    glDrawElements(GL_TRIANGLES, faces * 3, GL_UNSIGNED_SHORT, face_list);
    
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);

    /*
    glBegin(GL_LINE_LOOP);
        glVertex3f(0.0, 0.0, 0.0);
        glVertex3f(0.0, 1.0, 0.0);
        glVertex3f(1.0, 1.0, 0.0);
        glVertex3f(1.0, 0.0, 0.0);
    glEnd();
    
    glBegin(GL_LINE_LOOP);
        glVertex3f(0.0, 0.0, 1.0);
        glVertex3f(0.0, 1.0, 1.0);
        glVertex3f(1.0, 1.0, 1.0);
        glVertex3f(1.0, 0.0, 1.0);
    glEnd();
    */
    
#endif

    glFlush();
    
    glutSwapBuffers();
}

static void
resize(int w, int h)
{
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glFrustum(-1.0, 1.0, -1.0, 1.0, 1.0, 10000.0);
}

int
main()
{
    /*
    printf("cube = ");
    scanf("%d", &cube_ind);
    */
    
    mcubes_init();

    glutInitWindowPosition(0, 0);
    glutInitWindowSize(400, 400);
    glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
    glutCreateWindow("Marching Cubes Test");

    glEnable(GL_DEPTH_TEST);
    
    glutReshapeFunc(resize);
    glutIdleFunc(idle_func);
    glutDisplayFunc(draw_scene);
    glutMainLoop();

    mcubes_done();

    return 0;
}
