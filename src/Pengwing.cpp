#include "common.hpp"
#include "Shader.h"
#include "Model.h"
#include "camera_orbital.hpp"
#include "Object.h"
#include "Drache.h"
#include "Camera.h"
#include "Postprocessing.h"
#include "Noise.h"
#include "Penguin.h"

#include "perlin.h"
#include "terrain.h"

#include <map>
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
const float FAR_VALUE  = 150.0f;
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

// Landschaft ------------------------------------------------------------
//Perlin noise 
int octaves = 7; // Number of overlapping perlin maps
float persistence = 0.5f; // Persistence --> Decrease in amplitude of octaves
float lacunarity = 2.0f; // Lacunarity  --> Increase in frequency of octaves
float noiseScale = 64.0f; //Scale of the obtained map

//unendliche Landschaft
int numChunksVisible = 1;
int terrainXChunks = 3;
int terrainYChunks = 3;

// für objekte
unsigned int amount = 0;
glm::mat4* modelMatrices;
std::map<std::vector<int>, bool> terrainChunkDict; //Keep track of whether terrain chunk at position or not
std::vector<std::vector<int>> lastViewedChunks; //Keep track of last viewed vectors
std::map<std::vector<int>, int> terrainPosVsChunkIndexDict; //Chunk position vs index in map_chunks


//Keep track of number of map chunks
int countChunks = 0;

void createPlane(std::vector<int>& position, int xOffset, int yOffset, int height, int width, float heightMultiplier, float mapScale, unsigned int& program, GLuint& plane_VAO);
void createWorldTerrain(int height, int width, float heightMultiplier, float mapScale, unsigned int& program, std::vector<GLuint>& map_chunks, int numChunksVisible, glm::vec3 cameraPos);
std::vector<float> generateNoiseMap(int offsetX, int offsetY, int chunkHeight, int chunkWidth);
void setupModelTransformation(unsigned int& program);

// ----------------------------------------------------------------------


