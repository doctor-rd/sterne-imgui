#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fstream>
#include <vector>
#include <GLES2/gl2.h>
#include <GLFW/glfw3.h>
extern "C" {
#include <libavcodec/avcodec.h>
}

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

void store_frame(AVFrame *frame) {
    GLfloat r[width*height];
    GLfloat g[width*height];
    GLfloat b[width*height];
    glReadPixels(0, 0, width, height, GL_RED, GL_FLOAT, r);
    glReadPixels(0, 0, width, height, GL_RED, GL_FLOAT, g);
    glReadPixels(0, 0, width, height, GL_RED, GL_FLOAT, b);
    for (int y=0; y<height; y++) {
        for (int x=0; x<width; x++) {
            int p = (height-y)*width+x;
            frame->data[0][y*width+x] = (r[p]*255.f + g[p]*255.f + b[p]*255.f)/3.f;
        }
    }
    memset(frame->data[1], 127, width*height/4);
    memset(frame->data[2], 127, width*height/4);
}

void write_packets(AVCodecContext *ctx, std::fstream &f) {
    AVPacket pkt;
    av_init_packet(&pkt);
    while (1) {
        int ret = avcodec_receive_packet(ctx, &pkt);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
            return;
        else if (ret < 0) {
            fprintf(stderr, "Error during encoding\n");
            exit(1);
        }
        printf("Write packet %3"PRId64" (size=%5d)\n", pkt.pts, pkt.size);
        f.write((const char*)pkt.data, pkt.size);
        av_packet_unref(&pkt);
    }
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

    const AVCodec *codec = avcodec_find_encoder(AV_CODEC_ID_H264);

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
        ImGui::Text("deltaTime %.3f (%.1f FPS)", deltaTime, io.Framerate);
        ImGui::End();
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
    }

    glfwTerminate();
    return 0;
}
