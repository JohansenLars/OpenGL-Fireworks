#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <vector>
#include <string>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <algorithm>
#include <Windows.h>
#include <mmsystem.h>

using namespace std;

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define DEFAULT_WIDTH 1024
#define DEFAULT_HEIGHT 768

#define MODELS_SIZE 500


// Vertex Buffer Identifiers
#define GLOBAL_MATRICES 0
#define LIGHT_PROPERTIES 1
#define MATERIAL_PROPERTIES 2
#define CAMERA_PROPERTIES 3




// Vertex Array attributes
#define POSITION 0
#define UV 1
#define NORMAL 2


// Vertex Array binding points
#define STREAM0 0

// GLSL Uniform indices
#define TRANSFORM0 0
#define TRANSFORM1 1
#define LIGHT 2
#define MATERIAL 3
#define CAMERA 4

GLuint textureName;
GLuint textureName2;
GLuint textureName3;


typedef struct {
	GLuint vertexBufferName;
	GLuint indexBufferName;
	GLuint modelMatrixBufferName;
	GLuint vertexArrayName;
	GLuint useTexture;
	GLuint numIndices;
	GLfloat *modelMatrixPtr;
	std::vector<GLushort> indexData;
	std::vector<GLfloat> vertexData;
	GLfloat life;
	glm::vec3 position;
	glm::vec3 speed;
} Model;

Model models[MODELS_SIZE];


// Movement
float dirX = 0.0f;
float dirY = 1.0f;
float dirZ = 0.0f;
float speed = 1.0f;

// Movement
float mDirX = 0.0f;
float mDirY = 0.0f;
float mDirZ = 10.0f;
glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 30.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

//Time
double lastTime;
double deltaTime;

//Sphere
int numIndices;
int numVertices;
int numPer;
std::vector<GLfloat> vertexData;
std::vector<GLushort> indexData;

