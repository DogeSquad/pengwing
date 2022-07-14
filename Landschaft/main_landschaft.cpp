#define STB_IMAGE_IMPLEMENTATION

#include "utils.h"
#include "camera.h"
#include "perlin.h"
#include "createimage.h"
#include "terrain.h"

#include "shader.h"
#include "soil.h"
#include "modelLoader.h"
#include <vendor/glm/glm/gtx/string_cast.hpp>
#include<vendor/glfw/include/GLFW/glfw3.h>
#include<vendor/glfw/include/GLFW/glfw3native.h>
#include <vendor/glm/glm/glm.hpp>
#include <vendor/glad/include/glad/glad.h>

int windowWidth = 800, windowHeight = 800;

// Kamera
Camera camera(glm::vec3(0.0f, 15.0f, 0.0f));
float lastX = windowWidth / 2.0f;
float lastY = windowHeight / 2.0f;
bool firstMouse = true;
float deltaTime = 0.0f;	// zeit swischen dem nächsten und den davorherigen frame
float lastFrame = 0.0f;

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

void createPlane(std::vector<int> &position, int xOffset, int yOffset, int height, int width, float heightMultiplier, float mapScale, unsigned int &program, GLuint &plane_VAO);
void setupModelTransformation(unsigned int &);
void setupViewTransformation(unsigned int &);
void setupProjectionTransformation(unsigned int &, int, int);
void createWorldTerrain(int height, int width, float heightMultiplier, float mapScale, unsigned int &program, std::vector<GLuint> &map_chunks, int numChunksVisible);
void processInput(GLFWwindow *window);
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
std::vector<float> generateNoiseMap(int offsetX, int offsetY, int chunkHeight, int chunkWidth);