// Camera ---------------------------------------------------------
//Camera cam;
//camera_orbital orbitalCam;
//// ----------------------------------------------------------------

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

    //map generation
    int mapHeight = 128; //Height of each chunk
    int mapWidth = 128; //Width of each chunk
    float heightMultiplier = 75.0f; //Scale for height of peak
    float mapScale = 1.0f; //Scale for height and breadth of each chunk
    std::vector<GLuint> map_chunks(terrainXChunks * terrainYChunks);

    GLFWwindow* window = initOpenGL(WINDOW_WIDTH, WINDOW_HEIGHT,"Pengwing");
    glfwSetFramebufferSizeCallback(window, resizeCallback);

    stbi_set_flip_vertically_on_load(true);

    if (enableGUI) init_imgui(window);

    //camera_orbital cam(window);
    proj_matrix = glm::perspective(FOV, float(WINDOW_WIDTH)/float(WINDOW_HEIGHT), NEAR_VALUE, FAR_VALUE);

    // Loading Objects ----------------------------------
    // --
    glm::mat4 scene_mat = glm::identity<glm::mat4>();

    std::vector<Object*> objects = std::vector<Object*>();
    Shader shadow_shader("ShadowMapping/shadow_mapping.vert", "ShadowMapping/shadow_mapping.frag");
    Shader shadow_shader_unicol("ShadowMapping/shadow_mapping.vert", "ShadowMapping/shadow_mapping_unicol.frag");
    //objects.push_back(new Drache(shadow_shader, Model("backpack/backpack.obj", true), &scene_mat, "Backpack"));
    //objects[0]->position = glm::vec3(0.0f, 0.0f, 0.0f);
    //objects[0]->active = false;
    //
    //objects.push_back(new Object(shadow_shader_unicol, Model("plane.obj", false), &scene_mat, "Plane"));
    //objects[1]->scale = glm::vec3(200.0f, 1.0f, 200.0f);
    //objects[1]->position = glm::vec3(0.0f, 0.0f, 0.0f);
    //objects[1]->active = true;
    //objects.push_back(new Drache(shadow_shader_unicol, Model("dragon.obj", true), &scene_mat, "Drache"));
    //objects[2]->active = false;
    //
    //objects.push_back(new Penguin(shadow_shader, Model("penguin/penguin.obj", true), &scene_mat, "Penguin"));
    //objects[3]->scale = glm::vec3(0.00001f);
    //objects[3]->position = glm::vec3(0.0f, 1.0f, 0.0f);
    //objects[3]->active = true;
    
    objects.push_back(new Object(shadow_shader_unicol, Model("plane.obj", false), &scene_mat, "Plane"));
    objects[0]->scale = glm::vec3(200.0f, 1.0f, 200.0f);
    objects[0]->position = glm::vec3(0.0f, 0.0f, 0.0f);
    objects[0]->active = true;
    objects.push_back(new Object(shadow_shader_unicol, Model("cannon/cannon.obj", false), &scene_mat, "Cannon"));
    objects[1]->scale = 2.0f * glm::vec3(1.0f, 1.0f, 1.0f);
    objects[1]->position = glm::vec3(12.0f, 2.0f, 6.0f);
    objects[1]->active = true;


    objects.push_back(new Penguin(shadow_shader_unicol, Model("penguin/penguinunwinged.obj", true), &scene_mat, "Penguin"));
    objects[2]->position = glm::vec3(0.0f, 1.0f, 0.0f);
    objects[2]->active = true;
    objects.push_back(new Object(shadow_shader_unicol, Model("penguin/penguinunwingedwing.obj", false), &objects[2]->model_matrix, "Penguin Right Arm"));
    objects[3]->scale = 0.8f * glm::vec3(1.0f, 1.0f, 1.0f);
    objects[3]->position = glm::vec3(0.3f, 1.0f, 0.0f);
    objects[3]->active = true;
    objects.push_back(new Object(shadow_shader_unicol, Model("penguin/penguinunwingedwing.obj", false), &objects[2]->model_matrix, "Penguin Left Arm"));
    objects[4]->scale = 0.8f * glm::vec3(1.0f, 1.0f, 1.0f);
    objects[4]->rotation = glm::vec4(0.0f, 1.0f, 0.0f, glm::pi<float>());
    objects[4]->position = glm::vec3(2.0f, 1.0f, 0.0f);
    objects[4]->active = true;

    //objects.push_back(new Object(shadow_shader_unicol, Model("plane.obj", false), &scene_mat, "Plane"));
    //objects[1]->scale = glm::vec3(200.0f, 1.0f, 200.0f);
    //objects[1]->position = glm::vec3(0.0f, 0.0f, 0.0f);
    //objects[1]->active = true;
    
    // Landschaft
    Shader landschaftShader = Shader("Landschaft/landschaft.vert", "Landschaft/landschaft.frag");
    setupModelTransformation(landschaftShader.ID);
    landschaftShader.setMat4("vProjection", proj_matrix);


    // Background ----------------------------------------------------
    Shader backgroundShader = Shader("simple.vert", "background.frag");
    float fogColor[3] = { 0.839f, 0.910f, 0.953f };
    float skyColor[3] = { 0.160f, 0.605f, 0.867f };
    // ---------------------------------------------------------------

    // Shadow mapping ------------------------------------------------
    const unsigned int SHADOW_WIDTH = 2048, SHADOW_HEIGHT = 2048;   // Default 1024
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

    float minShadowBias = 0.031f;
    float maxShadowBias = 0.051f;
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
    noise.generatePerlin(glm::uvec3(200, 128, 200));
    unsigned int perlinNoiseID = noise.getPerlinNoiseID();
    noise.generateWorley(glm::uvec3(200, 80, 200), 20);
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

    glm::vec3 cloud_boundsMin(-1000.0f, 50.0f, -1000.0f);
    glm::vec3 cloud_boundsMax( 1000.0f, 55.0f,  1000.0f);
    pp_clouds.setVec3("boundsMin", cloud_boundsMin);
    pp_clouds.setVec3("boundsMax", cloud_boundsMax);

    bool CloudActive = false;
    float CloudScale = 6.231f;
    float CloudOffset[3] = { 0.0f, 0.0f, 0.0f };
    float CloudOffsetSpeed[3] = { 0.017f, 0.007f, 0.001f };
    float DensityThreshold = 0.864f;
    float DensityMultiplier = 1.256f;
    float DarknessThreshold = 3.869f;
    float LightAbsorption = 2.06f;
    float PhaseVal = 0.814f;

    float CloudColor[3] = { 0.95f, 0.95f, 1.0f };
    // ----------------------------------------------------------------

    // Fog ------------------------------------------------------------
    //Shader atmosphere_fog("simple.vert", "atmosphere_fog.frag");
    //Shader pp_fog("simple.vert", "Postprocessing/postprocessing_fog.frag");
    // ----------------------------------------------------------------

    // Camera ---------------------------------------------------------
    Camera cam = Camera(&scene_mat, "Camera");
    camera_orbital orbitalCam = camera_orbital(window);
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
    //glEnable(GL_POLYGON_SMOOTH);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);
    // ----------------------------------------------------------------


    // Lighting -------------------------------------------------------
    float theta = 1.0f;
    float azimuth = 0.8f;
    glm::vec3 lightPos = glm::vec3(glm::sin(azimuth) * glm::cos(theta), glm::sin(azimuth) * glm::sin(theta), glm::cos(theta));
    glm::vec3 lightColor = glm::vec3(0.9f, 0.9f, 0.89f);
    glm::mat4 lightProjection, lightView;
    glm::mat4 lightSpaceMatrix;
    float near_plane = -20.0f, far_plane = 50.5f;
    lightProjection = glm::ortho<float>(-50.0f, 50.0f, -50.0f, 50.0f, near_plane, far_plane);
    lightView = glm::lookAt(lightPos, glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    lightSpaceMatrix = lightProjection * lightView;
    // ----------------------------------------------------------------

    glfwSetKeyCallback(window, key_callback);
    glfwMaximizeWindow(window);
    std::chrono::time_point<std::chrono::system_clock> start, end;
    // rendering loop
    while (glfwWindowShouldClose(window) == false) {
        start = std::chrono::system_clock::now();

        //lightPos.y = glm::sin(i_FRAME * 0.01f);



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


        // Render to depth---------------------------------------------
        glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
        glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT); // reset viewport
        glClear(GL_DEPTH_BUFFER_BIT);
        simpleDepthShader.use();
        simpleDepthShader.setMat4("lightSpaceMatrix", proj_matrix * (!useOrbital ? cam.viewMatrix() : orbitalCam.view_matrix()));
        render_scene_with_shader(objects, &simpleDepthShader, i_FRAME);
        // ------------------------------------------------------------


        glBindFramebuffer(GL_FRAMEBUFFER, postprocessing.framebufferA);
        //glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glClearColor(0.7f, 0.7f, 0.9f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Background Rendering ---------------------------------------
        //glDisable(GL_DEPTH_TEST);
        //glActiveTexture(GL_TEXTURE1);
        //glBindTexture(GL_TEXTURE_2D, background);
        //backgroundDraw.use();
        //backgroundDraw.setInt("backgroundTexture", 1);
        //renderQuad();
        //glEnable(GL_DEPTH_TEST);
        glDisable(GL_DEPTH_TEST);
        backgroundShader.use();
        backgroundShader.setVec2("uRes", glm::vec2(WINDOW_WIDTH, WINDOW_HEIGHT));
        backgroundShader.setMat4("view_mat", !useOrbital ? cam.viewMatrix() : orbitalCam.view_matrix());
        backgroundShader.setMat4("proj_mat", proj_matrix);
        backgroundShader.setVec3("viewPos", !useOrbital ? cam.position : orbitalCam.position());
        backgroundShader.setVec3("sunPos", lightPos);
        backgroundShader.setVec3("sunColor", lightColor);

        backgroundShader.setVec3("skyColor", skyColor[0], skyColor[1], skyColor[2]);
        backgroundShader.setVec3("fogColor", fogColor[0], fogColor[1], fogColor[2]);
        renderQuad();
        glEnable(GL_DEPTH_TEST);
        // ------------------------------------------------------------

        // Second pass
        // set light uniforms
        glActiveTexture(GL_TEXTURE8);
        glBindTexture(GL_TEXTURE_2D, shadowDepthMap);
        shadow_shader.use();
        shadow_shader.setVec3("viewPos", !useOrbital ? cam.position : orbitalCam.position());
        shadow_shader.setFloat("minBias", minShadowBias);
        shadow_shader.setFloat("maxBias", maxShadowBias);
        shadow_shader.setVec3("lightColor", lightColor);
        shadow_shader.setVec3("lightPos", lightPos);
        shadow_shader.setMat4("lightSpaceMatrix", lightSpaceMatrix);
        shadow_shader.setVec3("fogColor", fogColor[0], fogColor[1], fogColor[2]);
        shadow_shader.setFloat("near", NEAR_VALUE);
        shadow_shader.setFloat("far", FAR_VALUE);
        shadow_shader.setInt("shadowMap", 8);
        // set light uniforms
        shadow_shader_unicol.use();
        shadow_shader_unicol.setVec3("viewPos", !useOrbital ? cam.position : orbitalCam.position());
        shadow_shader_unicol.setVec3("lightPos", lightPos);
        shadow_shader_unicol.setMat4("lightSpaceMatrix", lightSpaceMatrix);
        shadow_shader_unicol.setFloat("minBias", minShadowBias);
        shadow_shader_unicol.setFloat("maxBias", maxShadowBias);
        shadow_shader_unicol.setVec3("lightColor", lightColor);
        shadow_shader_unicol.setVec3("fogColor", fogColor[0], fogColor[1], fogColor[2]);
        shadow_shader_unicol.setFloat("near", NEAR_VALUE);
        shadow_shader_unicol.setFloat("far", FAR_VALUE);
        shadow_shader_unicol.setInt("shadowMap", 8);

        //render_scene_with_shader(objects, &simpleDepthShader, i_FRAME);
        if (!useOrbital) render_scene(objects, &cam, i_FRAME);
        else render_scene(objects, &orbitalCam, i_FRAME);

        createWorldTerrain(mapHeight, mapWidth, heightMultiplier, mapScale, landschaftShader.ID, map_chunks, numChunksVisible, !useOrbital ? cam.position : orbitalCam.position());
        landschaftShader.use();
        landschaftShader.setMat4("vView", !useOrbital ? cam.viewMatrix() : orbitalCam.view_matrix());
        landschaftShader.setVec3("camPosition", !useOrbital ? cam.position : orbitalCam.position());
        landschaftShader.use();
        for (int i = 0; i < terrainXChunks * terrainYChunks; i++) {
            //std::cout << i << " " << map_chunks[i] << std::endl;
            glBindVertexArray(map_chunks[i]);
            glDrawArrays(GL_TRIANGLES, 0, (mapWidth - 1) * (mapHeight - 1) * 6);
        }


        // Third pass -> render framebuffer A to screen buffer
        // ----------------------------------------------------------------------
        // Postprocess pass
        if (CloudActive)
        {
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
            pp_clouds.setVec3("CloudColor", CloudColor[0], CloudColor[1], CloudColor[2]);

            pp_clouds.setVec3("skyColor", skyColor[0], skyColor[1], skyColor[2]);

            pp_clouds.setVec3("lightPos", lightPos);
            pp_clouds.setFloat("near", NEAR_VALUE);
            pp_clouds.setFloat("far", FAR_VALUE);
            pp_clouds.setVec3("viewPos", !useOrbital ? cam.position : orbitalCam.position());
            pp_clouds.setMat4("view_mat", !useOrbital ? cam.viewMatrix() : orbitalCam.view_matrix());
            pp_clouds.setInt("frame", i_FRAME);
        }
        else {
            defaultPP.use();
        }
        postprocessing.postprocess(RenderDirection::A_TO_SCR);

        //pp_fog.use();
        //
        //glActiveTexture(GL_TEXTURE1);
        //glBindTexture(GL_TEXTURE_2D, depthMap);
        //
        //pp_fog.setInt("screenTexture", 0);
        //pp_fog.setInt("depthTexture", 1);
        //pp_fog.setFloat("near", NEAR_VALUE);
        //pp_fog.setFloat("far", FAR_VALUE);
        //pp_fog.setVec3("fogColor", fogColor[0], fogColor[1], fogColor[2]);
        //postprocessing.postprocess(RenderDirection::B_TO_SCR);


        // render Depth map to quad for visual debugging
        // ---------------------------------------------
        //glActiveTexture(GL_TEXTURE0);
        //glBindTexture(GL_TEXTURE_2D, depthMap);
        //debugDepthQuad.use();
        //renderQuad();

        if (enableGUI) {
            handleGUI(objects);
            // Cloud settings
            ImGui::Begin("Cloud Settings");
            ImGui::Checkbox("Active", &CloudActive);
            ImGui::SliderFloat("Cloud Scale", &CloudScale, 0.0f, 20.0f);
            ImGui::SliderFloat3("Cloud Offset", &CloudOffsetSpeed[0], -0.05f, 0.05f);
            ImGui::SliderFloat("Density Threshold", &DensityThreshold, 0.0f, 2.0f);
            ImGui::SliderFloat("Density Multiplier", &DensityMultiplier, 0.0f, 10.0f);
            ImGui::SliderFloat("Darkness Threshold", &DarknessThreshold, 0.0f, 10.0f);
            ImGui::SliderFloat("Light Absorption", &LightAbsorption, 0.0f, 10.0f);
            ImGui::SliderFloat("Phase Val", &PhaseVal, 0.0f, 3.0f);
            ImGui::ColorPicker3("Cloud Color", &CloudColor[0]);
            ImGui::End();

            ImGui::Begin("Background Settings");
            ImGui::ColorPicker3("Sky Color", &skyColor[0]);
            ImGui::ColorPicker3("Fog Color", &fogColor[0]);
            ImGui::End();

            ImGui::Begin("Shadow Settings");
            ImGui::SliderFloat("Minimum Shadow Bias", &minShadowBias, 0.00f, 3.0f);
            ImGui::SliderFloat("Maximum Shadow Bias", &maxShadowBias, 0.00f, 5.0f);
            ImGui::End();
        }

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
        glfwPollEvents();
        
        // FPS limiting
        end = std::chrono::system_clock::now();
        std::chrono::duration<double, std::milli> elapsed_milliseconds = end - start;
        Sleep(glm::max(0.0, 1000.0 / double(FPS) - elapsed_milliseconds.count()));
    }

    for (unsigned int i = 0; i < objects.size(); ++i) {
        objects[i]->destroy();
    }
    for (int i = 0; i < map_chunks.size(); i++) {
        glDeleteVertexArrays(1, &map_chunks[i]);
    }
    postprocessing.destroy();

    cleanup_imgui();
    glfwTerminate();
}
// Landschaft -------------------------------------
//erstellt noise map mit perlin noise
std::vector<float> generateNoiseMap(int offsetX, int offsetY, int chunkHeight, int chunkWidth) {
    std::vector<float> noiseValues;
    std::vector<float> normalizedNoiseValues;

    float amp = 1;
    float freq = 1;
    float maxPossibleHeight = 0;

    for (int i = 0; i < octaves; i++) {
        maxPossibleHeight += amp;
        amp *= persistence;
    }

    for (int y = 0; y < chunkHeight; y++) {
        for (int x = 0; x < chunkWidth; x++) {
            amp = 1;
            freq = 1;
            float noiseHeight = 0;
            for (int i = 0; i < octaves; i++) {
                float xSample = (x + (offsetX * (chunkWidth - 1)) - (float)((chunkWidth - 1) / 2)) / noiseScale * freq;
                float ySample = (-y + (offsetY * (chunkHeight - 1)) + (float)((chunkHeight - 1) / 2)) / noiseScale * freq;

                float perlinValue = getPerlinNoise(xSample, ySample);
                noiseHeight += perlinValue * amp;

                // Lacunarity erhöht die Fräquenz der Oktaven
                amp *= persistence;
                // Persistence senkt die Amplitude der Oktaven
                freq *= lacunarity;
            }

            noiseValues.push_back(noiseHeight);
        }
    }

    for (int y = 0; y < chunkHeight; y++) {
        for (int x = 0; x < chunkWidth; x++) {
            // Inverse lerp and scale values to range from 0 to 1
            float normalVal = (noiseValues[x + y * chunkWidth] + 1) / maxPossibleHeight;
            if (normalVal < 0) {
                normalVal = 0;
            }
            normalizedNoiseValues.push_back(normalVal);
        }
    }

    return normalizedNoiseValues;
}


