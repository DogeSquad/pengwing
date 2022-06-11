#include "common.hpp"
#include "Shader.h"
#include "mesh.hpp"
#include "camera.hpp"
#include "Object.h"

#include <string>
#include <chrono>
#include <imgui.hpp>

const int WINDOW_WIDTH =  1920;
const int WINDOW_HEIGHT = 1080;
const float FOV = 45.f;
const float NEAR_VALUE = 0.1f;
const float FAR_VALUE = 100.f;

#ifndef M_PI
#define M_PI 3.14159265359
#endif

glm::mat4 proj_matrix;

void
resizeCallback(GLFWwindow* window, int width, int height);

int
main(int, char* argv[]) {
    GLFWwindow* window = initOpenGL(WINDOW_WIDTH, WINDOW_HEIGHT,"Pengwing");
    glfwSetFramebufferSizeCallback(window, resizeCallback);

    init_imgui(window);

    camera cam(window);
    proj_matrix = glm::perspective(FOV, 1.f, NEAR_VALUE, FAR_VALUE);

    // Loading Shaders
    Shader shader = Shader("basic_colors.vert", "basic_colors.frag");

    // Loading Objects
    std::vector<Object> objects = std::vector<Object>();
    objects.push_back(Object(shader, loadMesh("dragon.obj", true)));   // 0 - Dragon
    objects.push_back(Object(shader, loadMesh("suzanne.obj", true)));  // 1 - Suzanne

    glEnable(GL_DEPTH_TEST);

    // GUI Settings
    int timeline_height = 200;
    
    // Timeline Setting
    int i_FRAME = 0;
    int FPS = 60;
    float duration = 30;
    int loop_start = 0;
    int loop_end = int(FPS * duration);
    bool play = true;

    glfwMaximizeWindow(window);
    // rendering loop
    while (glfwWindowShouldClose(window) == false) {
        // FPS limiting
        std::chrono::time_point<std::chrono::system_clock> start, end;
        start = std::chrono::system_clock::now();

        glfwPollEvents();
        glClearColor(0.25f, 0.25f, 0.25f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Handle GUI
        imgui_new_frame(1920, 200);
        ImGui::Begin("Timeline", NULL, ImGuiWindowFlags_NoMove);
        ImGui::Columns(2);
        ImGui::SetWindowPos(ImVec2(0, WINDOW_HEIGHT - ImGui::GetWindowSize().y));
        ImGui::SetWindowSize(ImVec2(1920, timeline_height));
        if (ImGui::Button("Start/Pause"))
        {
            play = !play;
        }
        //ImGui::SliderInt(std::to_string(i_FRAME / FPS).append("s Frame").c_str(), &i_FRAME, 0, duration * FPS);
        ImGui::SliderInt("Frame", &i_FRAME, 0, duration * FPS);
        ImGui::SliderInt("Loop Start", &loop_start, 0, duration * FPS);
        ImGui::SliderInt("Loop End", &loop_end, 0, duration * FPS);
        ImGui::NextColumn();
        ImGui::Text("");
        ImGui::End();

        ImGui::Begin("Objects");
        for (unsigned int i = 0; i < objects.size(); ++i) {
            ImGui::Checkbox(std::to_string(i).c_str(), &objects[i].active);
        }
        ImGui::End();

        // Render and Update Objects
        for (unsigned i = 0; i < objects.size(); ++i) {
            objects[i].update(i_FRAME);
            objects[i].render(cam.view_matrix(), proj_matrix);
        }

        // Advance Timeline
        if (play)
        {
            i_FRAME += 1;
            if (i_FRAME > loop_end)
            {
                i_FRAME = loop_start;
            }
        }

        imgui_render();
        glfwSwapBuffers(window);
        
        // FPS limiting
        end = std::chrono::system_clock::now();
        std::chrono::duration<double, std::milli> elapsed_seconds = end - start;
        Sleep(glm::max(0.0, 1 / FPS -  elapsed_seconds.count()));
    }

    for (unsigned int i = 0; i < objects.size(); ++i) {
        objects[i].destroy();
    }
    
    cleanup_imgui();
    glfwTerminate();
}

void resizeCallback(GLFWwindow*, int width, int height)
{
    // set new width and height as viewport size
    glViewport(0, 0, width, height);
    proj_matrix = glm::perspective(FOV, static_cast<float>(width) / height, NEAR_VALUE, FAR_VALUE);
}
