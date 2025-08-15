
#ifndef SHADER_H_
#define SHADER_H_

#include <stdarg.h>
#include <stdbool.h>

#include <cglm/cglm.h>

typedef unsigned int shader_t;
typedef unsigned int program_t;

char *read_glsl(const char *filename);

program_t init_shader(const char *vertexPath, const char *fragmentPath);
void set_shader_bool(program_t program, const char *name, bool value);
void set_shader_int(program_t program, const char *name, int value);
void set_shader_float(program_t program, const char *name, float value);
void set_shader_vec3(program_t program, const char *name, vec3 value);
void set_shader_vec4(program_t program, const char *name, vec4 value);
void set_shader_mat3(program_t program, const char *name, mat3 value);
void set_shader_mat4(program_t program, const char *name, mat4 value);

#endif

