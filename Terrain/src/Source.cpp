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

// Camera
Camera camera(glm::vec3(260,50,300));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;

// Shader Data Set by User Input
unsigned int heightMapScale = 100;
bool firstMouse = true;
bool stepTess = false;
float fogDensity = 0;

unsigned int postProcessFX = 0;
bool usingDepthBuffer = false;
float depthNearPlane = 1;
float depthFarPlane = 1000;

// Lists for Alternate Shaders and Height Maps
std::map<int, std::pair<std::string, Shader*>> shaderList;
Shader* currentShader;
void SetCurrentShader(int shaderNum);

std::map<int, unsigned int> heightMapList;
unsigned int currentHMap;
void SetCurrentHeightMap(int mapNum);

std::map<int, Model*> modelList;

// Framebuffer
void SetVAO(std::vector<float> vertices);
void SetFBOColour();
void SetFBODepth();
void RenderQuad();

unsigned int terrainVAO;
unsigned int quadVAO, quadVBO, FBO;
unsigned int textureColourBuffer;
unsigned int textureDepthBuffer;

// Timing
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

	//Post Processing Shader
	Shader postProcessingShader("..\\shaders\\postProcessing.vs", "..\\shaders\\postProcessing.fs");

	shaderList[0] = { "Standard Shader", &standardShader };
	shaderList[1] = { "Tesselation Shader", &tessShader };
	shaderList[2] = { "Tesselation Shader w/ PN Triangles", &tessPNShader };

	heightMapList[0] = loadTexture("..\\resources\\textures\\defaultHM.jpg");
	heightMapList[1] = loadTexture("..\\resources\\textures\\riverHM.jpg");
	heightMapList[2] = loadTexture("..\\resources\\textures\\centreLakeHM.png");
	heightMapList[3] = loadTexture("..\\resources\\textures\\lightningHM.png");
	SetCurrentHeightMap(0);

	unsigned int waterTex = loadTexture("..\\resources\\textures\\waterMat.jpg");
	unsigned int grassTex = loadTexture("..\\resources\\textures\\grassMat.jpg");
	unsigned int rockTex = loadTexture("..\\resources\\textures\\rockMat.jpg");
	unsigned int snowTex = loadTexture("..\\resources\\textures\\snowMat.jpg");

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, currentHMap);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, waterTex);
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, grassTex);
	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_2D, rockTex);
	glActiveTexture(GL_TEXTURE5);
	glBindTexture(GL_TEXTURE_2D, snowTex);

	//Model treeModel("..\\resources\\models\\Tree1.obj");
	//modelList[0] = &treeModel;

	//Terrain Constructor ; number of grids in width, number of grids in height, gridSize
	Terrain terrain(50, 50,10);
	terrainVAO = terrain.getVAO();

	glm::vec3 lightPos(0.0f, 5.0f, .0f);
	const glm::vec3 skyColour(1.0f, 1.0f, 1.0f);
	glClearColor(skyColour.x, skyColour.y, skyColour.z, 1.0f);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	for (int i = 0; i < shaderList.size(); i++)
	{
		SetCurrentShader(i);
		(*currentShader).use();

		(*currentShader).setFloat("heightMapScale", heightMapScale);
		(*currentShader).setInt("heightMapTex", 1);
		(*currentShader).setInt("waterTex", 2);
		(*currentShader).setInt("grassTex", 3);
		(*currentShader).setInt("rockTex", 4);
		(*currentShader).setInt("snowTex", 5);

		(*currentShader).setFloat("maxDivisions", 100);
		(*currentShader).setFloat("fogDistance", 0.0001f);
		(*currentShader).setVec3("sky", skyColour);

		////light properties
		(*currentShader).setVec3("dirLight.ambient", 0.5f, 0.5f, 0.5f);
		(*currentShader).setVec3("dirLight.diffuse", 0.3f, 0.3f, 0.3f);
		(*currentShader).setVec3("dirLight.specular", 0.2f, 0.2f, 0.2f);
		(*currentShader).setVec3("dirLight.direction", lightPos);
		////material properties
		(*currentShader).setVec3("mat.ambient", 0.3, 0.3, 0.3);
		(*currentShader).setVec3("mat.diffuse", 0.3, 0.3, 0.3);
		(*currentShader).setVec3("mat.specular", 0.1f, 0.1f, 0.1f);
		(*currentShader).setFloat("mat.shininess", 0.25f);
	}

	SetCurrentShader(3);

	while (!glfwWindowShouldClose(window))
	{
		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 1000.0f);
		glm::mat4 view = camera.GetViewMatrix();
		glm::mat4 model = glm::mat4(1.0f);
		
		switch (usingDepthBuffer)
		{
			case(true): SetFBODepth(); break;
			case(false): SetFBOColour(); break;
		}

		glBindFramebuffer(GL_FRAMEBUFFER, FBO);
		(*currentShader).use();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glEnable(GL_DEPTH_TEST);
	
		(*currentShader).setMat4("model", model);
		(*currentShader).setMat4("view", view);
		(*currentShader).setMat4("projection", projection);
		(*currentShader).setVec3("eyePos", camera.Position);


		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, currentHMap);
		(*currentShader).setInt("heightMapTex", 1);

		(*currentShader).setFloat("stepTess", stepTess);
		(*currentShader).setFloat("heightMapScale", heightMapScale);
		(*currentShader).setFloat("fogDensity", fogDensity); 

		glBindVertexArray(terrainVAO);

		if (currentShader != shaderList[0].second) { glDrawArrays(GL_PATCHES, 0, terrain.getSize()); }
		else { glDrawArrays(GL_TRIANGLES, 0, terrain.getSize()); }

		//if (modelList.size() > 0)
		//{
		//	standardShader.use();
		//	for (auto& model : modelList)
		//	{
		//		glActiveTexture(GL_TEXTURE1);
		//		glBindTexture(GL_TEXTURE_2D, grassTex);
		//		(*currentShader).setInt("heightMapTex", 1);
		//		(*model.second).Draw(standardShader);
		//	}
		//}

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		postProcessingShader.use();
		glDisable(GL_DEPTH_TEST);

		switch (usingDepthBuffer)
		{
		glActiveTexture(GL_TEXTURE0);
		case(true): 
			postProcessingShader.setInt("postProcessEffect", postProcessFX);
			postProcessingShader.setBool("isDepthBuffer", true);
			postProcessingShader.setFloat("nearPlane", depthNearPlane);
			postProcessingShader.setFloat("farPlane", depthFarPlane);
			glBindTexture(GL_TEXTURE_2D, textureDepthBuffer);
			break;
		case(false):
			postProcessingShader.setBool("isDepthBuffer", false);
			glBindTexture(GL_TEXTURE_2D, textureColourBuffer);
			break;
		}

		RenderQuad();

		glfwSwapBuffers(window);
		glfwPollEvents();
		processInput(window);
	}

	glfwTerminate();
	return 0;
}

