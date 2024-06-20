#include "glad.h"
#include <GL/gl.h>
#include <GLFW/glfw3.h>
#include <stdio.h>

#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h" /* http://nothings.org/stb/stb_image_write.h */

#define HANDMADE_MATH_IMPLEMENTATION
#include "handmade_math.h"


void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void process_input(GLFWwindow *window);

const int screen_width = 1920;
const int screen_height = 1080;

hmm_vec3 cameraPos   = {.X = 0.0f, .Y = 0.0f, .Z = 900.0f};
hmm_vec3 cameraFront = {.X = 0.0f, .Y = 0.0f, .Z = -1.0f};
hmm_vec3 cameraUp    = {.X = 0.0f, .Y = 1.0f, .Z = 0.0f};

int firstMouse = 1;
float yaw   = -90.0f;	// yaw is initialized to -90.0 degrees since a yaw of 0.0 results in a direction vector pointing to the right so we initially rotate a bit to the left.
float pitch =  0.0f;
float lastX =  (float)screen_width / 2.0;
float lastY =  (float)screen_height / 2.0;
float fov   =  45.0f;

// timing
float deltaTime = 0.0f;	// time between current frame and last frame
float lastFrame = 0.0f;

const char *vs = "#version 460 core\n"
    "layout (location = 0) in vec3 aPos;\n"
    "layout (location = 1) in vec2 aTexCoord;\n"
    "\n"
    "out vec2 TexCoord;\n"
    "\n"
    "uniform mat4 model;\n"
    "uniform mat4 view;\n"
    "uniform mat4 projection;\n"
    "\n"
    "void main()\n"
    "{\n"
    "    gl_Position = projection*view*model*vec4(aPos, 1.0f);\n"
    "    TexCoord = vec2(aTexCoord.x, aTexCoord.y);\n"
    "}";

const char *fs = "#version 460 core\n"
    "out vec4 FragColor;\n"
    "\n"
    "in vec2 TexCoord;\n"
    "\n"
    "uniform sampler2D texture1;\n"
    "\n"
    "void main()\n"
    "{\n"
    "    FragColor = texture(texture1, TexCoord);\n"
    "}";

const char *textvs = "#version 460 core\n"
"layout (location = 0) in vec4 vertex; // <vec2 pos, vec2 tex>\n"
"out vec2 TexCoords;\n"
"\n"
"uniform mat4 projection;\n"
"\n"
"void main()\n"
"{\n"
"    gl_Position = projection * vec4(vertex.xy, 0.0, 1.0);\n"
"    TexCoords = vertex.zw;\n"
"}";