int createSphere(float radius, int numH, int numV) {

	if (numH < 4 || numV < 2)
		return 0;

	// Variables needed for the calculations
	float pi = glm::pi<float>();
	float pi2 = pi * 2.0f;
	float d1 = pi / numV;
	float d2 = pi2 / numH;

	// Allocate the data needed to store the necessary positions, normals and texture coordinates
	numVertices = numH * (numV - 1) + 2;
	numPer = (3 + 3 + 2);

	std::vector<GLfloat> tmpVertexData(numVertices * numPer);
	vertexData = tmpVertexData;

	// Create the top vertex
	vertexData[0] = 0.0f; vertexData[1] = radius; vertexData[2] = 0.0f;
	vertexData[3] = 0.0f; vertexData[4] = 1.0f; vertexData[5] = 0.0f;
	vertexData[6] = 0.5f; vertexData[7] = 1.0f;

	// Loop through the divisions along the vertical axis
	for (int i = 0; i < numV - 1; ++i) {
		// Loop through the divisions along the horizontal axis
		for (int j = 0; j < numH; ++j) {
			// Calculate the variables needed for this iteration
			int base = (i * numH + j + 1) * numPer;
			float t1 = d1 * (i + 1);
			float t2 = d2 * j;
			// Position (like given in lecture)
			vertexData[base] = radius * glm::sin(t2) * glm::sin(t1);
			vertexData[base + 1] = radius * glm::cos(t1);
			vertexData[base + 2] = radius * glm::cos(t2) * glm::sin(t1);
			// Normal (the same as position except unit length)
			vertexData[base + 3] = glm::sin(t2) * glm::sin(t1);
			vertexData[base + 4] = glm::cos(t1);
			vertexData[base + 5] = glm::cos(t2)*glm::sin(t1);
			// UV 
			vertexData[base + 6] = glm::asin(vertexData[base + 3]) / pi + 0.5f;
			vertexData[base + 7] = glm::asin(vertexData[base + 4]) / pi + 0.5f;
		}
	}

	// Create the bottom vertex
	vertexData[(numVertices - 1)*numPer] = 0.0f; vertexData[(numVertices - 1)*numPer + 1] = -radius; vertexData[(numVertices - 1)*numPer + 2] = 0.0f;
	vertexData[(numVertices - 1)*numPer + 3] = 0.0f; vertexData[(numVertices - 1)*numPer + 4] = -1.0f; vertexData[(numVertices - 1)*numPer + 5] = 0.0f;
	vertexData[(numVertices - 1)*numPer + 6] = 0.5f; vertexData[(numVertices - 1)*numPer + 7] = 0.0f;

	// Allocate the data needed to store the indices
	int numTriangles = (numH*(numV - 1) * 2);
	numIndices = numTriangles * 3;
	std::vector<GLushort> tmpIndexData(numIndices);
	indexData = tmpIndexData;

	// Create the triangles for the top
	for (int j = 0; j < numH; j++) {
		indexData[j * 3] = 0;
		indexData[j * 3 + 1] = (GLushort)(j + 1);
		indexData[j * 3 + 2] = (GLushort)((j + 1) % numH + 1);
	}
	// Loop through the segment circles 
	for (int i = 0; i < numV - 2; ++i) {
		for (int j = 0; j < numH; ++j) {
			indexData[((i*numH + j) * 2 + numH) * 3] = (GLushort)(i*numH + j + 1);
			indexData[((i*numH + j) * 2 + numH) * 3 + 1] = (GLushort)((i + 1)*numH + j + 1);
			indexData[((i*numH + j) * 2 + numH) * 3 + 2] = (GLushort)((i + 1)*numH + (j + 1) % numH + 1);

			indexData[((i*numH + j) * 2 + numH) * 3 + 3] = (GLushort)((i + 1)*numH + (j + 1) % numH + 1);
			indexData[((i*numH + j) * 2 + numH) * 3 + 4] = (GLushort)(i*numH + (j + 1) % numH + 1);
			indexData[((i*numH + j) * 2 + numH) * 3 + 5] = (GLushort)(i*numH + j + 1);
		}
	}
	// Create the triangles for the bottom
	int triIndex = (numTriangles - numH);
	int vertIndex = (numV - 2)*numH + 1;
	for (short i = 0; i < numH; i++) {
		indexData[(triIndex + i) * 3] = (GLushort)(vertIndex + i);
		indexData[(triIndex + i) * 3 + 1] = (GLushort)((numH*(numV - 1) + 1));
		indexData[(triIndex + i) * 3 + 2] = (GLushort)(vertIndex + (i + 1) % numH);
	}
}
// Light properties (4 valued vectors due to std140 see OpenGL 4.5 reference)
GLfloat lightProperties[]{
	// Position
	0.0f, 0.0f, 10.0f, 0.0f,
	// Ambient Color
	0.5f, 0.5f, 0.5f, 0.0f,
	// Diffuse Color
	0.5f, 0.5f, 0.5f, 0.0f,
	// Specular Color
	2.6f, 0.6f, 0.6f, 0.0f
};

GLfloat materialProperties[] = {
	// Shininess color
	0.0f, 1.0f, 0.0f, 0.0f,
	// Shininess
	1.0f
};

// Camera properties 
GLfloat cameraProperties[]{
	mDirX, mDirY, mDirZ
};

// Pointers for updating GPU data
GLfloat *projectionMatrixPtr;
GLfloat *viewMatrixPtr;
GLfloat *globalMatricesPtr;


// Names
GLuint programName;
GLuint bufferNames[8];

/*
 * Read shader source file from disk
 */
char *readSourceFile(const char *filename, int *size) {

	// Open the file as read only
	FILE *file = fopen(filename, "r");

	// Find the end of the file to determine the file size
	fseek(file, 0, SEEK_END);
	long fileSize = ftell(file);

	// Rewind
	fseek(file, 0, SEEK_SET);

	// Allocate memory for the source and initialize it to 0
	char *source = (char *)malloc(fileSize + 1);
	for (int i = 0; i <= fileSize; i++) source[i] = 0;

	// Read the source
	fread(source, fileSize, 1, file);

	// Close the file
	fclose(file);

	// Store the size of the file in the output variable
	*size = fileSize - 1;

	// Return the shader source
	return source;

}

/*
 * Callback function for OpenGL debug messages
 */
void glDebugCallback(GLenum sources, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *msg, const void *userParam) {
	printf("DEBUG: %s\n", msg);
}

