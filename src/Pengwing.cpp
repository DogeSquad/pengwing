#include "common.hpp"
#include "Shader.h"
#include "Model.h"
#include "camera_orbital.hpp"
#include "Object.h"
#include "Drache.h"
#include "Camera.h"
#include "Postprocessing.h"

#include <string>
#include <chrono>
#include <imgui.hpp>

// Window Settings
const int WINDOW_WIDTH       = 1920;
const int WINDOW_HEIGHT      = 1080;
constexpr float ASPECT_RATIO = static_cast<float>(WINDOW_WIDTH) / static_cast<float>(WINDOW_HEIGHT);

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
void render_scene(std::vector<Object*> objects, Camera* cam, unsigned int frame);
void render_depth(std::vector<Object*> objects, Shader* shader, unsigned int frame);

#ifndef M_PI
#define M_PI 3.14159265359
#endif

glm::mat4 proj_matrix;

void
resizeCallback(GLFWwindow* window, int width, int height);

int
main(int, char* argv[]) {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SAMPLES, 4);

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
        objects[0]->active = true;
        objects.push_back(new Object(sunglasses_shader, Model("sunglasses/sunglasses.obj", true), &objects[0]->model_matrix, "Sunglasses"));
        objects[1]->position = glm::vec3(-4.9f, 8.1f, -0.1f);
        objects[1]->rotation = glm::vec4(0.0f, 1.0f, 0.0f, glm::half_pi<float>() + 0.4f);
        objects[1]->scale = glm::vec3(19.0f, 19.0f, 19.0f);
        objects[1]->active = true;
    }

    objects.push_back(new Object(shader, Model("plane.obj", false), &scene_mat, "Plane"));
    objects[2]->scale = glm::vec3(10.0f, 1.0f, 10.0f);
    objects[2]->position = glm::vec3(0.0f, 0.0f, 0.0f);
    objects[2]->active = true;

    objects.push_back(new Drache(albedo_texture, Model("backpack/backpack.obj", true), &scene_mat, "Backpack"));
    objects[3]->position = glm::vec3(0.0f, 0.0f, 0.0f);
    objects[3]->active = false;
    // --
    // ---------------------------------------------------
    
    // Shadow mapping ------------------------------------------------
    const unsigned int SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;
    unsigned int depthMapFBO;
    glGenFramebuffers(1, &depthMapFBO);

    unsigned int depthMap;
    glGenTextures(1, &depthMap);
    glBindTexture(GL_TEXTURE_2D, depthMap);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
        SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    // --
    // ---------------------------------------------------

    Camera cam = Camera(&scene_mat, "Camera");
    
    // Shadow Mapping
    Shader debugDepthQuad("simple.vert", "ShadowMapping/depth_debug_shader.frag");
    Shader simpleDepthShader("ShadowMapping/depth_shader.vert", "empty.frag");
    debugDepthQuad.use();
    debugDepthQuad.setInt("depthMap", 0);

    float near_plane = 1.0f, far_plane = 7.5f;
    glm::mat4 lightProjection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, near_plane, far_plane);
    glm::mat4 lightView = glm::lookAt(glm::vec3(-2.0f, 4.0f, -1.0f),
        glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec3(0.0f, 1.0f, 0.0f));
    glm::mat4 lightSpaceMatrix = lightProjection * lightView;
    simpleDepthShader.use();
    simpleDepthShader.setMat4("lightSpaceMatrix", lightSpaceMatrix);

    // Post processing shaders
    Postprocessing postprocessing(WINDOW_WIDTH, WINDOW_HEIGHT);

    Shader sdf("simple.vert", "Postprocessing/postprocessing_simpleSDF.frag");
    sdf.use();
    sdf.setInt("screenTexture", 0);
    sdf.setFloat("aspectRatio", ASPECT_RATIO);

    // Depth Testing
    glEnable(GL_DEPTH_TEST);

    // Blending
    glEnable(GL_BLEND);

    // Anti-Aliasing
    glEnable(GL_POLYGON_SMOOTH);
    glEnable(GL_MULTISAMPLE);

    glfwSetKeyCallback(window, key_callback);
    glfwMaximizeWindow(window);

    // Lighting
    glm::vec3 lightPos(0.0f, 5.0f, 5.0f);

    std::chrono::time_point<std::chrono::system_clock> start, end;
    // rendering loop
    while (glfwWindowShouldClose(window) == false) {
        // FPS limiting
        start = std::chrono::system_clock::now();

        glfwPollEvents();

        cam.update(i_FRAME);
        // First pass --> render to shadow
        // ----------------------------------------------------------------------
        glm::mat4 lightProjection, lightView;
        glm::mat4 lightSpaceMatrix;
        float near_plane = 1.0f, far_plane = 7.5f;
        lightProjection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, near_plane, far_plane);
        lightView = glm::lookAt(lightPos, glm::vec3(0.0f), glm::vec3(0.0, 1.0, 0.0));
        lightSpaceMatrix = lightProjection * lightView;
        // render scene from light's point of view
        simpleDepthShader.use();
        simpleDepthShader.setMat4("lightSpaceMatrix", lightSpaceMatrix);

        glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
        glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
        glClear(GL_DEPTH_BUFFER_BIT);
        render_depth(objects, &simpleDepthShader, i_FRAME);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // reset viewport
        glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // render Depth map to quad for visual debugging
        // ---------------------------------------------
        debugDepthQuad.use();
        debugDepthQuad.setFloat("near_plane", near_plane);
        debugDepthQuad.setFloat("far_plane", far_plane);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, depthMap);
        postprocessing.renderQuad();

        /*
        // Second pass
        glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glBindTexture(GL_TEXTURE_2D, depthMap);
        glEnable(GL_DEPTH_TEST);
        render_scene(objects, &cam, i_FRAME);
        */

        // Third pass -> render framebuffer A to screen buffer
        // ----------------------------------------------------------------------
        // postprocessing.postprocess(&sdf, RenderDirection::A_TO_SCR);

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

    postprocessing.destroy();

    cleanup_imgui();
    glfwTerminate();
}

void render_depth(std::vector<Object*> objects, Shader* shader,  unsigned int frame)
{
    for (unsigned i = 0; i < objects.size(); ++i) {
        objects[i]->update(frame);
        shader->setMat4("model", objects[i]->model_matrix);
        objects[i]->render(shader);
    }
}

void render_scene(std::vector<Object*> objects, Camera* cam, unsigned int frame)
{
    // Render and Update Objects
    for (unsigned i = 0; i < objects.size(); ++i) {
        objects[i]->update(frame);
        objects[i]->render(cam->viewMatrix(), proj_matrix);
    }
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
