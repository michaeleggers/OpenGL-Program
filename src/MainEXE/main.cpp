
#include <stdio.h>
#include <string>

#include <SDL.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <glad/glad.h>

#include "me_model_import.h"
#include "me_IO.h"
#include "me_Material.h"
#include "camera.h"


struct DrawElementsIndirectCommand {
	uint32_t  count;
	uint32_t instanceCount;
	uint32_t firstIndex;
	int  baseVertex;
	uint32_t baseInstance;
};

struct DrawArraysIndirectCommand {
	uint32_t count;
	uint32_t instanceCount;
	uint32_t first;
	uint32_t baseInstance;
};

struct DrawData {
	uint32_t indexOffset;
	uint32_t vertexOffset;
	uint32_t materialID;
	uint32_t transformID;
};

struct PerFrameData
{
	glm::mat4 view;
	glm::mat4 projection;
};

struct MouseInputState
{
	uint8_t		buttonStates[5]; // see: https://wiki.libsdl.org/SDL2/SDL_MouseButtonEvent#Remarks
	int32_t		x, y;
	int32_t		oldX, oldY;
	int32_t     dX, dY;
};

static int windowWidth = 800;
static int windowHeight = 600;
static std::string basePath;

bool				keys[SDL_NUM_SCANCODES];
MouseInputState		mouseInput;


GLuint CreateShader(std::string shaderFile, GLenum shaderType)
{
	std::string shaderCode = LoadTextFile(shaderFile);

	GLuint shader = glCreateShader(shaderType);
	GLint const shaderLength = shaderCode.size();
	GLchar const* shaderSource = shaderCode.c_str();
	glShaderSource(shader, 1, &shaderSource, &shaderLength);
	glCompileShader(shader);
	GLchar errorBuffer[1024] = {};
	GLsizei retErrorLength = 0;
	glGetShaderInfoLog(shader,
		sizeof(errorBuffer),
		&retErrorLength,
		errorBuffer);
	if (retErrorLength > 0) {
		printf("Shader compilation error in File: %s\n:%s\n", shaderFile.c_str(), errorBuffer);
		getchar();
		exit(-1);
	}

	return shader;
}

GLuint CreateShaderProgram(std::string const vertShaderFile, std::string const fragShaderFile)
{
	GLuint vertShader = CreateShader(vertShaderFile, GL_VERTEX_SHADER);
	GLuint fragShader = CreateShader(fragShaderFile, GL_FRAGMENT_SHADER);

	GLuint shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertShader);
	glAttachShader(shaderProgram, fragShader);
	glLinkProgram(shaderProgram);
	GLint linkStatus;
	glGetProgramiv(shaderProgram, GL_LINK_STATUS, &linkStatus);
	if (linkStatus == GL_FALSE) {
		GLchar errorBuffer[1024] = {};
		GLsizei retErrorLength = 0;
		glGetProgramInfoLog(shaderProgram,
			sizeof(errorBuffer),
			&retErrorLength,
			errorBuffer);
		printf("Failed to link shaders:\n%s", errorBuffer);
		getchar();
		exit(-1);
	}

	return shaderProgram;
}

#define DOLLY_SPEED			1.0
#define SLOWDOLLYSPEED		(DOLLY_SPEED * 0.1)
void UpdateCamera(Camera& camera)
{
	float dollySpeed = DOLLY_SPEED;
	if (keys[SDL_SCANCODE_LSHIFT]) {
		dollySpeed = SLOWDOLLYSPEED;
	}
	if (keys[SDL_SCANCODE_W]) {
		camera.MoveForward(dollySpeed);
	}
	if (keys[SDL_SCANCODE_S]) {
		camera.MoveForward(-dollySpeed);
	}
	if (keys[SDL_SCANCODE_A]) {
		camera.MoveSide(dollySpeed);
	}
	if (keys[SDL_SCANCODE_D]) {
		camera.MoveSide(-dollySpeed);
	}

	if (mouseInput.buttonStates[SDL_BUTTON_LEFT] == SDL_PRESSED) {
		camera.RotateAroundUp(-mouseInput.dX * 0.005f);			
		camera.RotateAroundSide(mouseInput.dY * 0.005f);
	}
}

