#include "common.hpp"
#include "Shader.h"
#include "Model.h"
#include "camera_orbital.hpp"
#include "Object.h"
#include "Drache.h"
#include "Camera.h"
#include "Postprocessing.h"
#include "Noise.h"

#include <string>
#include <chrono>
#include <imgui.hpp>

// Window Settings
const int WINDOW_WIDTH       = 1920;
const int WINDOW_HEIGHT      = 1080;
constexpr float ASPECT_RATIO = static_cast<float>(WINDOW_WIDTH) / static_cast<float>(WINDOW_HEIGHT);

// Camera Settings
const float FOV        = 45.0f;
const float NEAR_VALUE = 0.1f;
const float FAR_VALUE  = 100.0f;
bool useOrbital        = true;

// GUI Settings
bool enableGUI            = true;
const int timeline_height = 200;

// Timeline Settings
const int FPS        = 60;
const float duration = 30;
int i_FRAME          = 0;
int loop_start       = 0;
int loop_end         = int(FPS * duration);
bool play            = true;

// Forward Declaration
void handleGUI(std::vector<Object*> objects);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
void render_scene(std::vector<Object*> objects, Camera* cam, unsigned int frame);
void render_scene(std::vector<Object*> objects, camera_orbital* orbitalCam, unsigned int frame);
void render_scene_with_shader(std::vector<Object*> objects, Shader* shader, unsigned int frame);
void renderQuad();
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
    proj_matrix = glm::perspective(FOV, 1.0f, NEAR_VALUE, FAR_VALUE);

    // Loading Objects ----------------------------------
    // --
    glm::mat4 scene_mat = glm::identity<glm::mat4>();

    std::vector<Object*> objects = std::vector<Object*>();
    Shader shadow_shader("ShadowMapping/shadow_mapping.vert", "ShadowMapping/shadow_mapping.frag");
    objects.push_back(new Drache(shadow_shader, Model("backpack/backpack.obj", true), &scene_mat, "Backpack"));
    objects[0]->position = glm::vec3(0.0f, 0.0f, 0.0f);
    objects[0]->active = false;

    Shader shadow_shader_unicol("ShadowMapping/shadow_mapping.vert", "ShadowMapping/shadow_mapping_unicol.frag");
    objects.push_back(new Object(shadow_shader_unicol, Model("plane.obj", false), &scene_mat, "Plane"));
    objects[1]->scale = glm::vec3(50.0f, 1.0f, 50.0f);
    objects[1]->position = glm::vec3(0.0f, 0.0f, 0.0f);
    objects[1]->active = false;
    objects.push_back(new Drache(shadow_shader_unicol, Model("dragon.obj", true), &scene_mat, "Drache"));
    objects[2]->active = true;

    /*
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
    objects[2]->position = glm::vec3(0.0f, 0.0f, 0.0f);
    objects[2]->active = false;

    objects.push_back(new Drache(albedo_texture, Model("backpack/backpack.obj", true), &scene_mat, "Backpack"));
    objects[3]->position = glm::vec3(0.0f, 0.0f, 0.0f);
    objects[3]->active = true;
    */
    // ---------------------------------------------------
    
    // Shadow mapping ------------------------------------------------
    const unsigned int SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;
    unsigned int shadowDepthMapFBO;
    glGenFramebuffers(1, &shadowDepthMapFBO);

    unsigned int shadowDepthMap;
    glGenTextures(1, &shadowDepthMap);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, shadowDepthMap);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
        SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
    glBindTexture(GL_TEXTURE_2D, 0);
    
    glBindFramebuffer(GL_FRAMEBUFFER, shadowDepthMapFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, shadowDepthMap, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    Shader debugDepthQuad("simple.vert", "ShadowMapping/depth_debug_shader.frag");
    Shader simpleDepthShader("ShadowMapping/depth_shader.vert", "empty.frag");
    debugDepthQuad.use();
    debugDepthQuad.setInt("depthMap", 0);
    // ---------------------------------------------------------------

    // Depth map -----------------------------------------------------
    unsigned int depthMapFBO;
    glGenFramebuffers(1, &depthMapFBO);

    unsigned int depthMap;
    glGenTextures(1, &depthMap);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, depthMap);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
        WINDOW_WIDTH, WINDOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_NONE);
    glBindTexture(GL_TEXTURE_2D, 0);

    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    // ---------------------------------------------------------------

    Noise noise = Noise();
    noise.generatePerlin(glm::uvec3(256, 128, 256));
    unsigned int perlinNoiseID = noise.getPerlinNoiseID();
    noise.generateWorley(glm::uvec3(256, 128, 256), 10);
    unsigned int worleyNoiseID = noise.getWorleyNoiseID();

    // Postprocessing ------------------------------------------------
    Postprocessing postprocessing(WINDOW_WIDTH, WINDOW_HEIGHT);
    Shader defaultPP("simple.vert", "Postprocessing/postprocessing_basic.frag");
    defaultPP.use();
    defaultPP.setInt("screenTexture", 0);
    // ---------------------------------------------------------------

    // Clouds ---------------------------------------------------------
    Shader pp_clouds("clouds.vert", "clouds.frag");
    pp_clouds.use();
    pp_clouds.setInt("screenTexture", 0);
    pp_clouds.setInt("depthTexture", 1);
    pp_clouds.setMat4("proj_mat", proj_matrix);
    pp_clouds.setFloat("near", NEAR_VALUE);
    pp_clouds.setFloat("far", FAR_VALUE);
    pp_clouds.setVec2("uRes", glm::vec2(WINDOW_WIDTH, WINDOW_HEIGHT));

    glm::vec3 cloud_boundsMin(-100.0f, 20.0f, -100.0f);
    glm::vec3 cloud_boundsMax( 100.0f, 25.0f,  100.0f);
    pp_clouds.setVec3("boundsMin", cloud_boundsMin);
    pp_clouds.setVec3("boundsMax", cloud_boundsMax);

    // Settings
    float CloudScale = 2.0f;
    float CloudOffset[3] = { 0.0f, 0.0f, 0.0f };
    float CloudOffsetSpeed[3] = { 0.0f, 0.0f, 0.0f };
    float DensityThreshold = 0.2f;
    float DensityMultiplier = 0.53f;
    float DarknessThreshold = 0.5f;
    float LightAbsorption = 0.5f;
    float PhaseVal = 1.0f;
    // ----------------------------------------------------------------

    // Camera ---------------------------------------------------------
    Camera cam(&scene_mat, "Camera");
    camera_orbital orbitalCam(window);
    // ----------------------------------------------------------------



    // Depth Testing --------------------------------------------------
    glEnable(GL_DEPTH_TEST);
    // ----------------------------------------------------------------

    // Blending -------------------------------------------------------
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    // ----------------------------------------------------------------

    // Anti-Aliasing --------------------------------------------------
    glEnable(GL_MULTISAMPLE);
    glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);
    // ----------------------------------------------------------------



    // Lighting -------------------------------------------------------
    glm::vec3 lightPos(glm::normalize(glm::vec3(3.0f, 3.0f, 3.0f)));
    glm::mat4 lightProjection, lightView;
    glm::mat4 lightSpaceMatrix;
    float near_plane = -10.0f, far_plane = 20.5f;
    lightProjection = glm::ortho<float>(-20.0, 20.0, -20.0, 20.0, near_plane, far_plane);
    lightView = glm::lookAt(lightPos, glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    lightSpaceMatrix = lightProjection * lightView;
    // ----------------------------------------------------------------

    glfwSetKeyCallback(window, key_callback);
    glfwMaximizeWindow(window);
    std::chrono::time_point<std::chrono::system_clock> start, end;
    // rendering loop
    while (glfwWindowShouldClose(window) == false) {
        start = std::chrono::system_clock::now();
        glfwPollEvents();

        if (!useOrbital) cam.update(i_FRAME);
        // First pass --> render to shadow-----------------------------
        glBindFramebuffer(GL_FRAMEBUFFER, shadowDepthMapFBO);
        glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
        glClear(GL_DEPTH_BUFFER_BIT);
        glDisable(GL_CULL_FACE);
        simpleDepthShader.use();
        simpleDepthShader.setMat4("lightSpaceMatrix", lightSpaceMatrix);
        render_scene_with_shader(objects, &simpleDepthShader, i_FRAME);
        glEnable(GL_CULL_FACE);
        // ------------------------------------------------------------


        // Render to depth-----------------------------
        glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
        glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT); // reset viewport
        glClear(GL_DEPTH_BUFFER_BIT);
        simpleDepthShader.use();
        simpleDepthShader.setMat4("lightSpaceMatrix", proj_matrix * (!useOrbital ? cam.viewMatrix() : orbitalCam.view_matrix()));
        render_scene_with_shader(objects, &simpleDepthShader, i_FRAME);
        // ------------------------------------------------------------

        glBindFramebuffer(GL_FRAMEBUFFER, postprocessing.framebufferA);
        //glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glEnable(GL_DEPTH_TEST);
        // Second pass
        // set light uniforms
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, shadowDepthMap);
        shadow_shader.use();
        shadow_shader.setVec3("viewPos", !useOrbital ? cam.position : orbitalCam.position());
        shadow_shader.setVec3("lightPos", lightPos);
        shadow_shader.setMat4("lightSpaceMatrix", lightSpaceMatrix);
        shadow_shader.setInt("shadowMap", 8);
        // set light uniforms
        shadow_shader_unicol.use();
        shadow_shader_unicol.setVec3("viewPos", !useOrbital ? cam.position : orbitalCam.position());
        shadow_shader_unicol.setVec3("lightPos", lightPos);
        shadow_shader_unicol.setMat4("lightSpaceMatrix", lightSpaceMatrix);
        shadow_shader_unicol.setInt("shadowMap", 8);

        //render_scene_with_shader(objects, &simpleDepthShader, i_FRAME);
        if (!useOrbital) render_scene(objects, &cam, i_FRAME);
        else render_scene(objects, &orbitalCam, i_FRAME);
        
        // Third pass -> render framebuffer A to screen buffer
        // ----------------------------------------------------------------------
        // Postprocess pass
        pp_clouds.use();

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, depthMap);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_3D, perlinNoiseID);
        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_3D, worleyNoiseID);


        pp_clouds.setInt("screenTexture", 0);
        pp_clouds.setInt("depthTexture", 1);
        pp_clouds.setInt("perlinNoise", 2);
        pp_clouds.setInt("worleyNoise", 3);

        pp_clouds.setVec3("CloudOffset", CloudOffset[0], CloudOffset[1], CloudOffset[2]);
        pp_clouds.setFloat("CloudScale", CloudScale);
        pp_clouds.setFloat("DensityThreshold", DensityThreshold);
        pp_clouds.setFloat("DensityMultiplier", DensityMultiplier);
        pp_clouds.setFloat("DarknessThreshold", DarknessThreshold);
        pp_clouds.setFloat("LightAbsorption", LightAbsorption);
        pp_clouds.setFloat("PhaseVal", PhaseVal);


        pp_clouds.setVec3("lightPos", lightPos);
        pp_clouds.setFloat("near", NEAR_VALUE);
        pp_clouds.setFloat("far", FAR_VALUE);
        pp_clouds.setVec3("viewPos", !useOrbital ? cam.position : orbitalCam.position());
        pp_clouds.setMat4("view_mat", !useOrbital ? cam.viewMatrix() : orbitalCam.view_matrix());
        pp_clouds.setInt("frame", i_FRAME);
        postprocessing.postprocess(RenderDirection::A_TO_SCR);

        // render Depth map to quad for visual debugging
        // ---------------------------------------------
        //glActiveTexture(GL_TEXTURE0);
        //glBindTexture(GL_TEXTURE_2D, depthMap);
        //debugDepthQuad.use();
        //renderQuad();

        if (enableGUI) handleGUI(objects);
        // Cloud settings
        ImGui::Begin("Cloud Settings");
        ImGui::SliderFloat("Cloud Scale", &CloudScale, 0.0f, 20.0f);
        ImGui::SliderFloat3("Cloud Offset", &CloudOffsetSpeed[0], -0.05f, 0.05f);
        ImGui::SliderFloat("Density Threshold", &DensityThreshold, 0.0f, 2.0f);
        ImGui::SliderFloat("Density Multiplier", &DensityMultiplier, 0.0f, 10.0f);
        ImGui::SliderFloat("Darkness Threshold", &DarknessThreshold, 0.0f, 10.0f);
        ImGui::SliderFloat("Light Absorption", &LightAbsorption, 0.0f, 10.0f);
        ImGui::SliderFloat("Phase Val", &PhaseVal, 0.0f, 3.0f);
        ImGui::End();

        CloudOffset[0] += CloudOffsetSpeed[0];
        CloudOffset[1] += CloudOffsetSpeed[1];
        CloudOffset[2] += CloudOffsetSpeed[2];

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

unsigned int quadVAO = 0;
unsigned int quadVBO;
void renderQuad()
{
    if (quadVAO == 0)
    {
        float quadVertices[] = {
            // positions        // texture Coords
            -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
            -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
             1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
             1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
        };
        // setup plane VAO
        glGenVertexArrays(1, &quadVAO);
        glGenBuffers(1, &quadVBO);
        glBindVertexArray(quadVAO);
        glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    }
    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
}

void render_scene_with_shader(std::vector<Object*> objects, Shader* shader,  unsigned int frame)
{
    for (unsigned i = 0; i < objects.size(); ++i) {
        objects[i]->update(frame);
        shader->use();
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
void render_scene(std::vector<Object*> objects, camera_orbital* orbitalCam, unsigned int frame)
{
    // Render and Update Objects
    for (unsigned i = 0; i < objects.size(); ++i) {
        objects[i]->update(frame);
        objects[i]->render(orbitalCam->view_matrix(), proj_matrix);
    }
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_G && action == GLFW_PRESS)
        enableGUI = !enableGUI;
    if (key == GLFW_KEY_SPACE && action == GLFW_PRESS)
        play = !play;
    if (key == GLFW_KEY_O && action == GLFW_PRESS)
        useOrbital = !useOrbital;
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
