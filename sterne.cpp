#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <stdlib.h>
#include <vector>
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

int main() {
    if (!glfwInit())
        return 1;
    GLFWwindow* window = glfwCreateWindow(640, 480, "Sterne", NULL, NULL);
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
    ImGui_ImplOpenGL3_Init("#version 400");

    double then = 0.0;
    while (!glfwWindowShouldClose(window)) {
        double now = glfwGetTime();
        double deltaTime = now - then;
        then = now;

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glfwPollEvents();
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            break;
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        ImGui::Begin("Settings");
        ImGui::Text("deltaTime %.3f (%.1f FPS)", deltaTime, io.Framerate);
        ImGui::End();
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
    }

    glfwTerminate();
    return 0;
}
