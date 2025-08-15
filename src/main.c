
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <cglm/cglm.h>
#include <cglm/struct.h>

#include "include/shader.h"
#include "include/fluid.h"


float quadVertices[] = {
    -1.0f,  1.0f, 0.0f, 1.0f,
    -1.0f, -1.0f, 0.0f, 0.0f,
     1.0f, -1.0f, 1.0f, 0.0f,

    -1.0f,  1.0f, 0.0f, 1.0f,
     1.0f, -1.0f, 1.0f, 0.0f,
     1.0f,  1.0f, 1.0f, 1.0f,
};

float arrowVertices[] = {
    0.0f, 0.0f,
    1.0f, 0.0f,
    1.0f, 1.0f,
    0.0f, 1.0f
};

fluid_t *f;

int N = 256;
volatile int fb_width = 800, fb_height = 600;
volatile int win_width = 800, win_height = 600; 
volatile double mx = 0, my = 0;
volatile double prev_mouse_x = 0, prev_mouse_y = 0;

void cursor_position_callback(GLFWwindow *window, double xpos, double ypos)
{
    prev_mouse_x = mx;
    prev_mouse_y = my;

    mx = xpos;
    my = ypos;
}

void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
    fb_width = width;
    fb_height = height;
    glViewport(0, 0, width, height);
}

void window_size_callback(GLFWwindow *window, int width, int height)
{
    win_width = width;
    win_height = height;
}

void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    float fx = (mx / (double)win_width) * (f->N);
    float fy = ((my / (double)win_height)) * (f->N);

    int ix = clampi((int)(fx + 0.5f), 1, f->N);
    int iy = clampi((int)(fy + 0.5f), 1, f->N);

    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
        fluid_add_density(f, ix, iy, 50.0f);

    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS)
    {
        float vx = (float)(mx - prev_mouse_x) * (f->N / (float)win_width);
        float vy = (float)(prev_mouse_y - my) * (f->N / (float)win_height);
        fluid_add_velocity(f, ix, iy, vx, -vy);
    }

    prev_mouse_x = mx;
    prev_mouse_y = my;
}

int main()
{
    if (!glfwInit())
    {
        fprintf(stderr, "Failed to initialize GLFW\n");
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    GLFWwindow *window = glfwCreateWindow(win_width, win_height, "OpenGL C App", NULL, NULL);
    if (!window)
    {
        fprintf(stderr, "Failed to create GLFW window\n");
        return -1;
    }

    glfwMakeContextCurrent(window);

    GLenum err = glewInit();
    if (err != GLEW_OK)
    {
        fprintf(stderr, "GLEW Error: %s\n", glewGetErrorString(err));
        return -1;
    }

    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetWindowSizeCallback(window, window_size_callback);
    glfwSetCursorPosCallback(window, cursor_position_callback);

    f = create_fluid(N, 0.1f, 0.0001f, 0.000001f);
    float *texData = malloc(N*N*3*sizeof(float));

    program_t fluidShader = init_shader("shaders/vertex/quad.vert", "shaders/fragment/quad.frag");
    program_t arrowShader = init_shader("shaders/vertex/arrow.vert", "shaders/fragment/arrow.frag");

    unsigned int quadVAO, quadVBO;
    glGenVertexArrays(1, &quadVAO);
    glGenBuffers(1, &quadVBO);

    glBindVertexArray(quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);

    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);


    vec2s cellCenters[N * N], velocities[N * N];
    for (int j = 0; j < N; ++j)
    {
        for (int i = 0; i < N; ++i)
        {
            int idx = j * N + i;
            cellCenters[idx].x = (float)i;
            cellCenters[idx].y = (float)j;

            float dx = i - N / 2;
            float dy = j - N / 2;
            velocities[idx].x = -dy / 20.0f;
            velocities[idx].y = dx / 20.0f;
        }
    }

    unsigned int arrowVAO, VBOs[3];
    glGenVertexArrays(1, &arrowVAO);
    glBindVertexArray(arrowVAO);
    glGenBuffers(3, VBOs);

    glBindBuffer(GL_ARRAY_BUFFER, VBOs[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(arrowVertices), arrowVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, VBOs[1]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cellCenters), cellCenters, GL_STATIC_DRAW);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(vec2s), (void *)0);
    glEnableVertexAttribArray(1);
    glVertexAttribDivisor(1, 1);

    glBindBuffer(GL_ARRAY_BUFFER, VBOs[2]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(velocities), velocities, GL_STATIC_DRAW);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(vec2s), (void *)0);
    glEnableVertexAttribArray(2);
    glVertexAttribDivisor(2, 1);

    mat4 proj;
    glm_ortho(0.0f, N, 0.0f, N, -1.0f, 1.0f, proj);

    glUseProgram(arrowShader);
    set_shader_mat4(arrowShader, "projection", proj);
    set_shader_float(arrowShader, "arrowScale", 30.f);

    unsigned int tex;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, N, N, 0, GL_RGB, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    while (!glfwWindowShouldClose(window))
    {
        processInput(window);
        glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        fluid_step(f);

        for (int y = 0; y < N; ++y)
        {
            for (int x = 0; x < N; ++x)
            {
                int src = IX(x+1, y+1, N);
                int dst = (N-1 - y) * N + x;
                texData[dst*3 + 0] = f->u[src];
                texData[dst*3 + 1] = f->v[src];
                texData[dst*3 + 2] = f->dens[src];

                velocities[dst].x = f->u[src];
                velocities[dst].y = f->v[src];
            }
        }

        glBindBuffer(GL_ARRAY_BUFFER, VBOs[2]);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(velocities), velocities);

        glBindTexture(GL_TEXTURE_2D, tex);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, N, N, 0, GL_RGB, GL_FLOAT, texData);

        glUseProgram(fluidShader);
        glBindVertexArray(quadVAO);
        glBindTexture(GL_TEXTURE_2D, tex);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        //glUseProgram(arrowShader);
        //glBindVertexArray(arrowVAO);
        //glDrawArraysInstanced(GL_TRIANGLES, 0, 4, N * N);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &quadVAO);
    glDeleteVertexArrays(1, &arrowVAO);
    glDeleteBuffers(1, &quadVBO);
    glDeleteBuffers(3, VBOs);
    glDeleteProgram(fluidShader);
    glDeleteProgram(arrowShader);
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}