int renewModels() {
	GLuint useText;
	int count = 0;
	
	//Count the models that are ready to be rendered again.
	for (int i = 0; i < MODELS_SIZE; i++) {
		if (models[i].life <= 0) {
			count++;
		}

		//When half of the models are ready we give them new life.
		if (count == MODELS_SIZE / 2) {
			glm::vec3 randomdir;
			
			//Base position
			glm::vec3 position = glm::vec3((rand() % 40000 - 20000.0f) / 1000.0f,
										   (rand() % 40000 - 20000.0f) / 1000.0f,
																				0);

			//Randomly select between the 3 textures
			int random = rand() % 100;
			if (random > 67) {
				useText = textureName3;
			}
			else if (random > 33 && random < 67) {
				useText = textureName2;
			}
			else {
				useText = textureName;
			}

			for (int j = 0; j < MODELS_SIZE; j++) {
				if (models[j].life <= 0) {
					models[j].life = 3;
					models[j].useTexture = useText;
					float spread = 2.5f;
					glm::vec3 maindir = glm::vec3(0.0f, 5.0f, 0.0f);
					if (j <= 125) {
						randomdir = glm::vec3(0.2f + (rand() % 1000 - 500.0f) / 1000.0f,
							(rand() % 1000 - 500.0f) / 1000.0f,
							(rand() % 1000 - 500.0f) / 1000.0f);
					}
					else if (j <= 250 && j > 125) {
						randomdir = glm::vec3(-0.2f + (rand() % 1000 - 500.0f) / 1000.0f,
							(rand() % 1000 - 500.0f) / 1000.0f,
							(rand() % 1000 - 500.0f) / 1000.0f);
					}
					else if (j <= 375 && j > 250) {
						randomdir = glm::vec3((rand() % 1000 - 500.0f) / 1000.0f,
							0.2f + (rand() % 1000 - 500.0f) / 1000.0f,
							(rand() % 1000 - 500.0f) / 1000.0f);
					}
					else if (j <= 500 && j > 375) {
						randomdir = glm::vec3((rand() % 1000 - 500.0f) / 1000.0f,
							-0.2f + (rand() % 1000 - 500.0f) / 1000.0f,
							(rand() % 1000 - 500.0f) / 1000.0f);
					}

					models[j].speed = maindir + randomdir * spread;
					models[j].position = position;
				}
			}
		}
	}

	return 1;
}

int simulateModels(float delta) {

	for (int i = 0; i < MODELS_SIZE; i++) {
		models[i].life -= (rand() % 100 + 1) / 100;
		models[i].speed += glm::vec3(-0.0f, -2.81f, -0.0f) * (float)delta * 0.5f;
		models[i].position += models[i].speed * (float)delta;

	}
	return 1;
}