int main(int, char**)
{
	//map generation
	int mapHeight = 128; //Height of each chunk
	int mapWidth = 128; //Width of each chunk
	float heightMultiplier = 75.0f; //Scale for height of peak
	float mapScale = 1.0f; //Scale for height and breadth of each chunk

	// Setup window
	GLFWwindow *window = setupWindow(windowWidth, windowHeight);
	ImGuiIO& io = ImGui::GetIO(); // Create IO object
	// glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);

	//GLFW soll unsere Maus "capturen"
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	ImVec4 clearColor = ImVec4(1.0f, 1.0f, 1.0f, 1.00f);

	unsigned int shaderProgram = createProgram("./shaders/vshader.vs", "./shaders/fshader.fs");
	glUseProgram(shaderProgram);

	//unsigned int VAO;
	std::vector<GLuint> map_chunks(terrainXChunks * terrainYChunks);
	//glGenVertexArrays(1, &VAO);

	setupModelTransformation(shaderProgram);
	setupProjectionTransformation(shaderProgram, mapWidth, mapHeight); //maybe window width and height, check


	Shader ourShader("./shaders/model.vs", "./shaders/model.fs");
	Model ourModel("./objects/models/penguin/penguin.obj"); //eventuell Pinguine einladen
	ourShader.use();

	while (!glfwWindowShouldClose(window))
	{

		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		// input
		// -----
		processInput(window);
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
		glClearColor(clearColor.x, clearColor.y, clearColor.z, clearColor.w);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);




		int camPosition_uniform = glGetUniformLocation(shaderProgram, "camPosition");
		if (camPosition_uniform == -1) {
			fprintf(stderr, "Could not bind location: camPosition\n");
			exit(0);
		}
		//glUniform3fv(camPosition_uniform, 1, glm::value_ptr(camera.Position));

		//Landschaft erstellen
		createWorldTerrain(mapHeight, mapWidth, heightMultiplier, mapScale, shaderProgram, map_chunks, numChunksVisible);

		setupViewTransformation(shaderProgram);

		glUseProgram(shaderProgram);

		{
			static float f = 0.0f;
			static int counter = 0;

		}

		// Rendering
		ImGui::Render();
		int display_w, display_h;
		glfwGetFramebufferSize(window, &display_w, &display_h);
		glViewport(0, 0, display_w, display_h);

		for (int i = 0; i < terrainXChunks * terrainYChunks; i++) {
			//std::cout << i << " " << map_chunks[i] << std::endl;
			glBindVertexArray(map_chunks[i]);
			glDrawArrays(GL_TRIANGLES, 0, (mapWidth - 1) * (mapHeight - 1) * 6);
		}

		ourShader.use();
		glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)windowWidth / (float)windowHeight, 0.1f, 100.0f);
		glm::mat4 view = camera.GetViewMatrix();
		ourShader.setMat4("projection", projection);
		ourShader.setMat4("view", view);

		glm::mat4 model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));
		model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));
		ourShader.setMat4("model", model);
		ourModel.Draw(ourShader);
		for (unsigned int i = 0; i < amount; i++)
		{
			ourShader.setMat4("model", modelMatrices[i]);
			ourModel.Draw(ourShader);
		}

		glDepthFunc(GL_LEQUAL);

		view = glm::mat4(glm::mat3(camera.GetViewMatrix()));
		glActiveTexture(GL_TEXTURE0);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		glBindVertexArray(0);
		glDepthFunc(GL_LESS);


		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		glfwPollEvents();
		glfwSwapBuffers(window);

	}

	for (int i = 0; i < map_chunks.size(); i++) {
		glDeleteVertexArrays(1, &map_chunks[i]);
	}
	// Cleanup
	cleanup(window);

	return 0;
}

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
				float xSample = (x + (offsetX*(chunkWidth - 1)) - (float)((chunkWidth - 1) / 2)) / noiseScale * freq;
				float ySample = (-y + (offsetY*(chunkHeight - 1)) + (float)((chunkHeight - 1) / 2)) / noiseScale * freq;

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
void createPlane(std::vector<int> &position, int xOffset, int yOffset, int height, int width, float heightMultiplier, float mapScale, unsigned int &program, GLuint &plane_VAO) //Check if offsets required
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
	GLfloat *expanded_vertices = currTerrain->getTrianglePoints();

	GLuint vertex_VBO;
	glGenBuffers(1, &vertex_VBO);
	glBindBuffer(GL_ARRAY_BUFFER, vertex_VBO);
	glBufferData(GL_ARRAY_BUFFER, nVertices * 3 * sizeof(GLfloat), expanded_vertices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);


	GLfloat *expanded_normals = currTerrain->getNormals(height, width);

	GLuint normal_VBO;
	glGenBuffers(1, &normal_VBO);
	glBindBuffer(GL_ARRAY_BUFFER, normal_VBO);
	glBufferData(GL_ARRAY_BUFFER, nVertices * 3 * sizeof(GLfloat), expanded_normals, GL_STATIC_DRAW);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(2);

	// delete []expanded_normals;
	delete[]expanded_vertices;


	GLfloat *expanded_vertices2 = currTerrain->getTrianglePoints();
	GLfloat *expanded_colors = new GLfloat[nVertices * 3];

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
void createWorldTerrain(int mapHeight, int mapWidth, float heightMultiplier, float mapScale, unsigned int &program, std::vector<GLuint> &map_chunks, int numChunksVisible) {

	int chunkHeight = mapHeight;
	int chunkWidth = mapWidth;

	//sicher gehen dass man die richtigen x und y Koord. nimmt
	int currChunkCoordX = (int)round(camera.Position.x / chunkWidth); //nimm aktuelle x-chunk Koordinate
	int currChunkCoordY = (int)round(camera.Position.z / chunkHeight); //nimm atkuelle y-chunk Koordinate (oder z abhängig von unserem Koord.Sys)


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

void setupModelTransformation(unsigned int &program)
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

void setupViewTransformation(unsigned int &program)
{

	glm::mat4 view = camera.GetViewMatrix();
	glUseProgram(program);
	int vView_uniform = glGetUniformLocation(program, "vView");
	if (vView_uniform == -1) {
		fprintf(stderr, "Could not bind location: vView\n");
		exit(0);
	}
	glUniformMatrix4fv(vView_uniform, 1, GL_FALSE, glm::value_ptr(view));
}

void setupProjectionTransformation(unsigned int &program, int screen_width, int screen_height)
{
	//Projection transformation
	float aspect = (float)screen_width / (float)screen_height;

	glm::mat4 projection = glm::perspective(45.0f, (GLfloat)screen_width / (GLfloat)screen_height, 0.1f, 1000.0f);

	//Pass on the projection matrix to the vertex shader
	glUseProgram(program);
	int vProjection_uniform = glGetUniformLocation(program, "vProjection");
	if (vProjection_uniform == -1) {
		fprintf(stderr, "Could not bind location: vProjection\n");
		exit(0);
	}
	glUniformMatrix4fv(vProjection_uniform, 1, GL_FALSE, glm::value_ptr(projection));
}

void processInput(GLFWwindow *window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		camera.ProcessKeyboard(FORWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		camera.ProcessKeyboard(BACKWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		camera.ProcessKeyboard(LEFT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		camera.ProcessKeyboard(RIGHT, deltaTime);

	if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
		camera.ProcessKeyboard(FORWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
		camera.ProcessKeyboard(BACKWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
		camera.ProcessKeyboard(LEFT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
		camera.ProcessKeyboard(RIGHT, deltaTime);
}

// glfw: whenever the window size changed, this callback function executes
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	// make sure the viewport matches the new window dimensions; note that width and
	// height will be significantly larger than specified on retina displays.
	glViewport(0, 0, width, height);
}


// glfw: whenever the mouse moves, this callback is called
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

	lastX = xpos;
	lastY = ypos;

	camera.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	camera.ProcessMouseScroll(yoffset);
}
