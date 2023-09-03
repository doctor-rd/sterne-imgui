#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <stdlib.h>
#include <fstream>
#include <vector>
#include <GLES2/gl2.h>
#include <GLFW/glfw3.h>

typedef struct {
  GLfloat x;
  GLfloat y;
  GLfloat z;
} Coord;

GLfloat rnd() {
    return (GLfloat)rand() / (GLfloat)RAND_MAX;
}

void appendStars(std::vector<Coord> &coord, int n, GLfloat r, GLfloat m) {
    for (int i=0; i<n; i++)
        coord.push_back((Coord){r*(2.0f*rnd()-1.0f), r*(2.0f*rnd()-1.0f), m*rnd()});
}

void moveStars(std::vector<Coord> &coord, GLfloat m, GLfloat d) {
    for (Coord &c : coord) {
        c.z+=d;
        while (c.z>m)
            c.z-=m;
        while (c.z<0.0f)
            c.z+=m;
    }
}

const int width = 640;
const int height = 480;

struct bmp_header_t {
    unsigned short bfType;
    unsigned int bfSize;
    unsigned int bfReserved;
    unsigned int bfOffBits;
} __attribute__((packed)) bmp_header = {0x4D42, 0, 0, 54};

struct bmp_info_t {
    unsigned int biSize;
    int biWidth;
    int biHeight;
    unsigned short biPlanes;
    unsigned short biBitCount;
    unsigned int biCompression;
    unsigned int biSizeImage;
    int biXPelsPerMeter;
    int biYPelsPerMeter;
    unsigned int biClrUsed;
    unsigned int biClrImportant;
} __attribute__((packed)) bmp_info = {40, width, height, 1, 24, 0, 0, 0, 0, 0, 0};

struct {
    bmp_header_t bmp_header;
    bmp_info_t bmp_info;
} __attribute__((packed)) bmp_headers = {bmp_header, bmp_info};

void take_screenshot(char *filename) {
    char buffer[width*height*3];
    glReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, buffer);
    auto bmpfile = std::fstream(filename, std::ios::out | std::ios::binary);
    bmpfile.write(((const char*) &bmp_headers), sizeof(bmp_headers));
    bmpfile.write(buffer, sizeof(buffer));
    bmpfile.close();
}

const char* vertex_shader =
"#version 100\n"
"attribute vec3 coord;"
"varying lowp float vBright;"
"void main() {"
"  gl_Position = vec4(coord.x, coord.y, -0.1*coord.z, 2.0*coord.z-1.0);"
"  vBright = 1.2-(coord.z/4.5);"
"}";

const char* fragment_shader =
"#version 100\n"
"varying lowp float vBright;"
"void main() {"
"  gl_FragColor = vec4(vBright, vBright, vBright, 1.0);"
"}";

int main() {
    if (!glfwInit())
        return 1;
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    GLFWwindow* window = glfwCreateWindow(width, height, "Sterne", NULL, NULL);
    if (!window) {
        glfwTerminate();
        return 1;
    }
    glfwMakeContextCurrent(window);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 100");

    GLuint vs = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vs, 1, &vertex_shader, NULL);
    glCompileShader(vs);
    GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fs, 1, &fragment_shader, NULL);
    glCompileShader(fs);
    GLuint shader_programme = glCreateProgram();
    glAttachShader(shader_programme, fs);
    glAttachShader(shader_programme, vs);
    glLinkProgram(shader_programme);

    int n_stars = 8000;
    const GLfloat d = 10.0f;
    std::vector<Coord> stars;
    GLfloat speed = -0.1f;
    char screenshot_filename[256] = "screenshot.bmp";

    double then = 0.0;
    while (!glfwWindowShouldClose(window)) {
        double now = glfwGetTime();
        double deltaTime = now - then;
        then = now;

        if (n_stars>stars.size())
            appendStars(stars, n_stars-stars.size(), 6.0f, d);
        if (n_stars<stars.size())
            stars.resize(n_stars);

        std::vector<Coord> vertices;
        for (Coord c : stars) {
            vertices.push_back(c);
            c.z-=speed;
            vertices.push_back(c);
        }

        GLuint vbo = 0;
        glGenBuffers(1, &vbo);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, vertices.size()*sizeof(Coord), vertices.data(), GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
        glUseProgram(shader_programme);
        GLint loc_speed = glGetUniformLocation(shader_programme, "speed");
        glUniform1f(loc_speed, speed);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LEQUAL);
        glDrawArrays(GL_LINES, 0, vertices.size());
        glDisableVertexAttribArray(0);
        moveStars(stars, d, speed*deltaTime/0.2);
        glfwPollEvents();
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            break;
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        ImGui::Begin("Settings");
        ImGui::SliderFloat("speed", &speed, -1.0f, 1.0f);
        ImGui::SliderInt("number of stars", &n_stars, 100, 40000);
        ImGui::InputText("Screenshot filename", screenshot_filename, sizeof(screenshot_filename));
        if (ImGui::Button("Screenshot"))
            take_screenshot(screenshot_filename);
        ImGui::Text("deltaTime %.3f (%.1f FPS)", deltaTime, io.Framerate);
        ImGui::End();
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
    }

    glfwTerminate();
    return 0;
}