int createModels() {

	stbi_set_flip_vertically_on_load(1);
	GLint width, height, numChannels;
	GLubyte *imageData1 = stbi_load("texture1.png", &width, &height, &numChannels, 3);
	GLubyte *imageData2 = stbi_load("texture2.png", &width, &height, &numChannels, 3);
	GLubyte *imageData3 = stbi_load("texture3.png", &width, &height, &numChannels, 3);


	createSphere(0.1f, 10, 10);

	for (int i = 0; i < MODELS_SIZE; i++) {

		models[i].vertexData = vertexData;
		models[i].indexData = indexData;
		models[i].life = 3.0f;


		float spread = 2.5f;
		glm::vec3 maindir = glm::vec3(0.0f, 5.0f, 0.0f);
		glm::vec3 randomdir = glm::vec3(
			(rand() % 2000 - 1000.0f) / 1000.0f,
			(rand() % 2000 - 1000.0f) / 1000.0f,
			(rand() % 2000 - 1000.0f) / 1000.0f
		);

		models[i].speed = maindir + randomdir * spread;
		models[i].position = glm::vec3(0.0f, 0.0f, 0.0f);

		// Create and initialize 4 buffer names
		glCreateBuffers(1, &models[i].vertexBufferName);
		glCreateBuffers(1, &models[i].indexBufferName);
		glCreateBuffers(1, &models[i].modelMatrixBufferName);

		// Allocate storage for the vertex array buffers
		glNamedBufferStorage(models[i].vertexBufferName, numVertices * numPer * sizeof(GLfloat), &models[i].vertexData[0], 0);
		// Allocate storage for the triangle indices
		glNamedBufferStorage(models[i].indexBufferName, numIndices * sizeof(GLshort), &models[i].indexData[0], 0);
		// Allocate storage for the transformation matrices and retrieve their addresses
		glNamedBufferStorage(models[i].modelMatrixBufferName, 16 * sizeof(GLfloat), NULL, GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);

		// Get a pointer to the model matrix data
		models[i].modelMatrixPtr = (GLfloat *)glMapNamedBufferRange(models[i].modelMatrixBufferName, 0, 16 * sizeof(GLfloat),
			GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);

		// Create and initialize a vertex array object
		glCreateVertexArrays(1, &models[i].vertexArrayName);

		// Associate attributes with binding points
		glVertexArrayAttribBinding(models[i].vertexArrayName, POSITION, STREAM0);
		glVertexArrayAttribBinding(models[i].vertexArrayName, UV, STREAM0);
		glVertexArrayAttribBinding(models[i].vertexArrayName, NORMAL, STREAM0);

		// Specify attribute format
		glVertexArrayAttribFormat(models[i].vertexArrayName, POSITION, 3, GL_FLOAT, GL_FALSE, 0);
		glVertexArrayAttribFormat(models[i].vertexArrayName, UV, 2, GL_FLOAT, GL_FALSE, 6 * sizeof(GL_FLOAT));
		glVertexArrayAttribFormat(models[i].vertexArrayName, NORMAL, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GL_FLOAT));



		// Enable the attributes
		glEnableVertexArrayAttrib(models[i].vertexArrayName, POSITION);
		glEnableVertexArrayAttrib(models[i].vertexArrayName, UV);
		glEnableVertexArrayAttrib(models[i].vertexArrayName, NORMAL);


		// Bind the indices to the vertex array
		glVertexArrayElementBuffer(models[i].vertexArrayName, models[i].indexBufferName);

		// Bind the buffers to the vertex array
		glVertexArrayVertexBuffer(models[i].vertexArrayName, STREAM0, models[i].vertexBufferName, 0, 8 * sizeof(GLfloat));


		glGenTextures(1, &textureName);
		glBindTexture(GL_TEXTURE_2D, textureName);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, imageData1);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

		glGenerateMipmap(GL_TEXTURE_2D);

		glBindTexture(GL_TEXTURE_2D, 0);

		glGenTextures(1, &textureName2);
		glBindTexture(GL_TEXTURE_2D, textureName2);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, imageData2);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

		glGenerateMipmap(GL_TEXTURE_2D);

		glBindTexture(GL_TEXTURE_2D, 1);

		glGenTextures(1, &textureName3);
		glBindTexture(GL_TEXTURE_2D, textureName3);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, imageData3);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

		glGenerateMipmap(GL_TEXTURE_2D);

		glBindTexture(GL_TEXTURE_2D, 2);

	}



	return 1;
}

/*
 * Initialize OpenGL
 */