//erstellt Mesh für einen "chunk"
void createPlane(std::vector<int>& position, int xOffset, int yOffset, int height, int width, float heightMultiplier, float mapScale, unsigned int& program, GLuint& plane_VAO) //Check if offsets required
{

    //generiert height map mit perlin noise
    std::vector<float> noiseMap = generateNoiseMap(position[0], position[1], height, width);

    glUseProgram(program);

    terrain* currTerrain = new terrain(position, height, width, heightMultiplier, mapScale, noiseMap);

    //Generate VAO object
    glGenVertexArrays(1, &plane_VAO);
    glBindVertexArray(plane_VAO);

    //Create VBOs for the VAO
    //Position information (data + format)
    int nVertices = currTerrain->getTriangleVerticesCount(height, width);
    GLfloat* expanded_vertices = currTerrain->getTrianglePoints();

    GLuint vertex_VBO;
    glGenBuffers(1, &vertex_VBO);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_VBO);
    glBufferData(GL_ARRAY_BUFFER, nVertices * 3 * sizeof(GLfloat), expanded_vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);


    GLfloat* expanded_normals = currTerrain->getNormals(height, width);

    GLuint normal_VBO;
    glGenBuffers(1, &normal_VBO);
    glBindBuffer(GL_ARRAY_BUFFER, normal_VBO);
    glBufferData(GL_ARRAY_BUFFER, nVertices * 3 * sizeof(GLfloat), expanded_normals, GL_STATIC_DRAW);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(2);

    // delete []expanded_normals;
    delete[]expanded_vertices;


    GLfloat* expanded_vertices2 = currTerrain->getTrianglePoints();
    GLfloat* expanded_colors = new GLfloat[nVertices * 3];

    modelMatrices = new glm::mat4[amount];
    // initialisiere random seed
    srand(glfwGetTime());
    float radius = 100.0;
    float offset = 20.0f;
    for (unsigned int i = 0; i < amount; i++)
    {
        glm::mat4 model = glm::mat4(1.0f);
        // 1. translation: displace along circle with 'radius' in range [-offset, offset]
        float angle = (float)i / (float)amount * 360.0f;
        float displacement = (rand() % (int)(2 * offset * 100)) / 100.0f - offset;
        float x = sin(angle) * radius + displacement;
        displacement = (rand() % (int)(2 * offset * 100)) / 100.0f - offset;
        float y = displacement * 0.5f; // keep height of asteroid field smaller compared to width of x and z
        displacement = (rand() % (int)(2 * offset * 100)) / 100.0f - offset;
        float z = cos(angle) * radius + displacement;
        model = glm::translate(model, glm::vec3(x, y, z));

        // 2. scale: Scale between 0.05 and 0.25f
        float scale = (rand() % 20) / 100.0f + 1;
        model = glm::scale(model, glm::vec3(scale));

        // 3. rotation: add random rotation around a (semi)randomly picked rotation axis vector
//        float rotAngle = (rand() % 360);
//        model = glm::rotate(model, rotAngle, glm::vec3(0.4f, 0.6f, 0.8f));

        // 4. now add to list of matrices
        modelMatrices[i] = model;
    }

    //Creating different terrain types
    //Height is the upper threshold
    terrainType snowTerrain1 = terrainType("snow1", heightMultiplier * 1.0f, 0.9, 0.95, 0.92); //white
    terrainType snowTerrain2 = terrainType("snow2", heightMultiplier * 0.7f, 0.9, 0.91, 0.92); //white a bit different
    terrainType snowTerrain3 = terrainType("snow3", heightMultiplier * 0.12f, 0.9, 0.93, 0.93); //white a bit more different
    terrainType snowTerrain4 = terrainType("snow4", heightMultiplier * 0.08f, 0.9, 0.95, 0.99); //white even a bit more different
    terrainType waterTerrain = terrainType("water", heightMultiplier * 0.04f, 0.0, 0.50, 0.81); //Blue

    int terrainCount = 5;
    std::vector<terrainType> terrainArr;
    terrainArr.push_back(waterTerrain);
    terrainArr.push_back(snowTerrain4);
    terrainArr.push_back(snowTerrain3);
    terrainArr.push_back(snowTerrain2);
    terrainArr.push_back(snowTerrain1);

    for (int i = 0; i < nVertices; i++) {

        //Assign color according to height
        float currHeight = expanded_vertices2[i * 3 + 1];
        for (int j = 0; j < terrainCount; j++) {
            if (currHeight <= terrainArr[j].height) {
                // expanded_colors[i*3] = lerp(currHeight/heightMultiplier, 0.3, 1);
                // expanded_colors[i*3+1] = lerp(currHeight/heightMultiplier, 0.3, 1);
                // expanded_colors[i*3+2] = lerp(currHeight/heightMultiplier, 0.3, 1);
                expanded_colors[i * 3] = terrainArr[j].red;
                expanded_colors[i * 3 + 1] = terrainArr[j].green;
                expanded_colors[i * 3 + 2] = terrainArr[j].blue;
                break;
            }
        }

    }

    GLuint color_VBO;
    glGenBuffers(1, &color_VBO);
    glBindBuffer(GL_ARRAY_BUFFER, color_VBO);
    glBufferData(GL_ARRAY_BUFFER, nVertices * 3 * sizeof(GLfloat), expanded_colors, GL_STATIC_DRAW);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(1);
    delete[]expanded_colors;
    delete[]expanded_vertices2;

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    //glBindVertexArray(0); //Unbind the VAO to disable changes outside this function.
}

