#include "common.hpp"
#include "shader.hpp"
#include "buffer.hpp"
#include "mesh.hpp"
#include "helper.hpp"
#include <iostream>
#include <chrono>

const int WINDOW_WIDTH = 1280;
const int WINDOW_HEIGHT = 720;

std::chrono::time_point<std::chrono::system_clock> start_time;

float getTimeDelta();

glm::uvec2 uRes;

glm::vec4 color;
glm::vec3 z;
std::vector<geometry> objects;
geometry object;

void
resizeCallback(GLFWwindow* window, int width, int height);

// called whenever the window gets resized
void
resizeCallback(GLFWwindow* window, int width, int height);

int
main(int, char* argv[]) {
    GLFWwindow* window = initOpenGL(WINDOW_WIDTH, WINDOW_HEIGHT, argv[0]);
    glfwSetFramebufferSizeCallback(window, resizeCallback);

    // Initiate uniforms
    uRes = glm::vec2(1.0f * WINDOW_WIDTH, 1.0f * WINDOW_HEIGHT);

    // load and compile shaders and link program
    unsigned int vertexShader = compileShader("raycast.vert", GL_VERTEX_SHADER);
    unsigned int fragmentShader = compileShader("raymarch.frag", GL_FRAGMENT_SHADER);
    unsigned int shaderProgram = linkProgram(vertexShader, fragmentShader);
    // after linking the program the shader objects are no longer needed
    glDeleteShader(fragmentShader);
    glDeleteShader(vertexShader);

	// Load object and calculate center
    color = glm::vec4(0.1f, 0.5f, 0.1f, 1.f);
    objects = loadScene("sphere.obj", true, color);
    object = objects[0];
    z = glm::vec3(0.);
    for (unsigned int f = 0; f < object.positions.size(); ++f) {
        z.x += object.positions[f].x;
        z.y += object.positions[f].y;
        z.z += object.positions[f].z;
    }
    z /= float(object.positions.size());

	// Define uniform variables
    glUseProgram(shaderProgram);
    int res = glGetUniformLocation(shaderProgram, "uRes");
    int time = glGetUniformLocation(shaderProgram, "uTime");
    int positions = glGetUniformLocation(shaderProgram, "uPositions");
    int center = glGetUniformLocation(shaderProgram, "uCenter");
    int faces = glGetUniformLocation(shaderProgram, "uFaces");

    // vertex data
    float vertices[] = {
        -1.0f, -1.0f, 0.0f,
         1.f, -1.f, 0.0f,
         -1.f,1.f,0.f,
         1.0f,  1.f, 0.0f
    };

    unsigned int indices[] = {
        0, 1, 2, 1, 2, 3
    };

    unsigned int VAO;
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    unsigned int VBO = makeBuffer(GL_ARRAY_BUFFER, GL_STATIC_DRAW, sizeof(vertices), vertices);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    unsigned int IBO = makeBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_STATIC_DRAW, sizeof(indices), indices);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);

	// Define shader files to check for real-time recompiling
    const LPCSTR vs = "../Shaders/raycast.vert";
    const LPCSTR fs = "../Shaders/raymarch.frag";

    __int64 dates, newdates;
    dates = get_filetime(vs) + get_filetime(fs);

    // rendering loop
    while (!glfwWindowShouldClose(window)) {

        // check for shader reload
        newdates = get_filetime(vs) + get_filetime(fs);
        if (newdates != dates) {
            std::cout << "Recompiling shaders" << std::endl;
            vertexShader = compileShader("raycast.vert", GL_VERTEX_SHADER);
            fragmentShader = compileShader("raymarch.frag", GL_FRAGMENT_SHADER);
            shaderProgram = linkProgram(vertexShader, fragmentShader);
            res = glGetUniformLocation(shaderProgram, "uRes");
            time = glGetUniformLocation(shaderProgram, "uTime");
            positions = glGetUniformLocation(shaderProgram, "uPositions");
            center = glGetUniformLocation(shaderProgram, "uCenter");
            faces = glGetUniformLocation(shaderProgram, "uFaces");


            glDeleteShader(fragmentShader);
            glDeleteShader(vertexShader);
            dates = newdates;
        }

        // set background color...
        glClearColor(0.f, 0.f, 0.f, 1.0f);
        // and fill screen with it (therefore clearing the window)
        glClear(GL_COLOR_BUFFER_BIT);

        // render something...
        glUseProgram(shaderProgram);

        glUniform1f(time, getTimeDelta());
        glUniform2ui(res, WINDOW_WIDTH, WINDOW_HEIGHT);
        glUniform3fv(positions, object.positions.size(),&object.positions[0].x);
        glUniform3f(center, z.x,z.y,z.z);
        glUniform3uiv(faces, object.faces.size(), &object.faces[0].x);

        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void*)0);

        // swap buffers == show rendered content
        glfwSwapBuffers(window);
        // process window events
        glfwPollEvents();
    }


    glfwTerminate();
}

void resizeCallback(GLFWwindow*, int width, int height)
{
    // set new width and height as viewport size
    glViewport(0, 0, width, height);
}

float getTimeDelta() {
    auto now = std::chrono::system_clock::now();
    return static_cast<float>((std::chrono::duration_cast<std::chrono::milliseconds>(now - start_time).count() % 500000) / 1000.f);
}