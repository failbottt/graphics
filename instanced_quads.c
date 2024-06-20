#include "glad.h"
#include <GL/gl.h>
#include <GLFW/glfw3.h>
#include <stdio.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#define HANDMADE_MATH_IMPLEMENTATION
#include "handmade_math.h"

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void process_input(GLFWwindow *window);
void draw_text(float xpos, float ypos, float width, float height, char *text);

int screen_width = 1920;
int screen_height = 1080;
unsigned int shaderProgram;
unsigned int VBO, VAO;
unsigned int font_texture_atlas;

const char *vs = "#version 460 core\n"
"layout (location = 0) in vec2 v_pos_pattern;\n"
"layout (location = 1) in vec4 v_quad;\n"
"uniform vec4 projection;\n"
"void main() {\n"
"vec2 center = (v_quad.xy + v_quad.zw)*0.5;\n"
"vec2 half_dim = (v_quad.xy - v_quad.zw)*0.5;\n"
"vec2 pos = center + half_dim*v_pos_pattern;\n"
"gl_Position = vec4(pos, 0.0, 1.0);\n"
"}\n"
;

const char *fs = "#version 460 core\n"
"out vec4 FragColor;\n"
"\n"
"void main()\n"
"{ \n"
"    FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);\n"
"}";

int main(void)
{
    GLFWwindow* window;

    /* Initialize the library */
    if (!glfwInit())
        return -1;

    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(screen_width, screen_height, "Hello World", NULL, NULL);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
    if (!window)
    {
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // NOTE: you have to have a GL context prior to calling this
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        printf("Failed to initialize GLAD");
        return -1;
    }

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    

    // compile shaders
    // build and compile our shader program
    // ------------------------------------
    // vertex shader
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vs, NULL);
    glCompileShader(vertexShader);
    // check for shader compile errors
    int success;
    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        printf("ERROR::SHADER::VERTEX::COMPILATION_FAILED: %s\n", infoLog);
    }
    // fragment shader
    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fs, NULL);
    glCompileShader(fragmentShader);
    // check for shader compile errors
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        printf("ERROR::SHADER::FRAGMENT::COMPILATION_FAILED: %s\n", infoLog);
    }
    // link shaders
    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    // check for linking errors
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        printf("ERROR::SHADER::PROGRAM::LINKING_FAILED %s\n", infoLog);
    }
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

     // set up vertex data (and buffer(s)) and configure vertex attributes
    // ------------------------------------------------------------------
    	
    /* float vertices[] = { */
    /*     // first triangle */
    /*      0.5f,  0.5f, 0.0f,  // top right */
    /*      0.5f, -0.5f, 0.0f,  // bottom right */
    /*     -0.5f,  0.5f, 0.0f,  // top left */ 
    /*     // second triangle */
    /*      0.5f, -0.5f, 0.0f,  // bottom right */
    /*     -0.5f, -0.5f, 0.0f,  // bottom left */
    /*     -0.5f,  0.5f, 0.0f   // top left */
    /* }; */


    float vertices[] = {
        // top triangle
        -1.f, +1.f, // top left
        +1.0f, +1.f, // top right
        -1.f, -1.f, // bottom left

        // bottom triangle
        +1.f, +1.f, //top right
        -1.f, -1.f, // bottom left 
        +1.f, -1.f, // bottom right

        // [12] four quad specifiers
        // top left -- bottom right
        //
        // tlx, tly -- blx, bly
        -1.0f, -1.0, 250.f/screen_width, 60.f/screen_height,
        -0.7f, 0.5f, +0.8f, +0.8f,
        -0.7f, -0.3f, -0.3f, -0.7f,
        +0.5f, +0.5f, +0.9f, +0.1f,
        +0.5f, -0.1f, +0.9f, -0.5f,
    };
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STREAM_DRAW);

    // position attribute
    glEnableVertexAttribArray(0);
    glVertexAttribDivisor(0, 0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);

    glEnableVertexAttribArray(1);
    glVertexAttribDivisor(1, 1);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 4*sizeof(float), (void*)(12 * sizeof(float)));
    // color attribute

    glBindVertexArray(0);

    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window))
    {
        process_input(window);
         // render
        // ------
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

        glUseProgram(shaderProgram);
        glBindVertexArray(VAO);
        /* glPointSize(32); */
        /* glDrawArraysInstanced(GL_POINTS, 0, 6, 1); */
        glDrawArraysInstanced(GL_TRIANGLES, 0, 6, 5);

        /* Swap front and back buffers */
        glfwSwapBuffers(window);
        /* Poll for and process events */
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void process_input(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, 1);
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}