void SetCurrentShader(int shaderNum)
{
	if (shaderNum < shaderList.size()) 
	{
		std::cout << "Using: '" << shaderList[shaderNum].first << "' (" << (shaderNum + 1) << "/" << shaderList.size() << ")" << std::endl;
		currentShader = shaderList[shaderNum].second;
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}
}

void SetCurrentHeightMap(int mapNum)
{
	if (mapNum < heightMapList.size())
	{
		currentHMap = heightMapList[mapNum];
	}
}

void SetFBOColour()
{
	glGenFramebuffers(1, &FBO);
	glBindFramebuffer(GL_FRAMEBUFFER, FBO);

	////////// Colour Attachment //////////
	// Texture to bind to:
	glGenTextures(1, &textureColourBuffer);
	glBindTexture(GL_TEXTURE_2D, textureColourBuffer);

	// Creating space for scene:
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL); 
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	// Bind to frame colour buffer:
	glBindFramebuffer(GL_FRAMEBUFFER, textureColourBuffer);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureColourBuffer, 0);

	// Render buffer:
	unsigned int rbo;
	glGenRenderbuffers(1, &rbo);
	glBindRenderbuffer(GL_RENDERBUFFER, rbo);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, SCR_WIDTH, SCR_HEIGHT);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);
}