const char *textfs = "#version 460 core\n"
"in vec2 TexCoords;\n"
"out vec4 color;\n"
"\n"
"uniform sampler2D text;\n"
"uniform vec3 textColor;\n"
"\n"
"void main()\n"
"{    \n"
"    vec4 sampled = vec4(1.0, 1.0, 1.0, texture(text, TexCoords).r);\n"
"    color = vec4(textColor, 1.0) * sampled;\n"
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
    glfwSetCursorPosCallback(window, mouse_callback);

     // tell GLFW to capture our mouse
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // NOTE: you have to have a GL context prior to calling this
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        printf("Failed to initialize GLAD");
        return -1;
    }

    // configure global opengl state
    // ----
    glEnable(GL_DEPTH_TEST);

    // quads w/ textures shader program
    // ----
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vs, NULL);
    glCompileShader(vertexShader);
    int success;
    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        printf("ERROR::SHADER::VERTEX::COMPILATION_FAILED: %s\n", infoLog);
    }
    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fs, NULL);
    glCompileShader(fragmentShader);
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        printf("ERROR::SHADER::FRAGMENT::COMPILATION_FAILED: %s\n", infoLog);
    }
    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        printf("ERROR::SHADER::PROGRAM::LINKING_FAILED %s\n", infoLog);
    }
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // text shader program
    // ----
    unsigned int text_vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(text_vertex_shader, 1, &textvs, NULL);
    glCompileShader(text_vertex_shader);
    int text_success;
    char text_info_log[512];
    glGetShaderiv(text_vertex_shader, GL_COMPILE_STATUS, &text_success);
    if (!text_success)
    {
        glGetShaderInfoLog(text_vertex_shader, 512, NULL, text_info_log);
        printf("ERROR::TEXT_SHADER::VERTEX::COMPILATION_FAILED: %s\n", text_info_log);
    }
    unsigned int text_fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(text_fragment_shader, 1, &textfs, NULL);
    glCompileShader(text_fragment_shader);
    glGetShaderiv(text_fragment_shader, GL_COMPILE_STATUS, &text_success);
    if (!text_success)
    {
        glGetShaderInfoLog(text_fragment_shader, 512, NULL, text_info_log);
        printf("ERROR::SHADER::FRAGMENT::COMPILATION_FAILED: %s\n", text_info_log);
    }
    unsigned int text_shader_program = glCreateProgram();
    glAttachShader(text_shader_program, text_vertex_shader);
    glAttachShader(text_shader_program, text_fragment_shader);
    glLinkProgram(text_shader_program);
    glGetProgramiv(text_shader_program, GL_LINK_STATUS, &text_success);
    if (!text_success) {
        glGetProgramInfoLog(text_shader_program, 512, NULL, text_info_log);
        printf("ERROR::SHADER::PROGRAM::LINKING_FAILED %s\n", text_info_log);
    }
    glDeleteShader(text_vertex_shader);
    glDeleteShader(text_fragment_shader);

    hmm_mat4 ortho_projection = HMM_Orthographic(0.0, (float)screen_width, 0.0, (float)screen_height, 0.1f, 1000.0f);
    glUseProgram(text_shader_program);
    glUniformMatrix4fv(glGetUniformLocation(text_shader_program, "projection"), 1, GL_FALSE, (GLfloat*)ortho_projection.Elements);
    
    // font setup
    // ----
    /* long size; */
    /* unsigned char* fontBuffer; */
    /* FILE* fontFile = fopen("./third_party/fonts/Hack-Regular.ttf", "rb"); */
    /* fseek(fontFile, 0, SEEK_END); */
    /* size = ftell(fontFile); */
    /* fseek(fontFile, 0, SEEK_SET); */
    /* fontBuffer = malloc(size); */
    /* fread(fontBuffer, size, 1, fontFile); */
    /* fclose(fontFile); */

    /* stbtt_fontinfo info; */
    /* if (!stbtt_InitFont(&info, fontBuffer, 0)) */
    /* { */
    /*     printf("failed\n"); */
    /* } */

    /* char *ascii_chars = " !\"#$%&\'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~"; */
    
    /* int x = 0; */
    /* int y = 0; */
    /* int max_char_height = 0; */
    /* int l_h = 64; /1* line height -- font height *1/ */
    /* float scale = stbtt_ScaleForPixelHeight(&info, l_h); */
    /* int ascent, descent, lineGap; */
    /* stbtt_GetFontVMetrics(&info, &ascent, &descent, &lineGap); */
    /* ascent = roundf(ascent * scale); */
    /* descent = roundf(descent * scale); */

    /* // get the w/h of the atlas */
    /* for (size_t i = 0; i < strlen(ascii_chars); i++) */ 
    /* { */
    /*     int advance_width, left_side_bearing; */
    /*     stbtt_GetCodepointHMetrics(&info, ascii_chars[i], &advance_width, &left_side_bearing); */
    /*     /1* (Note that each Codepoint call has an alternative Glyph version which caches the work required to lookup the character word[i].) *1/ */
    /*     printf("advance_width: %d, left_side_bearing: %d\n", advance_width, left_side_bearing); */

    /*     /1* get bounding box for character (may be offset to account for chars that dip above or below the line) *1/ */
    /*     int c_x1, c_y1, c_x2, c_y2; */
    /*     stbtt_GetCodepointBitmapBox(&info, ascii_chars[i], scale, scale, &c_x1, &c_y1, &c_x2, &c_y2); */
    /*     printf("c_w: %d, c_h: %d\n", c_x2 - c_x1, c_y2 - c_y1); */
        
    /*     /1* compute y (different characters have different heights) *1/ */
    /*     y = ascent + c_y1; */
    /*     if (y > max_char_height) { */
    /*         max_char_height = y; */
    /*     } */
        
    /*     /1* render character (stride and offset is important here) *1/ */
    /*     /1* int byteOffset = x + roundf(left_side_bearing * scale) + (y * b_w); *1/ */
    /*     /1* stbtt_MakeCodepointBitmap(&info, bitmap + byteOffset, c_x2 - c_x1, c_y2 - c_y1, b_w, scale, scale, ascii_chars[i]); *1/ */

    /*     x += roundf(advance_width * scale); */
        
    /*     // get the space between two letters (kerning) */
    /*     int kern; */
    /*     kern = stbtt_GetCodepointKernAdvance(&info, ascii_chars[i], ascii_chars[i + 1]); */
    /*     x += roundf(kern * scale); */
    /* } */
    /* int font_atlas_width = x; */
    /* int font_atlas_height = l_h; */

    /* x = 0; */
    /* y = 0; */
    /* int b_w = font_atlas_width; /1* bitmap width *1/ */
    /* int b_h = l_h; /1* bitmap height *1/ */

    /* /1* create a bitmap for the phrase *1/ */
    /* unsigned char* bitmap = calloc(b_w * b_h, sizeof(unsigned char)); */
       
    /* for (size_t i = 0; i < strlen(ascii_chars); ++i) */
    /* { */
    /*     /1* advance x *1/ */
    /*     /1* how wide is this character *1/ */
    /*     int advance_width, left_side_bearing; */
    /*     stbtt_GetCodepointHMetrics(&info, ascii_chars[i], &advance_width, &left_side_bearing); */
    /*     /1* (Note that each Codepoint call has an alternative Glyph version which caches the work required to lookup the character word[i].) *1/ */
    /*     /1* get bounding box for character (may be offset to account for chars that dip above or below the line) *1/ */
    /*     int c_x1, c_y1, c_x2, c_y2; */
    /*     stbtt_GetCodepointBitmapBox(&info, ascii_chars[i], scale, scale, &c_x1, &c_y1, &c_x2, &c_y2); */
    /*     /1* compute y (different characters have different heights) *1/ */

    /*     y = ascent + c_y1; */
        
    /*     /1* render character (stride and offset is important here) *1/ */
    /*     int byteOffset = x + roundf(left_side_bearing * scale) + (y * b_w); */
    /*     stbtt_MakeCodepointBitmap(&info, bitmap + byteOffset, c_x2 - c_x1, c_y2 - c_y1, b_w, scale, scale, ascii_chars[i]); */

    /*     x += roundf(advance_width * scale); */
        
    /*     // get the space between two letters (kerning) */
    /*     int kern; */
    /*     kern = stbtt_GetCodepointKernAdvance(&info, ascii_chars[i], ascii_chars[i + 1]); */
    /*     x += roundf(kern * scale); */
    /* } */

    /* /1* save out a 1 channel image *1/ */
    /* stbi_write_png("out.png", b_w, b_h, 1, bitmap, b_w); */
    /* free(bitmap); */

    /* float xpos = (float)screen_width / 2; */
    /* float ypos = (float)screen_height / 2; */
    /* int w, h = 32; */

    /* float text_verts[6][4] = { */
    /*     { xpos,     ypos + h,   0.0f, 0.0f }, */            
    /*     { xpos,     ypos,       0.0f, 1.0f }, */
    /*     { xpos + w, ypos,       1.0f, 1.0f }, */

    /*     { xpos,     ypos + h,   0.0f, 0.0f }, */
    /*     { xpos + w, ypos,       1.0f, 1.0f }, */
    /*     { xpos + w, ypos + h,   1.0f, 0.0f } */           
    /* }; */
    /* GLuint font_vertex_array_obj; */
    /* GLuint font_vertex_buffer_obj; */
    /* glGenVertexArrays(1, &font_vertex_array_obj); */
    /* glGenBuffers(1, &font_vertex_buffer_obj); */
    /* glBindVertexArray(font_vertex_array_obj); */
    /* glBindBuffer(GL_ARRAY_BUFFER, font_vertex_buffer_obj); */
    /* glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW); */
    /* glEnableVertexAttribArray(0); */
    /* glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4*sizeof(float), 0); */
    /* glBindBuffer(GL_ARRAY_BUFFER, 0); */
    /* glBindVertexArray(0); */

    /* unsigned int font_texture; */
    /* glGenTextures(1, &font_texture); */
    /* glBindTexture(GL_TEXTURE_2D, font_texture); */ 
    /* glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); */
    /* glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); */
    /* glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); */
    /* glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); */
    /* glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, font_atlas_width, font_atlas_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, bitmap); */

    // set up vertex data (and buffer(s)) and configure vertex attributes
    // ----
    float vertices[] = {
        -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
         0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,

        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
        -0.5f,  0.5f,  0.5f,  0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,

        -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
        -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

         0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
         0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  1.0f, 1.0f,
         0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,

        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
        -0.5f,  0.5f,  0.5f,  0.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f
    };
    // world space positions of our cubes
    hmm_vec3 cubePositions[] = {
        HMM_Vec3( 0.0f,  0.0f,  0.0f),
        HMM_Vec3( 2.0f,  5.0f, -15.0f),
        HMM_Vec3(-1.5f, -2.2f, -2.5f),
        HMM_Vec3(-3.8f, -2.0f, -12.3f),
        HMM_Vec3( 2.4f, -0.4f, -3.5f),
        HMM_Vec3(-1.7f,  3.0f, -7.5f),
        HMM_Vec3( 1.3f, -2.0f, -2.5f),
        HMM_Vec3( 1.5f,  2.0f, -2.5f),
        HMM_Vec3( 1.5f,  0.2f, -1.5f),
        HMM_Vec3(-1.3f,  1.0f, -1.5f)
    };

    unsigned int VBO, VAO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // texture coord attribute
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // doge and jeremey textures
    unsigned int texture1;
    glGenTextures(1, &texture1);
    glBindTexture(GL_TEXTURE_2D, texture1); 
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	// set texture wrapping to GL_REPEAT (default wrapping method)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // set texture filtering parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // load image, create texture and generate mipmaps
    int width, height, nrChannels;
    stbi_set_flip_vertically_on_load(1); // tell stb_image.h to flip loaded texture's on the y-advance_widthis.
    unsigned char *data = stbi_load("./third_party/images/pp.jpg", &width, &height, &nrChannels, 4);
    if (data)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        printf("Failed to load texture\n");
    }
    stbi_image_free(data);
    
    glUseProgram(shaderProgram);
    glUniform1i(glGetUniformLocation(shaderProgram, "texture1"), 0);

    // You can unbind the VAO afterwards so other VAO calls won't accidentally modify this VAO, but this rarely happens. Modifying other
    // VAOs requires a call to glBindVertexArray anyways so we generally don't unbind VAOs (nor VBOs) when it's not directly necessary.
    glBindVertexArray(0);

    // local space is the coordinates of the object
    // view is the "camera" -- think FPS view
    GLuint projLoc = glGetUniformLocation(shaderProgram, "projection");
    GLuint viewLoc = glGetUniformLocation(shaderProgram, "view");
    GLuint modelLoc = glGetUniformLocation(shaderProgram, "model");

    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window))
    {
        // per-frame time 
        // ----
        float currentFrame = (float)glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // input
        // ----
        process_input(window);

        // render
        // ------
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture1);

        glUseProgram(shaderProgram);
        glBindVertexArray(VAO);

        hmm_mat4 projection = HMM_Perspective(HMM_ToRadians(fov), (float)screen_width/(float)screen_height, 0.1f, 5000.0f);
        // L-R / U-D / Front-Back
        hmm_mat4 view       = HMM_LookAt(cameraPos, HMM_AddVec3(cameraPos, cameraFront), cameraUp);
        // pass transformation matrices to the shader
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, (const GLfloat*)projection.Elements);
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, (const GLfloat*)view.Elements);


        // draw cubes
        for (int i = 0; i < 10; i++) {
            hmm_mat4 model = HMM_Translate(cubePositions[i]);
            int n = i;
            if (n == 0) {
                n = 1;
            }
            float angle = 20.0f*n;
            model = HMM_Rotate_With_Mat4(model, ((float)glfwGetTime() * HMM_ToRadians(angle)*100), HMM_Vec3(1.0f, 0.3f, 0.5f));
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, (const GLfloat*)model.Elements);

            glDrawArrays(GL_TRIANGLES, 0, 36);
        }
        glBindVertexArray(0);
        glBindTexture(GL_TEXTURE_2D, 0);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void process_input(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, 1);
    }

    float cameraSpeed = (float)(10.5*deltaTime);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        cameraPos = HMM_AddVec3(cameraPos, HMM_MultiplyVec3f(cameraFront, cameraSpeed*100.5f));
    }

    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        cameraPos = HMM_SubtractVec3(cameraPos, HMM_MultiplyVec3f(cameraFront, cameraSpeed*100.5f));
    }

    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        hmm_vec3 cross = HMM_Cross(cameraFront, cameraUp);
        hmm_vec3 normalized = HMM_NormalizeVec3(cross);
        hmm_vec3 product = HMM_MultiplyVec3f(normalized, cameraSpeed);
        cameraPos = HMM_SubtractVec3(cameraPos, product);
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        hmm_vec3 cross = HMM_Cross(cameraFront, cameraUp);
        hmm_vec3 normalized = HMM_NormalizeVec3(cross);
        hmm_vec3 product = HMM_MultiplyVec3f(normalized, cameraSpeed);
        cameraPos = HMM_AddVec3(cameraPos, product);
    }
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
    float xpos = (float)xposIn;
    float ypos = (float)yposIn;

    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = 0;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top
    lastX = xpos;
    lastY = ypos;

    float sensitivity = 0.00035f; // change this value to your liking
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yaw += xoffset;
    pitch += yoffset;

    // make sure that when pitch is out of bounds, screen doesn't get flipped
    if (pitch > 89.0f)
        pitch = 89.0f;
    if (pitch < -89.0f)
        pitch = -89.0f;

    hmm_vec3 front = {
        .X = cos(HMM_ToRadians(yaw)) * cos(HMM_ToRadians(pitch)),
        .Y = sin(HMM_ToRadians(pitch)),
        .Z = sin(HMM_ToRadians(yaw)) * cos(HMM_ToRadians(pitch))
    };

    cameraFront = HMM_NormalizeVec3(front);
}

