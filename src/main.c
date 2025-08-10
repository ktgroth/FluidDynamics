
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#include <fontconfig/fontconfig.h>

void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
    printf("%d %d\n", width, height);
    glViewport(0, 0, width, height);
}

char *read_glsl(char *filename)
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

unsigned int compile_shader(const char **source, size_t n, const GLint *lengths, int type)
{
    unsigned int shader = glCreateShader(type);

    glShaderSource(shader, n, source, lengths);
    glCompileShader(shader);

    int success;
    char infoLog[512];
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

    if (!success)
    {
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        fprintf(stderr, "[ERROR SHADER COMPILATION] %s\n", infoLog);
        return -1;
    }

    return shader;
}

int link_program(unsigned int program, size_t n, ...)
{
    va_list args;
    va_start(args, n);

    for (size_t i = 0; i < n; ++i)
    {
        unsigned int shader = va_arg(args, unsigned int);
        glAttachShader(program, shader);
    }

    glLinkProgram(program);

    int success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success)
        return 0;
    va_end(args);

    va_start(args, n);

    for (size_t i = 0; i < n; ++i)
    {
        unsigned int shader = va_arg(args, unsigned int);
        glDeleteShader(shader);
    }
    va_end(args);

    return 1;
}

void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

int main()
{
    FcInit();
    if (!glfwInit())
    {
        fprintf(stderr, "Failed to initialize GLFW\n");
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    GLFWwindow *window = glfwCreateWindow(800, 600, "OpenGL C App", NULL, NULL);
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

    const char *vertexSource = read_glsl("shaders/vertex/vert_test.vert");
    if (!vertexSource)
    {
        fprintf(stderr, "[ERROR READING VERTEX GLSL]");
        return -1;
    }

    printf("%s", vertexSource);
    unsigned int vertexShader = compile_shader(&vertexSource, 1, NULL, GL_VERTEX_SHADER);

    const char *fragmentSource = read_glsl("shaders/fragment/frag_test.frag");
    if (!fragmentSource)
    {
        fprintf(stderr, "[ERROR READING FRAGMENT GLSL]");
        return -1;
    }

    printf("%s", fragmentSource);
    unsigned int fragmentShader = compile_shader(&fragmentSource, 1, NULL, GL_FRAGMENT_SHADER);

    unsigned int shaderProgram = glCreateProgram();

    int success;
    char infoLog[512];

    success = link_program(shaderProgram, 2, vertexShader, fragmentShader);
    if (!success)
    {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        fprintf(stderr, "[ERROR PROGRAM LINKING] %s\n", infoLog);
        return -1;
    }

    float vertices[] = {
         0.5f, -0.5f,  0.0f, 1.0f, 0.0f, 0.0f,
        -0.5f, -0.5f,  0.0f, 0.0f, 1.0f, 0.0f,
         0.0f,  0.5f,  0.0f, 0.0f, 0.0f, 1.0f,
    };

    unsigned int VAO, VBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    while (!glfwWindowShouldClose(window))
    {
        processInput(window);
        glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(shaderProgram);

        float timeValue = glfwGetTime();
        float greenValue = (sin(timeValue) / 2.0f) + 0.5f;
        int vertexColorLocation = glGetUniformLocation(shaderProgram, "ourColor");
        glUniform4f(vertexColorLocation, 0.0f, greenValue, 0.0f, 1.0f);

        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 3);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    FcFini();

    return 0;
}

