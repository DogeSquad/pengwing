#include "common.hpp"
#include "Shader.h"
#include "mesh.hpp"
#include "camera.hpp"
#include "Object.h"
#include "Drache.h"

#include <string>
#include <chrono>
#include <imgui.hpp>

// Window Settings
const int WINDOW_WIDTH =  1920;
const int WINDOW_HEIGHT = 1080;

// Camera Settings
const float FOV = 45.f;
const float NEAR_VALUE = 0.1f;
const float FAR_VALUE = 100.f;

// GUI Settings
bool enableGUI = true;
const int timeline_height = 200;

// Timeline Settings
const int FPS = 60;
const float duration = 30;
int i_FRAME = 0;
int loop_start = 0;
int loop_end = int(FPS * duration);
bool play = true;

// Forward Declaration
void handleGUI(std::vector<Object*> objects);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);

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

    if (enableGUI) init_imgui(window);

    camera cam(window);
    proj_matrix = glm::perspective(FOV, 1.f, NEAR_VALUE, FAR_VALUE);

    // Loading Shaders
    Shader shader = Shader("basic_colors.vert", "basic_colors.frag");

    // Loading Texture

    // Loading Objects
    glm::mat4 scene = glm::identity<glm::mat4>();

    std::vector<Object*> objects;
    objects.push_back(new Drache(shader, loadMesh("dragon.obj", true), &scene, "Drache"));
    objects.push_back(new Object(shader, loadMesh("plane.obj", true), &objects[0]->model_matrix, "Plane"));
    objects[1]->scale = glm::vec3(10.0f, 1.0f, 10.0f);

    glEnable(GL_DEPTH_TEST);

    glfwSetKeyCallback(window, key_callback);
    glfwMaximizeWindow(window);
    // rendering loop
    while (glfwWindowShouldClose(window) == false) {
        // FPS limiting
        std::chrono::time_point<std::chrono::system_clock> start, end;
        start = std::chrono::system_clock::now();

        glfwPollEvents();
        glClearColor(0.25f, 0.25f, 0.25f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        if (enableGUI) handleGUI(objects);

        // Render and Update Objects
        for (unsigned i = 0; i < objects.size(); ++i) {
            objects[i]->update(i_FRAME);
            objects[i]->render(cam.view_matrix(), proj_matrix);
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

        if (enableGUI) imgui_render();
        glfwSwapBuffers(window);
        
        // FPS limiting
        end = std::chrono::system_clock::now();
        std::chrono::duration<double, std::milli> elapsed_milliseconds = end - start;
        Sleep(glm::max(0.0, 1000.0 / double(FPS) - elapsed_milliseconds.count()));
    }

    for (unsigned int i = 0; i < objects.size(); ++i) {
        objects[i]->destroy();
    }
    
    cleanup_imgui();
    glfwTerminate();
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_G && action == GLFW_PRESS)
        enableGUI = !enableGUI;
    if (key == GLFW_KEY_SPACE && action == GLFW_PRESS)
        play = !play;
}

void handleGUI(std::vector<Object*> objects) {
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

    ImGui::Begin("Objects", NULL, ImGuiWindowFlags_NoMove);
    ImGui::SetWindowPos(ImVec2(0, 0));
    for (unsigned int i = 0; i < objects.size(); ++i) {
        ImGui::Checkbox(std::string(objects[i]->name).c_str(), &objects[i]->active);
    }
    ImGui::End();
}
void resizeCallback(GLFWwindow*, int width, int height) {
    // set new width and height as viewport size
    glViewport(0, 0, width, height);
    proj_matrix = glm::perspective(FOV, static_cast<float>(width) / height, NEAR_VALUE, FAR_VALUE);
}
