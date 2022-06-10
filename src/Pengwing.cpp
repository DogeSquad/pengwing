#include "common.hpp"
#include "Shader.h"
#include "mesh.hpp"
#include "camera.hpp"

#include <chrono>
#include <imgui.hpp>;

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

    Shader shader = Shader("basic_colors.vert", "basic_colors.frag");

    // load and compile shaders and link program

    std::vector<geometry> objects = loadScene("dragon.obj", true);
    objects.push_back (loadMesh("suzanne.obj", true));
    proj_matrix = glm::perspective(FOV, 1.f, NEAR_VALUE, FAR_VALUE);

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
        ImGui::SetWindowPos(ImVec2(0, WINDOW_HEIGHT - ImGui::GetWindowSize().y));
        ImGui::SetWindowSize(ImVec2(1920, timeline_height));
        if (ImGui::Button("Start/Pause"))
        {
            play = !play;
        }
        ImGui::SliderInt("Frame", &i_FRAME, 0, 30 * 60);
        ImGui::SliderInt("Loop Start", &loop_start, 0, 30 * 60);
        ImGui::SliderInt("Loop End", &loop_end, 0, 30 * 60);
        ImGui::End();
        ImGui::Begin("Other");
        if (ImGui::Button("Start/Pause"))
        {
            play = !play;
        }
        ImGui::End();

        // Render and Update Objects
        for (unsigned i = 0; i < objects.size(); ++i) {
            glm::mat4 model_matrix = objects[i].transform;
            model_matrix = glm::translate(model_matrix, glm::vec3(0.0f, sin(0.01f * i_FRAME), 0.0f));

            shader.use();
            shader.setMat4("model_mat", model_matrix);
            shader.setMat4("view_mat", cam.view_matrix());
            shader.setMat4("proj_mat", proj_matrix);
            shader.setVec3("light_dir", glm::vec3(1.0f, 0.f, 0.f));

            objects[i].bind();
            glDrawElements(GL_TRIANGLES, objects[i].vertex_count, GL_UNSIGNED_INT, (void*) 0);
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
