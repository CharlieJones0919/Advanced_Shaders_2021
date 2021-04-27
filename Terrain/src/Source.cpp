#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "stb_image.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>

#include "Shader.h"
#include "Camera.h"

#include <Model.h>
#include "Terrain.h"

#include<string>
#include <iostream>
#include <numeric>

#include <map>

// settings
const unsigned int SCR_WIDTH = 1200;
const unsigned int SCR_HEIGHT = 900;

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow *window);
unsigned int loadTexture(char const * path);
//unsigned int loadTexture2(char const * path);
//void setVAO(std::vector<float> vertices);

// camera
Camera camera(glm::vec3(260,50,300));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

std::map<int, std::pair<std::string, Shader*>> shaderList;
Shader* currentShader;
void SetCurrentShader(int next);
unsigned int heightMapScale = 50;
bool stepTess = true;

//arrays
unsigned int terrainVAO;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;


int main()
{
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "IMAT3907", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	//Plain Phong Shader
	Shader standardShader("..\\shaders\\plainVert.vs", "..\\shaders\\plainFrag.fs");
	//Tesselation Shader
	Shader tessShader("..\\shaders\\tessVertex.vs", "..\\shaders\\phongFrag.fs", "..\\shaders\\geometry.gs", "..\\shaders\\tessControlShader.tcs", "..\\shaders\\tessEvaluationShader.tes");
	//Tesselation with PN Triangles
	Shader tessPNShader("..\\shaders\\tessVertex.vs", "..\\shaders\\phongFrag.fs", "..\\shaders\\geometry.gs", "..\\shaders\\PNtessC.tcs", "..\\shaders\\PNtessE.tes");


	shaderList[0] = { "Standard Shader", &standardShader };
	shaderList[1] = { "Tesselation Shader", &tessShader };
	shaderList[2] = { "Tesselation Shader w/ PN Triangles", &tessPNShader };
	
	SetCurrentShader(0);

	unsigned int heightMap = loadTexture("..\\resources\\textures\\heightMap.jpg");
	unsigned int grassTexture = loadTexture("..\\resources\\textures\\grassMat.jpg");

	//std::string modelDir = "..\\resources\\models\\Tree1.obj";
	//std::ifstream modelPath(modelDir);
	//if (!modelPath) std::cout << "Error::could not read filepath: " << modelDir << std::endl; //Check file path was successfully loaded.
	//Model treeModel("..\\resources\\models\\Tree1.obj");
	//Model cyborgModel("..\\resources\\models\\nano\\nanosuit\\nanosuit.obj");

	//Terrain Constructor ; number of grids in width, number of grids in height, gridSize
	Terrain terrain(50, 50,10);
	terrainVAO = terrain.getVAO();

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	glm::vec3 dirLightPos(.7f, -.6f, .2f);

	glm::vec3 pos = glm::vec3(5, 0, 0);

	while (!glfwWindowShouldClose(window))
	{
		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		
		glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 1000.0f);
		glm::mat4 view = camera.GetViewMatrix();
		glm::mat4 model = glm::mat4(1.0f);

		(*currentShader).use();

		(*currentShader).setMat4("model", model);
		(*currentShader).setMat4("view", view);
		(*currentShader).setMat4("projection", projection);
		(*currentShader).setVec3("eyePos", camera.Position);

		(*currentShader).setFloat("maxDivisions", 50);
		(*currentShader).setFloat("stepTess", stepTess);

		(*currentShader).setInt("heightMapTex", 1);
		(*currentShader).setFloat("heightMapScale", heightMapScale);
	
		(*currentShader).setInt("groundTex", 2);
		(*currentShader).setFloat("groundTexScaler", 10);

		////light properties
		(*currentShader).setVec3("dirLight.direction", dirLightPos);
		(*currentShader).setVec3("dirLight.ambient", 0.5f, 0.5f, 0.5f);
		(*currentShader).setVec3("dirLight.diffuse", 0.55f, 0.55f, 0.55f);
		(*currentShader).setVec3("dirLight.specular", 0.6f, 0.6f, 0.6f);
		////material properties
		(*currentShader).setVec3("mat.ambient", 0.3, 0.387, 0.317);
		(*currentShader).setVec3("mat.diffuse", 0.396, 0.741, 0.691);
		(*currentShader).setVec3("mat.specular", 0.297f, 0.308f, 0.306f);
		(*currentShader).setFloat("mat.shininess", 0.9f);

		//treeModel.Draw((*currentShader));
		//cyborgModel.Draw((*currentShader));



		glBindVertexArray(terrainVAO);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, heightMap);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, grassTexture);
		glDrawArrays(GL_PATCHES, 0, terrain.getSize());

		glfwSwapBuffers(window);
		glfwPollEvents();
		processInput(window);
	}


	glfwTerminate();
	return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS) { glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); }
	if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS) { glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); }

	if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS) { SetCurrentShader(0); }
	if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS) { SetCurrentShader(1); }
	if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS) { SetCurrentShader(2); }

	if (glfwGetKey(window, GLFW_KEY_KP_ADD) == GLFW_PRESS) { heightMapScale++; }
	if (glfwGetKey(window, GLFW_KEY_KP_SUBTRACT) == GLFW_PRESS) { if (heightMapScale > 1) { heightMapScale--; } }

	if (glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS) { stepTess = true; }
	if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) { stepTess = false; }

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) { camera.ProcessKeyboard(FORWARD, deltaTime); }
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) { camera.ProcessKeyboard(BACKWARD, deltaTime); }
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) { camera.ProcessKeyboard(LEFT, deltaTime); }
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) { camera.ProcessKeyboard(RIGHT, deltaTime); }
}

void SetCurrentShader(int shaderNum)
{
	if (shaderNum < shaderList.size()) 
	{
		std::cout << "Using: '" << shaderList[shaderNum].first << "' (" << (shaderNum + 1) << "/" << shaderList.size() << ")" << std::endl;
		currentShader = shaderList[shaderNum].second;
	}
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
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
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	camera.ProcessMouseScroll(yoffset);
}

unsigned int loadTexture(char const * path)
{
	unsigned int textureID;
	glGenTextures(1, &textureID);

	int width, height, nrComponents;
	unsigned char *data = stbi_load(path, &width, &height, &nrComponents, 0);
	if (data)
	{
		GLenum format;
		if (nrComponents == 1)
			format = GL_RED;
		else if (nrComponents == 3)
			format = GL_RGB;
		else if (nrComponents == 4)
			format = GL_RGBA;

		glBindTexture(GL_TEXTURE_2D, textureID);
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		stbi_image_free(data);
		std::cout << "Loaded texture at path: " << path << " width " << width << " id " << textureID <<  std::endl;

	}
	else
	{
		std::cout << "Texture failed to load at path: " << path << std::endl;
		stbi_image_free(data);
		
	}

	return textureID;
}