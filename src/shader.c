
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "include/shader.h"

shader_t compile_shader(const char *path, int type);
int link_program(program_t program, shader_t vertex, shader_t fragment);

char *read_glsl(const char *filename)
{
    FILE * fp = fopen(filename, "r");
    if (!fp)
    {
        fprintf(stderr, "fopen(%s, \"r\"): ", filename);
        perror("");
        return NULL;
    }

    const char *template = "// %s\n";

    size_t source_len = snprintf(NULL, 0, template, filename);
    char *source = malloc(source_len);
    if (!source)
    {
        fclose(fp);
        return NULL;
    }

    sprintf(source, template, filename);

    size_t n = 0;
    ssize_t line_len;
    char *line = NULL;
    while ((line_len = getline(&line, &n, fp)) != -1)
    {
        char *tmp = (char *)realloc(source, (source_len + line_len + 1) * sizeof(char));

        if (!tmp)
        {
            free(source);
            free(line);
            fclose(fp);
            return NULL;
        }

        source = tmp;
        memcpy(source + source_len, line, line_len);
        source_len += line_len;
        source[source_len] = '\0';
    }

    free(line);
    fclose(fp);

    return source;
}

shader_t init_shader(const char *vertexPath, const char *fragmentPath)
{
    shader_t vertex = compile_shader(vertexPath, GL_VERTEX_SHADER);
    shader_t fragment = compile_shader(fragmentPath, GL_FRAGMENT_SHADER);
    program_t program = glCreateProgram();

    int success;
    success = link_program(program, vertex, fragment);
    if (!success)
    {
        char infoLog[512];
        glGetProgramInfoLog(program, 512, NULL, infoLog);
        fprintf(stderr, "[ERROR PROGRAM LINKING] %s\n", infoLog);
        exit(EXIT_FAILURE);
    }

    return program;
}

shader_t compile_shader(const char *path, int type)
{
    char *source = read_glsl(path);
    shader_t shader = glCreateShader(type);
    glShaderSource(shader, 1, (const char **)&source, NULL);
    glCompileShader(shader);

    int success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

    if (!success)
    {
        char infoLog[512];
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        fprintf(stderr, "[ERROR SHADER COMPILATION] %s\n%s", infoLog, source);
        exit(EXIT_FAILURE);
    }

    free(source);
    return shader;
}

void set_shader_bool(program_t program, const char *name, bool value)
{
    glUniform1i(glGetUniformLocation(program, name), value);
}

void set_shader_int(program_t program, const char *name, int value)
{
    glUniform1i(glGetUniformLocation(program, name), value);
}

void set_shader_float(program_t program, const char *name, float value)
{
    glUniform1f(glGetUniformLocation(program, name), value);
}

void set_shader_vec3(program_t program, const char *name, vec3 value)
{
    glUniform3fv(glGetUniformLocation(program, name), 1, value);
}

void set_shader_vec4(program_t program, const char *name, vec4 value)
{
    glUniform4fv(glGetUniformLocation(program, name), 1, value);
}

void set_shader_mat3(program_t program, const char *name, mat3 value)
{
    glUniformMatrix3fv(glGetUniformLocation(program, name), 1, GL_FALSE, (float *)value);
}

void set_shader_mat4(program_t program, const char *name, mat4 value)
{
    glUniformMatrix4fv(glGetUniformLocation(program, name), 1, GL_FALSE, (float *)value);
}

int link_program(program_t program, shader_t vertex, shader_t fragment)
{
    glAttachShader(program, vertex);
    glAttachShader(program, fragment);
    glLinkProgram(program);

    int success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success)
        return 0;

    glDeleteShader(vertex);
    glDeleteShader(fragment);

    return 1;
}