//erstelle "Chunks" und verbinde sie um ein endloses Terrain zu haben
void createWorldTerrain(int mapHeight, int mapWidth, float heightMultiplier, float mapScale, unsigned int& program, std::vector<GLuint>& map_chunks, int numChunksVisible, glm::vec3 cameraPos) {

    int chunkHeight = mapHeight;
    int chunkWidth = mapWidth;

    //sicher gehen dass man die richtigen x und y Koord. nimmt
    int currChunkCoordX = (int)round(cameraPos.x / chunkWidth); //nimm aktuelle x-chunk Koordinate
    int currChunkCoordY = (int)round(cameraPos.z / chunkHeight); //nimm atkuelle y-chunk Koordinate (oder z abhängig von unserem Koord.Sys)


    //prüfen welche "chunks" nicht mehr im Sichtfeld sind
    std::vector<std::vector<int>> chunksOutOfView;
    if (lastViewedChunks.empty() == false) {
        for (std::vector<int> v : lastViewedChunks) {
            if (abs(currChunkCoordX - v[0]) <= numChunksVisible && abs(currChunkCoordY - v[1]) <= numChunksVisible) {
                //Chunks im render view
            }
            else {
                //chunks außerhalb des render view
                chunksOutOfView.push_back(v);
                std::cout << "chunks out of view size " << chunksOutOfView.size() << std::endl;
            }
        }
    }

    //Clear the last viewedChunks
    lastViewedChunks.clear();

    for (int yOffset = -numChunksVisible; yOffset <= numChunksVisible; yOffset++) {
        for (int xOffset = -numChunksVisible; xOffset <= numChunksVisible; xOffset++) {

            std::vector<int> viewedChunkCoord(2);
            viewedChunkCoord[0] = currChunkCoordX + xOffset;
            viewedChunkCoord[1] = currChunkCoordY + yOffset;
            //terrain chunk der schon da ist
            if (terrainChunkDict.count(viewedChunkCoord) > 0) {


            }
            //terrain chunk der nicht schon da ist
            else {

                //prüfe in map_chunks ob Ersatz gebraucht wird
                if (countChunks >= terrainXChunks * terrainYChunks) {
                    terrainChunkDict[viewedChunkCoord] = true;
                    std::vector<int> replaceVector = chunksOutOfView[0];
                    std::cout << "Replace vector is " << replaceVector[0] << " " << replaceVector[1] << std::endl;
                    int map_index = terrainPosVsChunkIndexDict[replaceVector];
                    std::cout << "map index is " << map_index << std::endl;
                    createPlane(viewedChunkCoord, xOffset, yOffset, mapHeight, mapWidth, heightMultiplier, mapScale, program, map_chunks[map_index]);
                    terrainPosVsChunkIndexDict[viewedChunkCoord] = map_index;
                    chunksOutOfView.erase(chunksOutOfView.begin());
                    terrainChunkDict.erase(replaceVector);
                    std::cout << "Plane created at " << viewedChunkCoord[0] << " " << viewedChunkCoord[1] << std::endl;
                }

                else {
                    //kein Ersatz wird gebraucht
                    terrainChunkDict[viewedChunkCoord] = true;
                    terrainPosVsChunkIndexDict[viewedChunkCoord] = (xOffset + numChunksVisible) + (yOffset + numChunksVisible) * terrainXChunks;
                    std::cout << "Initial map index is " << (xOffset + numChunksVisible) + (yOffset + numChunksVisible) * terrainXChunks << std::endl;
                    createPlane(viewedChunkCoord, xOffset, yOffset, mapHeight, mapWidth, heightMultiplier, mapScale, program, map_chunks[(xOffset + numChunksVisible) + (yOffset + numChunksVisible) * terrainXChunks]);
                    std::cout << "Initial Plane created at " << viewedChunkCoord[0] << " " << viewedChunkCoord[1] << std::endl;

                }

                for (int x : map_chunks) {
                    std::cout << "map_chunk " << x << std::endl;
                }

                for (std::vector<int> v : lastViewedChunks) {
                    std::cout << v[0] << " " << v[1] << " - " << terrainPosVsChunkIndexDict[v] << std::endl;
                }

                countChunks++;

            }

            lastViewedChunks.push_back(viewedChunkCoord);


        }
    }

}
void setupModelTransformation(unsigned int& program)
{
    //Modelling transformations (Model -> World coordinates)
    glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0, 0.0, 0.0));//Model coordinates are the world coordinates


    glm::vec3 point_a = glm::vec3(1.0, 2.0, 2.0);
    glm::vec3 vec_w = glm::normalize(point_a);

    glm::vec3 point_t = glm::vec3(vec_w.y, vec_w.x, vec_w.y);
    glm::vec3 vec_u = glm::normalize(glm::cross(point_t, vec_w));

    glm::vec3 vec_v = glm::cross(vec_w, vec_u);

    glm::mat4 matr_uvw = glm::mat4(
        vec_u.x, vec_u.y, vec_u.z, 0.0f,
        vec_v.x, vec_v.y, vec_v.z, 0.0f,
        vec_w.x, vec_w.y, vec_w.z, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    );

    double degrees = 30;

    glm::mat4 matr_rot = glm::mat4(
        cos(glm::radians(degrees)), sin(glm::radians(degrees)), 0.0f, 0.0f,
        -sin(glm::radians(degrees)), cos(glm::radians(degrees)), 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    );

    glm::mat4 matr_trans = glm::transpose(matr_uvw);

    model = matr_uvw * matr_rot * matr_trans;
    //Model coordinates are the world coordinates
    model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0, 0.0, 0.0));

    //Pass on the modelling matrix to the vertex shader
    glUseProgram(program);
    int vModel_uniform = glGetUniformLocation(program, "vModel");
    if (vModel_uniform == -1) {
        fprintf(stderr, "Could not bind location: vModel\n");
        exit(0);
    }
    glUniformMatrix4fv(vModel_uniform, 1, GL_FALSE, glm::value_ptr(model));
}
// ------------------------------------------------

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
