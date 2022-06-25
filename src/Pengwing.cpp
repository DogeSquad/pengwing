#include "common.hpp"
#include "Shader.h"
#include "Model.h"
#include "camera_orbital.hpp"
#include "Object.h"
#include "Drache.h"
#include "Camera.h"

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

    stbi_set_flip_vertically_on_load(true);

    if (enableGUI) init_imgui(window);

    //camera_orbital cam(window);
    proj_matrix = glm::perspective(FOV, 1.f, NEAR_VALUE, FAR_VALUE);

    // Loading Objects ----------------------------------
    // --
    glm::mat4 scene_mat = glm::identity<glm::mat4>();

    std::vector<Object*> objects = std::vector<Object*>();

    Shader albedo_texture = Shader("basic_textured.vert", "basic_textured.frag");
    Shader shader = Shader("basic_colors.vert", "basic_colors.frag");
    Shader sunglasses_shader = Shader("basic_colors.vert", "basic_colors_black.frag");

    {
        objects.push_back(new Drache(shader, Model("dragon.obj", true), &scene_mat, "Drache"));
        objects[0]->active = false;
        objects.push_back(new Object(sunglasses_shader, Model("sunglasses/sunglasses.obj", true), &objects[0]->model_matrix, "Sunglasses"));
        objects[1]->position = glm::vec3(-4.9f, 8.1f, -0.1f);
        objects[1]->rotation = glm::vec4(0.0f, 1.0f, 0.0f, glm::half_pi<float>() + 0.4f);
        objects[1]->scale = glm::vec3(19.0f, 19.0f, 19.0f);
        objects[1]->active = false;
    }

    objects.push_back(new Object(shader, Model("plane.obj", false), &scene_mat, "Plane"));
    objects[2]->scale = glm::vec3(10.0f, 1.0f, 10.0f);
    objects[2]->position = glm::vec3(0.0f, -2.0f, 0.0f);
    objects[2]->active = false;

    objects.push_back(new Drache(albedo_texture, Model("backpack/backpack.obj", true), &scene_mat, "Backpack"));
    objects[3]->position = glm::vec3(0.0f, 0.0f, 0.0f);
    objects[3]->active = true;


    Camera cam = Camera(&scene_mat, "Camera");
    // --
    // ---------------------------------------------------
    
    // Set up Post Processing Quad A
    unsigned int framebufferA;
    glGenFramebuffers(1, &framebufferA);
    glBindFramebuffer(GL_FRAMEBUFFER, framebufferA);

    unsigned int textureColorbufferA;
    glGenTextures(1, &textureColorbufferA);
    glBindTexture(GL_TEXTURE_2D, textureColorbufferA);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, WINDOW_WIDTH, WINDOW_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureColorbufferA, 0);

    unsigned int rboA;
    glGenRenderbuffers(1, &rboA);
    glBindRenderbuffer(GL_RENDERBUFFER, rboA);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, WINDOW_WIDTH, WINDOW_HEIGHT);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rboA);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
    // Set up Post Processing Quad B
    unsigned int framebufferB;
    glGenFramebuffers(1, &framebufferB);
    glBindFramebuffer(GL_FRAMEBUFFER, framebufferB);

    unsigned int textureColorbufferB;
    glGenTextures(1, &textureColorbufferB);
    glBindTexture(GL_TEXTURE_2D, textureColorbufferB);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, WINDOW_WIDTH, WINDOW_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureColorbufferB, 0);

    unsigned int rboB;
    glGenRenderbuffers(1, &rboB);
    glBindRenderbuffer(GL_RENDERBUFFER, rboB);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, WINDOW_WIDTH, WINDOW_HEIGHT);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rboB);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    float quadVertices[] = {
        // positions   texCoords
        -1.0f,  1.0f,  0.0f, 1.0f,
        -1.0f, -1.0f,  0.0f, 0.0f,
         1.0f, -1.0f,  1.0f, 0.0f,

        -1.0f,  1.0f,  0.0f, 1.0f,
         1.0f, -1.0f,  1.0f, 0.0f,
         1.0f,  1.0f,  1.0f, 1.0f
    };
    unsigned int quadVAO, quadVBO;
    glGenVertexArrays(1, &quadVAO);
    glGenBuffers(1, &quadVBO);
    glBindVertexArray(quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    Shader edgeDetection("simple.vert", "Postprocessing/postprocessing_edgeDetection.frag");
    edgeDetection.use();
    edgeDetection.setInt("screenTexture", 0);

    Shader inversion("simple.vert", "Postprocessing/postprocessing_basic.frag");
    inversion.use();
    inversion.setInt("screenTexture", 0);

    // Depth Testing
    glEnable(GL_DEPTH_TEST);

    // Blending
    glEnable(GL_BLEND);

    // Anti-Aliasing
    glEnable(GL_POLYGON_SMOOTH);
    glEnable(GL_MULTISAMPLE);
    glfwWindowHint(GLFW_SAMPLES, 4);

    glfwSetKeyCallback(window, key_callback);
    glfwMaximizeWindow(window);

    std::chrono::time_point<std::chrono::system_clock> start, end;
    // rendering loop
    while (glfwWindowShouldClose(window) == false) {
        // FPS limiting
        start = std::chrono::system_clock::now();

        glfwPollEvents();

        // First pass --> render to framebuffer
        // ----------------------------------------------------------------------
        glBindFramebuffer(GL_FRAMEBUFFER, framebufferA);
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glEnable(GL_DEPTH_TEST);
        cam.update(i_FRAME);
        // Render and Update Objects
        for (unsigned i = 0; i < objects.size(); ++i) {
            objects[i]->update(i_FRAME);
            objects[i]->render(cam.viewMatrix(), proj_matrix);
        }
        // Second pass -> render framebuffer A to framebuffer b
        // ----------------------------------------------------------------------
        glBindFramebuffer(GL_FRAMEBUFFER, framebufferB);
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        edgeDetection.use();
        glBindVertexArray(quadVAO);
        glDisable(GL_DEPTH_TEST);
        glBindTexture(GL_TEXTURE_2D, textureColorbufferA);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        // Third pass -> render framebuffer B to actual screen buffer
        // ----------------------------------------------------------------------
        glBindFramebuffer(GL_FRAMEBUFFER, 0); // reset to actual screen buffer
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        inversion.use();
        glBindVertexArray(quadVAO);
        glDisable(GL_DEPTH_TEST);
        glBindTexture(GL_TEXTURE_2D, textureColorbufferB);
        glDrawArrays(GL_TRIANGLES, 0, 6);


        if (enableGUI) handleGUI(objects);

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

    glDeleteFramebuffers(1, &framebufferA);
    glDeleteFramebuffers(1, &framebufferB);
    glDeleteRenderbuffers(1, &rboA);
    glDeleteRenderbuffers(1, &rboB);

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