int initGL() {

	lastTime = glfwGetTime();
	// Register the debug callback function
	glDebugMessageCallback(glDebugCallback, NULL);
	glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, NULL, GL_TRUE);
	glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);

	// Create and initialize 4 buffer names
	glCreateBuffers(8, bufferNames);

	// Allocate storage for the transformation matrices and retrieve their addresses
	glNamedBufferStorage(bufferNames[GLOBAL_MATRICES], 16 * sizeof(GLfloat) * 2, NULL, GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);
	glNamedBufferStorage(bufferNames[LIGHT_PROPERTIES], 16 * sizeof(GLfloat), lightProperties, 0);
	glNamedBufferStorage(bufferNames[MATERIAL_PROPERTIES], 5 * sizeof(GLfloat), materialProperties, 0);
	glNamedBufferStorage(bufferNames[CAMERA_PROPERTIES], 3 * sizeof(GLfloat), cameraProperties, 0);

	// Get a pointer to the global matrices data
	globalMatricesPtr = (GLfloat *)glMapNamedBufferRange(bufferNames[GLOBAL_MATRICES], 0, 16 * sizeof(GLfloat) * 2,
		GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
	projectionMatrixPtr = globalMatricesPtr;
	viewMatrixPtr = globalMatricesPtr + 16;


	createModels();


	// Load and compile vertex shader
	GLuint vertexName = glCreateShader(GL_VERTEX_SHADER);
	int vertexLength = 0;
	char *vertexSource = readSourceFile("final.vert", &vertexLength);
	glShaderSource(vertexName, 1, (const char * const *)&vertexSource, &vertexLength);
	GLint compileStatus;
	glCompileShader(vertexName);
	glGetShaderiv(vertexName, GL_COMPILE_STATUS, &compileStatus);
	if (!compileStatus) {
		GLint logSize = 0;
		glGetShaderiv(vertexName, GL_INFO_LOG_LENGTH, &logSize);
		char *errorLog = (char *)malloc(sizeof(char) * logSize);
		glGetShaderInfoLog(vertexName, logSize, &logSize, errorLog);
		glDeleteShader(vertexName);
		printf("ERROR %s\n", errorLog);
		return 0;
	}
	free(vertexSource);

	// Load and compile fragment shader
	GLuint fragmentName = glCreateShader(GL_FRAGMENT_SHADER);
	int fragmentLength = 0;
	char *fragmentSource = readSourceFile("final.frag", &fragmentLength);
	glShaderSource(fragmentName, 1, (const char * const *)&fragmentSource, &fragmentLength);
	glCompileShader(fragmentName);
	glGetShaderiv(fragmentName, GL_COMPILE_STATUS, &compileStatus);
	if (!compileStatus) {
		GLint logSize = 0;
		glGetShaderiv(fragmentName, GL_INFO_LOG_LENGTH, &logSize);
		char *errorLog = (char *)malloc(sizeof(char) * logSize);
		glGetShaderInfoLog(fragmentName, logSize, &logSize, errorLog);
		glDeleteShader(fragmentName);

		printf("ERROR %s\n", errorLog);
		return 0;
	}
	free(fragmentSource);

	// Create and link vertex program
	programName = glCreateProgram();
	glAttachShader(programName, vertexName);
	glAttachShader(programName, fragmentName);
	glLinkProgram(programName);
	GLint linkStatus;
	glGetProgramiv(programName, GL_LINK_STATUS, &linkStatus);
	if (!linkStatus) {
		GLint logSize = 0;
		glGetProgramiv(programName, GL_INFO_LOG_LENGTH, &logSize);
		char *errorLog = (char *)malloc(sizeof(char) * logSize);
		glGetProgramInfoLog(programName, logSize, &logSize, errorLog);

		printf("LINK ERROR %s\n", errorLog);
		return 0;
	}

	glEnable(GL_DEPTH_TEST);

	return 1;

}
/*
 * Draw OpenGL screne

 */
void drawGLScene() {

	//Getting the time lapsed from last iteration.
	double currentTime = glfwGetTime();
	deltaTime = currentTime - lastTime;
	lastTime = currentTime;

	// Clear color and depth buffers
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Bind tranformation buffers to uniform indices
	glBindBufferBase(GL_UNIFORM_BUFFER, TRANSFORM0, bufferNames[GLOBAL_MATRICES]);
	glBindBufferBase(GL_UNIFORM_BUFFER, LIGHT, bufferNames[LIGHT_PROPERTIES]);
	glBindBufferBase(GL_UNIFORM_BUFFER, MATERIAL, bufferNames[MATERIAL_PROPERTIES]);
	glBindBufferBase(GL_UNIFORM_BUFFER, CAMERA, bufferNames[CAMERA_PROPERTIES]);
	
	// Set the view matrix
	glm::mat4 view = glm::mat4(1.0f);
	view = glm::translate(view, glm::vec3(mDirX, mDirY, mDirZ));
	view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
	memcpy(viewMatrixPtr, &view[0][0], 16 * sizeof(GLfloat));

	renewModels();
	simulateModels(deltaTime);
	


	for (int i = 0; i < MODELS_SIZE; i++) {
		if (models[i].life > 0) {
			
			glm::mat4 model = glm::mat4(1.0);
			model = glm::translate(model, models[i].position);
			memcpy(models[i].modelMatrixPtr, &model[0][0], 16 * sizeof(GLfloat));


			glUseProgram(programName);

			glBindVertexArray(models[i].vertexArrayName);
			glBindTexture(GL_TEXTURE_2D, models[i].useTexture);

			glBindBufferBase(GL_UNIFORM_BUFFER, TRANSFORM1, models[i].modelMatrixBufferName);
			
		}
		glDrawElements(GL_TRIANGLES, numIndices, GL_UNSIGNED_SHORT, 0);
		
	}
	
	// Disable
	glUseProgram(0);
	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);

}