int main(int argc, char** argv)
{
	if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
		printf("Could not init SDL2!\nSDL ERROR: %s\n", SDL_GetError());		
		exit(-1);
	}

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 6);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

	SDL_Window* sdlWindow = SDL_CreateWindow("OpenGL Program", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, windowWidth, windowHeight, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
	if (!sdlWindow) {
		printf("Could not create SDL2 window!\nSDL ERROR: %s\n", SDL_GetError());
		exit(-1);
	}

	SDL_GLContext sdlGLContext = SDL_GL_CreateContext(sdlWindow);
	if (!sdlGLContext) {
		printf("Could not create SDL2 OpenGL context!\nSDL ERROR: %s\n", SDL_GetError());
		exit(-1);
	}

	if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress))
	{
		printf("Failed to load OpenGL function pointers!\n");
		return -1;
	}

	/* Init path for loading resources */
	basePath = SDL_GetBasePath();
	
	/* Material Storage */
	MaterialManager materialManager(basePath);

	/* Load models */
	Model spitfire = ImportModel(materialManager, basePath, "knight/", "knight.obj");	

	Vertex cube_vertices[] = {
		// front
		{glm::vec3(-1.0, -1.0,  1.0)},
		{glm::vec3( 1.0, -1.0,  1.0)},
		{glm::vec3( 1.0,  1.0,  1.0)},
		{glm::vec3(-1.0,  1.0,  1.0)},
		// back
		{glm::vec3(-1.0, -1.0, -1.0)},
		{glm::vec3( 1.0, -1.0, -1.0)},
		{glm::vec3( 1.0,  1.0, -1.0)},
		{glm::vec3(-1.0,  1.0, -1.0)},
	};
	uint32_t cube_elements[] = {
		// front
		0, 1, 2,
		2, 3, 0,
		// right
		1, 5, 6,
		6, 2, 1,
		// back
		7, 6, 5,
		5, 4, 7,
		// left
		4, 0, 3,
		3, 7, 4,
		// bottom
		4, 5, 1,
		1, 0, 4,
		// top
		3, 2, 6,
		6, 7, 3
	};
	uint32_t indexCount = sizeof(cube_elements)/sizeof(cube_elements[0]);
	uint32_t vertexCount = sizeof(cube_vertices)/sizeof(Vertex);

	/* Camera */
	Camera camera = Camera(glm::vec3(0, 5, 10));

	/* Prepare buffers for GL shaders */
	GLuint dataIndices;
	glCreateBuffers(1, &dataIndices);
	glNamedBufferStorage(dataIndices, sizeof(uint32_t) * spitfire.indexCount, nullptr, GL_DYNAMIC_STORAGE_BIT);

	GLuint dataVertices;
	glCreateBuffers(1, &dataVertices);
	glNamedBufferStorage(dataVertices, sizeof(Vertex) * spitfire.vertexCount, nullptr, GL_DYNAMIC_STORAGE_BIT);

	GLuint materialBuffer;
	glCreateBuffers(1, &materialBuffer);
	glNamedBufferStorage(materialBuffer, sizeof(Material) * materialManager.m_Materials.size(), nullptr, GL_DYNAMIC_STORAGE_BIT);

	GLuint drawDataBuffer;
	glCreateBuffers(1, &drawDataBuffer);
	glNamedBufferStorage(drawDataBuffer, sizeof(DrawData) * spitfire.meshes.size(), nullptr, GL_DYNAMIC_STORAGE_BIT);
	
	/* Upload Materials */
	for (size_t i = 0; i < materialManager.m_Materials.size(); i++) {
		Material material = materialManager.m_Materials[i];
		glNamedBufferSubData(materialBuffer, i * sizeof(Material), sizeof(Material), &material);
	}

	std::vector<DrawArraysIndirectCommand> drawCmds{};
	uint32_t offset = 0;
	for (size_t i = 0; i < spitfire.meshes.size(); i++) {
		Mesh* mesh = &spitfire.meshes[i];

		/* Fill Index Buffer */
		glNamedBufferSubData(dataIndices, mesh->indexOffset * sizeof(uint32_t), mesh->indices.size() * sizeof(uint32_t), mesh->indices.data());

		/* Fill Vertex Buffer */
		glNamedBufferSubData(dataVertices, mesh->vertexOffset * sizeof(Vertex), mesh->vertices.size() * sizeof(Vertex), mesh->vertices.data());

		/* Fill DrawData buffer with actual draw data */
		DrawData dd = {
			mesh->indexOffset,	// indexOffset;
			mesh->vertexOffset,	// vertexOffset;
			mesh->materialID,	// materialID;
			0					// transformID;
		};
		glNamedBufferSubData(drawDataBuffer, i * sizeof(DrawData), sizeof(DrawData), &dd);

		/* Draw Commands */
		offset += mesh->indices.size();
		DrawArraysIndirectCommand drawCmd{};
		drawCmd.count = mesh->indices.size();
		drawCmd.instanceCount = 1;
		drawCmd.first = 0; // Offset into index-buffer?
		drawCmd.baseInstance = i;
		drawCmds.push_back(drawCmd);
	}
	GLuint drawBuffer;
	glCreateBuffers(1, &drawBuffer);
	glNamedBufferStorage(drawBuffer, drawCmds.size() * sizeof(DrawArraysIndirectCommand), nullptr, GL_DYNAMIC_STORAGE_BIT);
	glNamedBufferSubData(drawBuffer, 0, drawCmds.size() * sizeof(DrawArraysIndirectCommand), drawCmds.data());

	GLuint testMeshVAO;
	glCreateVertexArrays(1, &testMeshVAO);
	glBindVertexArray(testMeshVAO);
	//glVertexArrayElementBuffer(testMeshVAO, dataIndices);	
	
	/* Per frame data */
	PerFrameData perFrameData{};
	GLuint perFrameDataBuffer;
	glCreateBuffers(1, &perFrameDataBuffer);
	glNamedBufferStorage(perFrameDataBuffer, sizeof(PerFrameData), 0, GL_DYNAMIC_STORAGE_BIT);
	glBindBufferRange(GL_UNIFORM_BUFFER, 0, perFrameDataBuffer, 0, sizeof(PerFrameData));

	/* Shader Program for loading meshes */
	GLuint meshShaderProgram = CreateShaderProgram(basePath + "res/shaders/mesh_vert.glsl", basePath + "res/shaders/mesh_frag.glsl");

	/* Setup some basic OpenGL Params */
	//glEnable(GL_CULL_FACE);
	glDisable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glCullFace(GL_BACK);
	glFrontFace(GL_CCW);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	//glClearColor(0.0f, 0.1f, 0.5f, 1.0f);

	/* For now only one shader program active */
	glUseProgram(meshShaderProgram);
	/* Not sure if we need to have more VAOs later and bind different ones each frame */
	glBindVertexArray(testMeshVAO);
	/* Bind buffers _once_ and go */
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, dataIndices);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, dataIndices);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, dataVertices);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, drawDataBuffer);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, materialBuffer);

	bool running = true;
	while (running) {

		/* Do event handling */
		SDL_Event event;
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_QUIT) {
				running = false;
			}

			if (event.type == SDL_KEYDOWN) {
				keys[event.key.keysym.scancode] = true;				
			}
			if (event.type == SDL_KEYUP) {
				keys[event.key.keysym.scancode] = false;
			}

			SDL_GetMouseState(&mouseInput.x, &mouseInput.y);
			if (event.type == SDL_MOUSEBUTTONDOWN) {
				mouseInput.buttonStates[event.button.button] = SDL_PRESSED;
			}
			if (event.type == SDL_MOUSEBUTTONUP) {
				mouseInput.buttonStates[event.button.button] = SDL_RELEASED;
			}
			if (event.type == SDL_MOUSEMOTION) {
				mouseInput.dX = event.motion.xrel;
				mouseInput.dY = event.motion.yrel;
			}			
		}

		/* Update Camera */
		UpdateCamera(camera);
		mouseInput.dX = 0;
		mouseInput.dY = 0;

		/* Update Windows params */
		SDL_GetWindowSize(sdlWindow, &windowWidth, &windowHeight);
		glViewport(0, 0, windowWidth, windowHeight);

		/* Update per frame data */
		perFrameData.view = glm::lookAt(camera.m_Pos, camera.m_Center, camera.m_Up);
		perFrameData.projection = glm::perspective(glm::radians(45.0f), (float)windowWidth / (float)windowHeight, 0.1f, 1000.0f);
		glNamedBufferSubData(perFrameDataBuffer, 0, sizeof(PerFrameData), &perFrameData);

		/* Render */
		GLfloat color[] = { 0.2, 0.2, 0.2, 1.0 };
		glClearColor(0.2, 0.2, 0.2, 1.0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		//glClearBufferfv(GL_DEPTH, , color);
		//glBindVertexArray(vao);
		//glUseProgram(basicShaderProgram);
		//glDrawArrays(GL_TRIANGLES,
		//	0,
		//	3);


		//glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
		 
		//DrawElementsIndirectCommand drawCmd = {
		//	testMesh->indices.size(),
		//	1,
		//	0,
		//	0,
		//	0
		//};
		//glDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_INT, &drawCmd);


		//glDrawArraysInstanced(GL_TRIANGLES,
		//	0,
		//	testMesh->indices.size(),
		//	1);
		// 
		//glDrawArrays(GL_TRIANGLES, 0, testMesh->indices.size());

		glBindBuffer(GL_DRAW_INDIRECT_BUFFER, drawBuffer);
		//glMultiDrawArraysIndirect(GL_TRIANGLES, 0, spitfire.meshes.size(), 0);
		//glMultiDrawArraysIndirect(GL_TRIANGLES, 0, drawCmds.size(), 0);
		glMultiDrawArraysIndirect(GL_TRIANGLES, 0, drawCmds.size(), 0);



		SDL_GL_SwapWindow(sdlWindow);
	}

	SDL_DestroyWindow(sdlWindow);
	SDL_Quit();

	return 0;
}
