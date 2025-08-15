
#ifndef FLUID_H_
#define FLUID_H_

#include <stdlib.h>

#define IX(i, j, N) ((i) + (N + 2) * (j))

static inline int clampi(int v, int lo, int hi)
{
    return v < lo ? lo : (v > hi ? hi : v);
}

typedef struct
{
    int N;
    float dt;
    float diff;
    float visc;
    float *u, *v;
    float *u_prev, *v_prev;
    float *dens, *dens_prev;
} fluid_t;

fluid_t *create_fluid(int N, float dt, float diff, float visc);
void free_fluid(fluid_t *f);

void fluid_add_density(fluid_t *f, int x, int y, float amount);
void fluid_add_velocity(fluid_t *f, int x, int y, float amountX, float amountY);

void vel_step(int N, float *u, float *v, float *u0, float *v0, float visc, float dt);
void dens_step(int N, float *x, float *x0, float *u, float *v, float diff, float dt);

static inline void fluid_step(fluid_t *f)
{
    vel_step(f->N, f->u, f->v, f->u_prev, f->v_prev, f->visc, f->dt);
    dens_step(f->N, f->dens, f->dens_prev, f->u, f->v, f->diff, f->dt);

    int size = (f->N+2) * (f->N+2);
    memset(f->u_prev, 0, size * sizeof(float));
    memset(f->v_prev, 0, size * sizeof(float));
    memset(f->dens_prev, 0, size * sizeof(float));
}

#endif