void resizeGL(int width, int height) {

	// Prevent division by zero
	if (height == 0)
		height = 1;

	// Change the projection matrix
	glm::mat4 proj = glm::perspective(3.14f / 2.0f, (float)width / height, 0.1f, 100.0f);
	memcpy(projectionMatrixPtr, &proj[0][0], 16 * sizeof(GLfloat));

	// Set the OpenGL viewport
	glViewport(0, 0, width, height);

}

/*
 * Error callback function for GLFW
 */
static void glfwErrorCallback(int error, const char* description) {
	fprintf(stderr, "Error: %s\n", description);
}

/*
 * Input event callback function for GLFW
 */
static void glfwKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {


	if (key == GLFW_KEY_BACKSPACE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GLFW_TRUE);
	if (key == GLFW_KEY_UP && action == GLFW_PRESS) {
		dirX = 1.0f;
		dirY = 0;
		speed = 0.5f;
	}
	if (key == GLFW_KEY_DOWN && action == GLFW_PRESS) {
		dirX = -1.0f;
		dirY = 0;
		speed = 0.5f;
	}
	if (key == GLFW_KEY_LEFT && action == GLFW_PRESS) {
		dirX = 0;
		dirY = -1.0f;
		speed = 0.5f;
	}
	if (key == GLFW_KEY_RIGHT && action == GLFW_PRESS) {
		dirX = 0;
		dirY = 1.0f;
		speed = 0.5f;

	}
	// Movement
	if (key == GLFW_KEY_W && action == GLFW_PRESS) {
		mDirY += 0.1f;
		cameraPos += cameraFront;
	}
	if (key == GLFW_KEY_S && action == GLFW_PRESS) {
		mDirY -= 0.1f;
		cameraPos -= cameraFront;
	}
	if (key == GLFW_KEY_A && action == GLFW_PRESS) {
		mDirX -= 0.1f;
		cameraPos -= 0.1f;
	}
	if (key == GLFW_KEY_D && action == GLFW_PRESS) {
		mDirX += 0.1f;
		cameraPos += 0.1f;
	}
	if (key == GLFW_KEY_SPACE && action == GLFW_PRESS) {
		mDirZ += 0.1f;
	}
	if (key == GLFW_KEY_C && action == GLFW_PRESS) {
		mDirZ -= 0.1f;
	}

}


/*
 * Window size changed callback function for GLFW
 */
void glfwWindowSizeCallback(GLFWwindow* window, int width, int height) {

	resizeGL(width, height);

}

/*
 * Program entry function
 */
int main(void) {

	// Set error callback
	glfwSetErrorCallback(glfwErrorCallback);

	// Initialize GLFW
	if (!glfwInit()) {
		printf("Failed to initialize GLFW\n");
		exit(EXIT_FAILURE);
	}

	// Specify minimum OpenGL version
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);

	// Create window
	GLFWwindow* window = glfwCreateWindow(DEFAULT_WIDTH, DEFAULT_HEIGHT, "Minimal", NULL, NULL);
	if (!window) {
		printf("Failed to create GLFW window\n");
		glfwTerminate();
		exit(EXIT_FAILURE);
	}

	// Set input key event callback
	glfwSetKeyCallback(window, glfwKeyCallback);

	// Set window resize callback
	glfwSetWindowSizeCallback(window, glfwWindowSizeCallback);
	//glfwSetCursorPosCallback(window, mouse_callback);
	// Make the context current
	glfwMakeContextCurrent(window);

	//Sett mouse callback
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// Initialize GLEW
	if (glewInit() != GLEW_OK) {
		printf("Failed to initialize GLEW\n");
		glfwDestroyWindow(window);
		glfwTerminate();
		exit(EXIT_FAILURE);
	}

	// Make GLFW swap buffers directly 
	glfwSwapInterval(0);

	// Initialize OpenGL
	if (!initGL()) {
		printf("Failed to initialize OpenGL\n");
		glfwDestroyWindow(window);
		glfwTerminate();
		exit(EXIT_FAILURE);
	}

	// Initialize OpenGL view
	resizeGL(DEFAULT_WIDTH, DEFAULT_HEIGHT);

	// Run a loop until the window is closed
	while (!glfwWindowShouldClose(window)) {

		// Draw OpenGL screne
		drawGLScene();

		// Swap buffers
		glfwSwapBuffers(window);

		// Poll fow input events
		glfwPollEvents();

	}

	// Shutdown GLFW
	glfwDestroyWindow(window);
	glfwTerminate();

	// Exit
	exit(EXIT_SUCCESS);

}
