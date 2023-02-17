#include <stdio.h>
#include <string>

#include <SDL.h>
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <glad/glad.h>

static int windowWidth = 800;
static int windowHeight = 600;
static std::string basePath;

std::string LoadTextFile(std::string file)
{
	SDL_RWops* rwOps = SDL_RWFromFile(file.c_str(), "r");
	if (!rwOps) {
		printf("Failed to read file %s.\nSDL ERROR: %s\n", file.c_str(), SDL_GetError());
		exit(-1);
	}
	Sint64 fileSize = rwOps->size(rwOps);
	std::string data{};
	data.resize(fileSize);
	size_t bytesRead = SDL_RWread(rwOps, &data[0], fileSize, 1);

	SDL_RWclose(rwOps);

	return data;
}

GLuint CreateShader(std::string shaderCode, GLenum shaderType)
{
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
		printf("Shader compilation error: %s\n", errorBuffer);
		getchar();
		exit(-1);
	}

	return shader;
}

GLuint CreateShaderProgram(std::string const vert_shader_code, std::string const frag_shader_code)
{
	GLuint vertShader = CreateShader(vert_shader_code, GL_VERTEX_SHADER);
	GLuint fragShader = CreateShader(frag_shader_code, GL_FRAGMENT_SHADER);

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

	SDL_Window* sdlWindow = SDL_CreateWindow("OpenGL Program", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, windowWidth, windowHeight, SDL_WINDOW_OPENGL);
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
	
	/* Load and compile shaders */
	std::string vert_shader_code = LoadTextFile(basePath + "res/shaders/basic_vert.glsl");
	std::string frag_shader_code = LoadTextFile(basePath + "res/shaders/basic_frag.glsl");

	/* Dummy VAO */
	GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	GLuint shaderProgram = CreateShaderProgram(vert_shader_code, frag_shader_code);
	glUseProgram(shaderProgram);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glFrontFace(GL_CCW);
	//glClearColor(0.0f, 0.1f, 0.5f, 1.0f);
	

	bool running = true;
	while (running) {

		/* Do event handling */
		SDL_Event event;
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_QUIT) {
				running = false;
			}
		}

		/* Render */
		//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		GLfloat color[] = { 0.2, 0.2, 0.2, 1.0 };
		glClearBufferfv(GL_COLOR, 0, color);
		glDrawArrays(GL_TRIANGLES,
			0,
			3);

		SDL_GL_SwapWindow(sdlWindow);
	}

	SDL_DestroyWindow(sdlWindow);
	SDL_Quit();

	return 0;
}