void SetFBODepth()
{
	glGenFramebuffers(1, &FBO);

	////////// Depth Attachment //////////
	glGenTextures(1, &textureDepthBuffer);
	glBindTexture(GL_TEXTURE_2D, textureDepthBuffer);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SCR_WIDTH, SCR_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);

	// Texture Settings for Sampling & Iteration
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	// Bind to frame depth buffer:
	glBindFramebuffer(GL_FRAMEBUFFER, FBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, textureDepthBuffer, 0);

	// Specify there is no colour buffer:
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void RenderQuad()
{
	if (quadVAO == 0)
	{
		float quadVertices[] =
		{
			-1.0f,  1.0f,  0.0f,  0.0f,  1.0f,
			-1.0f, -1.0f,  0.0f,  0.0f,  0.0f,
			 1.0f,  1.0f,  0.0f,  1.0f,  1.0f,
			 1.0f, -1.0f,  0.0f,  1.0f,  0.0f
		};

		glGenVertexArrays(1, &quadVAO);
		glGenBuffers(1, &quadVBO);
		glBindVertexArray(quadVAO);

		glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);

		glEnableVertexAttribArray(0); // Position co-ordinates.
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(1); // UV values.
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	}

	glBindVertexArray(quadVAO);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glBindVertexArray(0);
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

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) { glfwSetWindowShouldClose(window, true); }

	if (glfwGetKey(window, GLFW_KEY_M) == GLFW_PRESS) { glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); }
	else if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS) { glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); }

	if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS) { SetCurrentShader(0); }
	if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS) { SetCurrentShader(1); }
	if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS) { SetCurrentShader(2); }

	if (glfwGetKey(window, GLFW_KEY_KP_1) == GLFW_PRESS) { SetCurrentHeightMap(0); }
	else	if (glfwGetKey(window, GLFW_KEY_KP_2) == GLFW_PRESS) { SetCurrentHeightMap(1); }
	else if (glfwGetKey(window, GLFW_KEY_KP_3) == GLFW_PRESS) { SetCurrentHeightMap(2); }
	else if (glfwGetKey(window, GLFW_KEY_KP_4) == GLFW_PRESS) { SetCurrentHeightMap(3); }

	if (glfwGetKey(window, GLFW_KEY_COMMA) == GLFW_PRESS) { usingDepthBuffer = false; }
	else if (glfwGetKey(window, GLFW_KEY_PERIOD) == GLFW_PRESS) { usingDepthBuffer = true; }
	
	if (!usingDepthBuffer)
	{
		if		(glfwGetKey(window, GLFW_KEY_F1) == GLFW_PRESS) { postProcessFX = 0; }
		else if (glfwGetKey(window, GLFW_KEY_F2) == GLFW_PRESS) { postProcessFX = 1; }
		else if (glfwGetKey(window, GLFW_KEY_F3) == GLFW_PRESS) { postProcessFX = 2; }
	}
	else
	{
		if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
		{
			depthFarPlane += 5; cout << "Depth Far Plane: " << depthFarPlane << endl;
		}
		else if ((glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) && (depthFarPlane > 5))
		{
			depthFarPlane -= 5; cout << "Depth Far Plane: " << depthFarPlane << endl;
		}

		if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
		{
			depthNearPlane++; cout << "Depth Near Plane: " << depthNearPlane << endl;
		}
		else if ((glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) && (depthNearPlane > 1))
		{
			depthNearPlane--; cout << "Depth Near Plane: " << depthNearPlane << endl;
		}
	}

	if (glfwGetKey(window, GLFW_KEY_KP_ADD) == GLFW_PRESS) { heightMapScale++; }
	else if (glfwGetKey(window, GLFW_KEY_KP_SUBTRACT) == GLFW_PRESS) { if (heightMapScale > 5) { heightMapScale--; } }

	if (glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS) { stepTess = true; }
	else if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) { stepTess = false; }

	if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS) { fogDensity = 0.25f; }
	else if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS) { fogDensity = 0.0f; }

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) { camera.ProcessKeyboard(FORWARD, deltaTime); }
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) { camera.ProcessKeyboard(LEFT, deltaTime); }
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) { camera.ProcessKeyboard(BACKWARD, deltaTime); }
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) { camera.ProcessKeyboard(RIGHT, deltaTime); }
}