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
unsigned int VBO, VAO, EBO;
unsigned int font_texture_atlas;

typedef struct {
    int x;
    int y;
} Char;

Char font_idx[] = {
    ['A'] = {.x = 0, .y = 0},
    ['B'] = {.x = 1, .y = 0},
    ['C'] = {.x = 2, .y = 0},
    ['D'] = {.x = 3, .y = 0},
    ['E'] = {.x = 4, .y = 0},
    ['F'] = {.x = 5, .y = 0},
    ['G'] = {.x = 6, .y = 0},
    ['H'] = {.x = 7, .y = 0},
    ['I'] = {.x = 8, .y = 0},
    ['J'] = {.x = 9, .y = 0},
    ['K'] = {.x = 10, .y = 0},
    ['L'] = {.x = 11, .y = 0},
    ['M'] = {.x = 12, .y = 0},
    ['N'] = {.x = 0, .y = 1},
    ['O'] = {.x = 1, .y = 1},
    ['P'] = {.x = 2, .y = 1},
    ['Q'] = {.x = 3, .y = 1},
    ['R'] = {.x = 4, .y = 1},
    ['S'] = {.x = 5, .y = 1},
    ['T'] = {.x = 6, .y = 1},
    ['U'] = {.x = 7, .y = 1},
    ['V'] = {.x = 8, .y = 1},
    ['W'] = {.x = 8, .y = 1},
    ['X'] = {.x = 10, .y = 1},
    ['Y'] = {.x = 11, .y = 1},
    ['Z'] = {.x = 12, .y = 1},
    ['a'] = {.x = 0, .y = 2},
    ['b'] = {.x = 1, .y = 2},
    ['c'] = {.x = 2, .y = 2},
    ['d'] = {.x = 3, .y = 2},
    ['e'] = {.x = 4, .y = 2},
    ['f'] = {.x = 5, .y = 2},
    ['g'] = {.x = 6, .y = 2},
    ['h'] = {.x = 7, .y = 2},
    ['i'] = {.x = 8, .y = 2},
    ['j'] = {.x = 9, .y = 2},
    ['k'] = {.x = 10, .y = 2},
    ['l'] = {.x = 11, .y = 2},
    ['m'] = {.x = 12, .y = 2},
    ['n'] = {.x = 0, .y = 3},
    ['o'] = {.x = 1, .y = 3},
    ['p'] = {.x = 2, .y = 3},
    ['q'] = {.x = 3, .y = 3},
    ['r'] = {.x = 4, .y = 3},
    ['s'] = {.x = 5, .y = 3},
    ['t'] = {.x = 6, .y = 3},
    ['u'] = {.x = 7, .y = 3},
    ['v'] = {.x = 8, .y = 3},
    ['w'] = {.x = 9, .y = 3},
    ['x'] = {.x = 10, .y = 3},
    ['y'] = {.x = 11, .y = 3},
    ['z'] = {.x = 12, .y = 3},
    ['0'] = {.x = 0, .y = 4},
    ['1'] = {.x = 1, .y = 4},
    ['2'] = {.x = 2, .y = 4},
    ['3'] = {.x = 3, .y = 4},
    ['4'] = {.x = 4, .y = 4},
    ['5'] = {.x = 5, .y = 4},
    ['6'] = {.x = 6, .y = 4},
    ['7'] = {.x = 7, .y = 4},
    ['8'] = {.x = 8, .y = 4},
    ['9'] = {.x = 9, .y = 4},
    ['+'] = {.x = 10, .y = 4},
    ['-'] = {.x = 11, .y = 4},
    ['='] = {.x = 12, .y = 4},
    ['('] = {.x = 0, .y = 5},
    [')'] = {.x = 1, .y = 5},
    ['['] = {.x = 2, .y = 5},
    [']'] = {.x = 3, .y = 5},
    ['{'] = {.x = 4, .y = 5},
    ['}'] = {.x = 5, .y = 5},
    ['<'] = {.x = 6, .y = 5},
    ['>'] = {.x = 7, .y = 5},
    ['/'] = {.x = 8, .y = 5},
    ['*'] = {.x = 9, .y = 5},
    [':'] = {.x = 10, .y = 5},
    ['#'] = {.x = 11, .y = 5},
    ['%'] = {.x = 12, .y = 5},
    ['!'] = {.x = 0, .y = 6},
    ['?'] = {.x = 1, .y = 6},
    ['.'] = {.x = 2, .y = 6},
    [','] = {.x = 3, .y = 6},
    ['\''] = {.x = 4, .y = 6},
    ['"'] = {.x = 5, .y = 6},
    ['@'] = {.x = 6, .y = 6},
    ['&'] = {.x = 7, .y = 6},
    ['$'] = {.x = 8, .y = 6},
    [' '] = {.x = 9, .y = 6},
    // not in monogram 6x10
    /* [';'] = {.x = 0, .y = 0}, */
    /* ['^'] = {.x = 0, .y = 0}, */
    /* ['_'] = {.x = 0, .y = 0}, */
    /* ['`'] = {.x = 0, .y = 0}, */
    /* ['|'] = {.x = 0, .y = 0}, */
    /* ['~'] = {.x = 0, .y = 0}, */
    /* ['\\'] = {.x = 10, .y = 0}, */
};

