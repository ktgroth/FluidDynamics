
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "include/fluid.h"

#define SWAP(x0, x) { float *tmp = x0; x0 = x; x = tmp; }

static const int SOLVER_ITERS = 20;

static void add_source(int N, float *x, float *s, float dt);
static void set_bnd(int N, int b, float *x);
static void lin_solve(int N, float *x, float *x0, float a, float c);
static void diffuse(int N, int b, float *x, float *x0, float diff, float dt);
static void advect(int N, int b, float *d, float *d0, float *u, float *v, float dt);
static void project(int N, float *u, float *v, float *p, float *div);


fluid_t *create_fluid(int N, float dt, float diff, float visc)
{
    fluid_t *f = (fluid_t *)calloc(1, sizeof(fluid_t));
    if (!f)
    {
        perror("create_fluid");
        exit(EXIT_FAILURE);
    }

    f->N = N;
    f->dt = dt;
    f->diff = diff;
    f->visc = visc;


    int size = (N+2) * (N+2);
    f->u            = (float *)calloc(size, sizeof(float));
    f->v            = (float *)calloc(size, sizeof(float));
    f->u_prev       = (float *)calloc(size, sizeof(float));
    f->v_prev       = (float *)calloc(size, sizeof(float));
    f->dens         = (float *)calloc(size, sizeof(float));
    f->dens_prev    = (float *)calloc(size, sizeof(float));

    return f;
}

void free_fluid(fluid_t *f)
{
    free(f->u);
    free(f->v);
    free(f->u_prev);
    free(f->v_prev);
    free(f->dens);
    free(f->dens_prev);
    free(f);
}

void fluid_add_density(fluid_t *f, int x, int y, float amount)
{
    int N = f->N;
    f->dens[IX(x, y, N)] += amount;
}

void fluid_add_velocity(fluid_t *f, int x, int y, float amountX, float amountY)
{
    int N = f->N;
    f->u[IX(x, y, N)] += amountX;
    f->v[IX(x, y, N)] += amountY;
}


void dens_step(int N, float *x, float *x0, float *u, float *v, float diff, float dt)
{
    add_source(N, x, x0, dt);
    SWAP(x0, x);
    diffuse(N, 0, x, x0, diff, dt);

    SWAP(x0, x);
    advect(N, 0, x, x0, u, v, dt);
}

void vel_step(int N, float *u, float *v, float *u0, float *v0, float visc, float dt)
{
    add_source(N, u, u0, dt);
    add_source(N, v, v0, dt);

    SWAP(u0, u);
    diffuse(N, 1, u, u0, visc, dt);
    SWAP(v0, v);
    diffuse(N, 2, v, v0, visc, dt);
    project(N, u, v, u0, v0);

    SWAP(u0, u);
    SWAP(v0, v);
    advect(N, 1, u, u0, u0, v0, dt);
    advect(N, 2, v, v0, u0, v0, dt);
    project(N, u, v, u0, v0);
}

static void add_source(int N, float *x, float *s, float dt)
{
    int i, size = (N+2) * (N+2);
    for (i = 0; i < size; ++i)
        x[i] += dt * s[i];
}

static void set_bnd(int N, int b, float *x)
{
    int i;
    for (i = 1; i <= N; ++i)
    {
        x[IX(0,   i, N)] = b == 1 ? -x[IX(1, i, N)] : x[IX(1, i, N)];
        x[IX(N+1, i, N)] = b == 1 ? -x[IX(N, i, N)] : x[IX(N, i, N)];
        x[IX(i,   0, N)] = b == 2 ? -x[IX(i, 1, N)] : x[IX(i, 1, N)];
        x[IX(i, N+1, N)] = b == 2 ? -x[IX(i, N, N)] : x[IX(i, N, N)];
    }

    x[IX(0,     0, N)] = 0.5 * (x[IX(1,   0, N)] + x[IX(0,   1, N)]);
    x[IX(0,   N+1, N)] = 0.5 * (x[IX(1, N+1, N)] + x[IX(0,   N, N)]);
    x[IX(N+1,   0, N)] = 0.5 * (x[IX(N,   0, N)] + x[IX(N+1, 1, N)]);
    x[IX(N+1, N+1, N)] = 0.5 * (x[IX(N, N+1, N)] + x[IX(N+1, N, N)]);
}

static void lin_solve(int N, float *x, float *x0, float a, float c)
{
    int i, j;
    for (i = 1; i <= N; ++i)
        for (j = 1; j <= N; ++j)
            x[IX(i, j, N)] = (x0[IX(i, j, N)] + a * (
                x[IX(i-1, j, N)] + x[IX(i+1, j, N)] +
                x[IX(i, j-1, N)] + x[IX(i, j+1, N)]
            )) / c;
}

static void diffuse(int N, int b, float *x, float *x0, float diff, float dt)
{
    int k;
    float a = dt * diff * N * N;
    for (k = 0; k < SOLVER_ITERS; ++k)
    {
        lin_solve(N, x, x0, a, 1.0f + 4.0f * a);
        set_bnd(N, b, x);
    }
}

static void advect(int N, int b, float *d, float *d0, float *u, float *v, float dt)
{
    int i, j, i0, j0, i1, j1;
    float x, y, s0, t0, s1, t1, dt0;

    dt0 = dt * N;
    for (i = 1; i <= N; ++i)
    {
        for (j = 1; j <= N; ++j)
        {
            x = i - dt0 * u[IX(i, j, N)];
            y = j - dt0 * v[IX(i, j, N)];

            if (x < 0.5)
                x = 0.5;
            if (y < 0.5)
                y = 0.5;
            if (x > N + 0.5)
                x = N + 0.5;
            if (y > N + 0.5)
                y = N + 0.5;

            i0 = (int)x;
            i1 = i0 + 1;

            j0 = (int)y;
            j1 = j0 + 1;

            s1 = x - i0;
            s0 = 1 - s1;

            t1 = y - j0;
            t0 = 1 - t1;

            d[IX(i, j, N)] = s0 * (t0 * d0[IX(i0, j0, N)] + t1 * d0[IX(i0, j1, N)]) +
                             s1 * (t0 * d0[IX(i1, j0, N)] + t1 * d0[IX(i1, j1, N)]);
        }
    }

    set_bnd(N, b, d);
}

static void project(int N, float *u, float *v, float *p, float *div)
{
    int i, j, k;
    float h;

    h = 1.0/N;
    for (i = 1; i <= N; ++i)
    {
        for (j = 1; j <= N; ++j)
        {
            div[IX(i, j, N)] = -0.5 * h * (u[IX(i+1, j, N)] - u[IX(i-1, j, N)] +
                                           v[IX(i, j+1, N)] - v[IX(i, j-1, N)]);
            p[IX(i, j, N)] = 0;
        }
    }

    set_bnd(N, 0, div);
    set_bnd(N, 0, p);

    for (k = 0; k < SOLVER_ITERS; ++k)
    {
        lin_solve(N, p, div, 1.0f, 4.0f);
        set_bnd(N, 0, p);
    }

    for (i = 1; i <= N; ++i)
    {
        for (j = 1; j <= N; ++j)
        {
            u[IX(i, j, N)] -= 0.5 * (p[IX(i+1, j, N)] - p[IX(i-1, j, N)]) / h;
            v[IX(i, j, N)] -= 0.5 * (p[IX(i, j+1, N)] - p[IX(i, j-1, N)]) / h;
        }
    }

    set_bnd(N, 1, u);
    set_bnd(N, 2, v);
}