const char *vs = "#version 460 core\n"
"layout (location = 0) in vec3 a_Position;//WorldSpace\n"
/* "layout (location = 1) in vec3 a_Color;\n" */
"layout (location = 2) in vec2 a_TexCoord;\n"
"\n"
"uniform mat4 projection;\n"
"\n"
"out vec4 v_Color;\n"
"out vec2 v_TexCoord;\n"
"\n"
"void main()\n"
"{\n"
"    gl_Position = projection * vec4(a_Position, 1.0f);    \n"
"    v_Color = vec4(1, 1, 1, 1);\n"
"    v_TexCoord = a_TexCoord;\n"
"}";

const char *fs = "#version 460 core\n"
"in vec4 v_Color;\n"
"in vec2 v_TexCoord;\n"
"\n"
"uniform sampler2D u_Sampler;\n"
"\n"
"out vec4 v_FragColor;\n"
"\n"
"void main()\n"
"{ \n"
"    vec4 tc = v_Color * texture(u_Sampler, v_TexCoord);\n"
"    v_FragColor = tc;\n"
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
    float vertices[] = {};
    unsigned int indices[] = {  // note that we start from 0!
        0, 1, 3,  // first Triangle
        1, 2, 3   // second Triangle
    };
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);
    // bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // color attribute
    /* glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float))); */
    /* glEnableVertexAttribArray(1); */
    // texture coord attribute
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(2);
    // remember: do NOT unbind the EBO while a VAO is active as the bound element buffer object IS stored in the VAO; keep the EBO bound.
    //glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    
    

    glGenTextures(1, &font_texture_atlas);
    glBindTexture(GL_TEXTURE_2D, font_texture_atlas); 
     // set the texture wrapping parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);	// set texture wrapping to GL_REPEAT (default wrapping method)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    // set texture filtering parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    // load image, create texture and generate mipmaps
    int width, height, nrChannels;
    /* stbi_set_flip_vertically_on_load(1); // tell stb_image.h to flip loaded texture's on the y-axis. */
    unsigned char *data = stbi_load("./third_party/fonts/minogram_6x10.png", &width, &height, &nrChannels, 4);
    if (data)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        printf("Failed to load texture\n");
        return -1;
    }
    stbi_write_png("out.png", width, height, 1, data, 3);
    stbi_image_free(data);
    
    glUseProgram(shaderProgram);
    glUniform1i(glGetUniformLocation(shaderProgram, "font_texture_atlas"), 0);

    hmm_mat4 ortho = HMM_Orthographic(0.f, screen_width, 0.f, screen_height, -1.f, 1.f);
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, (GLfloat*)ortho.Elements);

    // You can unbind the VAO afterwards so other VAO calls won't accidentally modify this VAO, but this rarely happens. Modifying other
    // VAOs requires a call to glBindVertexArray anyways so we generally don't unbind VAOs (nor VBOs) when it's not directly necessary.
    glBindVertexArray(0);

    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window))
    {
        process_input(window);
         // render
        // ------
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_COLOR_BUFFER_BIT);


        draw_text(20.5f, 500.5f, 18.f, 18.f, "This is a test! Hello, world.");
        draw_text(20.5f, 400.5f, 18.f, 18.f, "This is a test! Hello, world.");
        draw_text(20.5f, 300.5f, 18.f, 18.f, "This is a test! Hello, world.");
        draw_text(20.5f, 100.5f, 18.f, 18.f, "This is a test! Hello, world.");

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

void draw_text(float xpos, float ypos, float width, float height, char *text) {
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, font_texture_atlas);
    glUseProgram(shaderProgram);
    glBindVertexArray(VAO);

    for (size_t i = 0; i < strlen(text); i++) {
        float x = font_idx[(int)text[i]].x; 
        float y = font_idx[(int)text[i]].y;
        float sheet_width = 70.f;
        float sheet_height = 78.f;
        float sprite_height = 11.14285714f;
        float sprite_width = 5.384615385f;

        float brx = (x * sprite_width) / sheet_width;
        float bry = (y * sprite_height) / sheet_height;
        float blx = ((x+1) * sprite_width) / sheet_width;
        float bly = (y * sprite_height) / sheet_height;
        float trx = (x * sprite_width) / sheet_width;
        float try = ((y+1) * sprite_height) / sheet_height;
        float tlx = ((x+1) * sprite_width) / sheet_width;
        float tly = ((y+1) * sprite_height) / sheet_height;

        float sprite_verts[4*5] = {
             // positions                    // texture coords
             (xpos-width)+(i*18.f),  ypos-height, 0.0f, trx, try, // top right
             (xpos-width)+(i*18.f),  ypos, 0.0f, brx, bry, // bottom right
             xpos+(i*18.f),  ypos, 0.0f, blx, bly, // bottom left
             xpos+(i*18.f),  ypos-height, 0.0f, tlx, tly  // top left 
        };

        glBufferData(GL_ARRAY_BUFFER, sizeof(sprite_verts), sprite_verts, GL_STATIC_DRAW);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    }
}

