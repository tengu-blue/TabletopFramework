#include "RenderAPI_OpenGL.h"
#include <stdexcept>
#include "error_handling_OpenGL.h"

#include <fstream>
#include <iostream>

#include "../include/configuration_file.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

// static API methods

static void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
	// update viewport
	GL_CHECK(glViewport(0, 0, width, height));

	// call custom update functions attached to the API
	void* ptr = glfwGetWindowUserPointer(window);
	if (ptr) {
		RenderAPI* winPtr = reinterpret_cast<RenderAPI*>(ptr);

		winPtr->onResizeProcess(width, height);
	}

}

static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{

	// call custom update functions attached to the API
	void* ptr = glfwGetWindowUserPointer(window);
	if (ptr) {
		RenderAPI* winPtr = reinterpret_cast<RenderAPI*>(ptr);

		double xpos, ypos;
		glfwGetCursorPos(window, &xpos, &ypos);
		winPtr->onMouseInputProcess(button, action, mods, (float) xpos, (float) ypos);
	}
}

static void mouse_moved_callback(GLFWwindow* window, double xpos, double ypos) 
{
	// call custom update functions attached to the API
	void* ptr = glfwGetWindowUserPointer(window);
	if (ptr) {
		RenderAPI* winPtr = reinterpret_cast<RenderAPI*>(ptr);

		winPtr->onMouseMove((float)xpos, (float)ypos);
	}
}

static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
	// call custom update functions attached to the API
	void* ptr = glfwGetWindowUserPointer(window);
	if (ptr) {
		RenderAPI* winPtr = reinterpret_cast<RenderAPI*>(ptr);

		winPtr->onWheelScroll((float)xoffset, (float)yoffset);
	}
}


static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{

	// Escape key == terminate app
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, true);
		return;		// no need to continue
	}

	// call custom update functions attached to the API
	void* ptr = glfwGetWindowUserPointer(window);
	if (ptr) {
		RenderAPI* winPtr = reinterpret_cast<RenderAPI*>(ptr);

		winPtr->onKeyEvent(key, scancode, action, mods);
	}
}


// ------------------------------------------------------------------------------------------------------------------------

RenderAPI::RenderAPI(int width, int height, const std::string& title)
{	
	this->mWindow = nullptr;

	// initialize GLFW
	if (!glfwInit()) {
		throw std::runtime_error("Failed to initialize GLFW");
	}

	// window setup
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// create the window - get its OpenGL context
	mWindow = glfwCreateWindow(width, height, title.c_str(), NULL, NULL);
	if (!mWindow) {
		throw std::runtime_error("Failed to create GLFW window");
	}

	glfwSetFramebufferSizeCallback(mWindow, framebuffer_size_callback);
	glfwSetMouseButtonCallback(mWindow, mouse_button_callback);
	glfwSetCursorPosCallback(mWindow, mouse_moved_callback);
	glfwSetScrollCallback(mWindow, scroll_callback);
	glfwSetKeyCallback(mWindow, key_callback);

	glfwMakeContextCurrent(mWindow);


	// load OpenGL functions using glad
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		throw std::runtime_error("Failed to initialize GLAD");
	}

	// vsync
	glfwSwapInterval(1);

	// for static function callbacks to retrieve this instance
	glfwSetWindowUserPointer(mWindow, this);
}


// --------------------------------------------------------------------------------------------------------

void RenderAPI::renderingTick()
{
	// process input
	glfwPollEvents();

	GL_CHECK(glClearColor(0.2f, 0.3f, 0.3f, 1.0f));
	GL_CHECK(glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT));

	// BODY -> rendering etc (for game, postprocessing, UI etc.)
	for (auto&& renderingPass : renderingPasses)
	{
		renderingPass();
	}


	// GLFW: swap buffers and poll IO events
	glfwSwapBuffers(mWindow);

}

void RenderAPI::addRenderingPass(std::function<void(void)> renderPass)
{
	renderingPasses.push_back(renderPass);
}

void RenderAPI::addWindowResize(std::function<void(int, int)> windowResize) 
{
	windowResizeEvents.push_back(windowResize);
}

void RenderAPI::addMouseInput(std::function<void(int, int, int, float, float)> mouseEvent) 
{
	mouseClickedEvents.push_back(mouseEvent);
}

void RenderAPI::addMouseMove(std::function<void(float, float)> mouseEvent) 
{
	mouseMovedEvents.push_back(mouseEvent);
}

void RenderAPI::addMouseWheel(std::function<void(float, float)> mouseEvent)
{
	mouseWheelEvents.push_back(mouseEvent);
}

void RenderAPI::addKeyEvent(std::function<void(int, int, int, int)> keyEvent) {
	keyEvents.push_back(keyEvent);
}

// ---------------------------------------------------------------------------------------------------------------

bool RenderAPI::shouldClose()
{
	return glfwWindowShouldClose(mWindow);
}

glm::ivec2 RenderAPI::size()
{
	glm::ivec2 result;
	glfwGetWindowSize(mWindow, &(result.x), &(result.y));
	return result;
}

void RenderAPI::onResizeProcess(int width, int height)
{
	// eventually update camera stuff -> note: we only care about 2D rendering, maybe some zoom (scale) so no need for projection matrices etc. make sure the viewport is correct though 

	for (auto&& windowResizeEvent : windowResizeEvents)
	{
		windowResizeEvent(width, height);
	}
}


void RenderAPI::onMouseInputProcess(int button, int action, int mods, float mousex, float mousey) {

	for (auto&& mouseInputEvent : mouseClickedEvents)
	{
		mouseInputEvent(button, action, mods, mousex, mousey);
	}

}

void RenderAPI::onMouseMove(float x, float y) {

	for (auto&& mouseMovedEvent : mouseMovedEvents)
	{
		mouseMovedEvent(x, y);
	}
}

void RenderAPI::onWheelScroll(float xoffset, float yoffset) {
	for (auto&& mouseWheelEvent : mouseWheelEvents)
	{
		mouseWheelEvent(xoffset, yoffset);
	}
}

void RenderAPI::onKeyEvent(int key, int scancode, int action, int mods) {
	for (auto&& keyEvent : keyEvents)
	{
		keyEvent(key, scancode, action, mods);
	}
}


void RenderAPI::terminate()
{
	glfwTerminate();
}

// --------------------------------------------------------------------------------------------------------------

GLuint RenderAPI::createVAO() {
	GLuint id;

	GL_CHECK(glGenVertexArrays(1, &id));

	return id;
}

void RenderAPI::modifyVAO(GLuint id)
{
	GL_CHECK(glBindVertexArray(id));
}

void RenderAPI::stopModifyVAO()
{
	GL_CHECK(glBindVertexArray(0));
}

void RenderAPI::deleteVAO(GLuint id) {
	GL_CHECK(glDeleteVertexArrays(1, &id));
}




GLuint RenderAPI::createBuffer(int size)
{
	GLuint id;

	GL_CHECK(glGenBuffers(1, &id));
	GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, id));
	// static draw -> modified only some times, will be drawn all the time though (potentially better implementation than DYNAMIC)
	GL_CHECK(glBufferData(GL_ARRAY_BUFFER, size, NULL, GL_STATIC_DRAW));
	GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, 0));

	return id;
}

GLuint RenderAPI::createBuffer(int size, const void* data)
{
	GLuint id;

	GL_CHECK(glGenBuffers(1, &id));
	GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, id));
	// static draw -> modified only some times, will be drawn all the time though (potentially better implementation than DYNAMIC)
	GL_CHECK(glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW));
	GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, 0));

	return id;
}

void RenderAPI::modifyBuffer(GLuint id, GLintptr offset, GLsizeiptr size, const void* data)
{
	GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, id));

	GL_CHECK(glBufferSubData(GL_ARRAY_BUFFER, offset, size, data));

	GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, 0));
}

void RenderAPI::deleteBuffer(GLuint id) {
	GL_CHECK(glDeleteBuffers(1, &id));
}

void RenderAPI::setPointer(GLuint id, GLuint index, GLint size, GLenum type, GLsizei stride, size_t offset, bool instanced) {

	GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, id));

	// we don't need to specify offset and won't be using normalized ones
	GL_CHECK(glVertexAttribPointer(index, size, type, GL_FALSE, stride, (void*)offset));
	GL_CHECK(glEnableVertexAttribArray(index));

	// for instanced ones, we alway move per instance
	if(instanced)
		GL_CHECK(glVertexAttribDivisor(index, 1));

	GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, 0));
}



// Shaders ----------------------------------------------------------------------------------------------------------------


inline static std::string loadShaderSource(const fs::path& filePath) {
	// Check if the file exists before trying to open it
	if (!fs::exists(filePath)) {
		throw OpenGLError("File does not exist: " + std::filesystem::absolute(filePath).string());
	}

	std::ifstream fileStream(filePath, std::ios::binary); // Open in binary mode to preserve all data

	if (!fileStream.is_open()) {
		throw std::runtime_error("Failed to open file: " + std::filesystem::absolute(filePath).string());
	}

	// Use iterators to read the file content into a string
	std::string fileContent((std::istreambuf_iterator<char>(fileStream)), std::istreambuf_iterator<char>());

	return fileContent;
}

inline static std::string getShaderInfoLog(GLuint shader) {
	GLint logLength = 0;
	GL_CHECK(glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLength));

	std::vector<char> log(logLength);
	if (logLength > 0) {
		GL_CHECK(glGetShaderInfoLog(shader, logLength, nullptr, log.data()));
		return std::string(log.begin(), log.end());
	}

	return {};
}

inline static auto compileShader(GLenum aShaderType, const std::string& aSource) {
	auto shader = glCreateShader(aShaderType);
	
	const char* src = aSource.c_str();
	GL_CHECK(glShaderSource(shader, 1, &src, nullptr));
	GL_CHECK(glCompileShader(shader));

	// Error handling
	int result;
	GL_CHECK(glGetShaderiv(shader, GL_COMPILE_STATUS, &result));
	if (result == GL_FALSE) {
		throw OpenGLError(getShaderInfoLog(shader));
	}

	return shader;
}

GLuint RenderAPI::createShaderFrom(const fs::path& vertexShaderPath, const fs::path& fragmentShaderPath)
{
	// load from the default shader location
	auto vertexShader = loadShaderSource(vertexShaderPath);
	auto fragmentShader = loadShaderSource(fragmentShaderPath);

	// compile
	auto vs = compileShader(GL_VERTEX_SHADER, vertexShader);
	auto fs = compileShader(GL_FRAGMENT_SHADER, fragmentShader);

	// attach and link
	GLuint program_id = glCreateProgram();
	GL_CHECK(glAttachShader(program_id, vs));
	GL_CHECK(glAttachShader(program_id, fs));
	GL_CHECK(glLinkProgram(program_id));

	// check linked
	GLint isLinked = 0;
	GL_CHECK(glGetProgramiv(program_id, GL_LINK_STATUS, &isLinked));
	if (isLinked == GL_FALSE) {
		GLint maxLength = 0;
		GL_CHECK(glGetProgramiv(program_id, GL_INFO_LOG_LENGTH, &maxLength));

		std::vector<GLchar> infoLog(maxLength);
		GL_CHECK(glGetProgramInfoLog(program_id, maxLength, &maxLength, &infoLog[0]));

		throw OpenGLError("Shader program linking failed:" + std::string(infoLog.begin(), infoLog.end()));
	}

	// check validate
	GL_CHECK(glValidateProgram(program_id));
	GLint isValid = 0;
	GL_CHECK(glGetProgramiv(program_id, GL_VALIDATE_STATUS, &isValid));
	if (isValid == GL_FALSE) {
		GLint maxLength = 0;
		glGetProgramiv(program_id, GL_INFO_LOG_LENGTH, &maxLength);

		std::vector<GLchar> infoLog(maxLength);
		glGetProgramInfoLog(program_id, maxLength, &maxLength, &infoLog[0]);

		throw OpenGLError("Shader program validation failed:" + std::string(infoLog.begin(), infoLog.end()));
	}

	// delete the vertex and fragment (not needed now)
	GL_CHECK(glDeleteShader(vs));
	GL_CHECK(glDeleteShader(fs));

	return program_id;
}

GLuint RenderAPI::createShaderFromSource(std::string vertexShader, std::string fragmentShader)
{
	// compile
	auto vs = compileShader(GL_VERTEX_SHADER, vertexShader);
	auto fs = compileShader(GL_FRAGMENT_SHADER, fragmentShader);

	// attach and link
	GLuint program_id = glCreateProgram();
	GL_CHECK(glAttachShader(program_id, vs));
	GL_CHECK(glAttachShader(program_id, fs));
	GL_CHECK(glLinkProgram(program_id));

	// check linked
	GLint isLinked = 0;
	GL_CHECK(glGetProgramiv(program_id, GL_LINK_STATUS, &isLinked));
	if (isLinked == GL_FALSE) {
		GLint maxLength = 0;
		GL_CHECK(glGetProgramiv(program_id, GL_INFO_LOG_LENGTH, &maxLength));

		std::vector<GLchar> infoLog(maxLength);
		GL_CHECK(glGetProgramInfoLog(program_id, maxLength, &maxLength, &infoLog[0]));

		throw OpenGLError("Shader program linking failed:" + std::string(infoLog.begin(), infoLog.end()));
	}

	// check validate
	GL_CHECK(glValidateProgram(program_id));
	GLint isValid = 0;
	GL_CHECK(glGetProgramiv(program_id, GL_VALIDATE_STATUS, &isValid));
	if (isValid == GL_FALSE) {
		GLint maxLength = 0;
		glGetProgramiv(program_id, GL_INFO_LOG_LENGTH, &maxLength);

		std::vector<GLchar> infoLog(maxLength);
		glGetProgramInfoLog(program_id, maxLength, &maxLength, &infoLog[0]);

		throw OpenGLError("Shader program validation failed:" + std::string(infoLog.begin(), infoLog.end()));
	}

	// delete the vertex and fragment (not needed now)
	GL_CHECK(glDeleteShader(vs));
	GL_CHECK(glDeleteShader(fs));

	return program_id;
}



void RenderAPI::useProgram(GLuint id)
{
	GL_CHECK(glUseProgram(id));
}

GLint RenderAPI::getUniform(GLuint shader_id, const GLchar* name)
{
	return glGetUniformLocation(shader_id, name);
}

GLuint RenderAPI::generateTexture(const fs::path& path_to_texture, int& width, int& height)
{
	// load the image if fails -> throw error
	int nrChannels;
	unsigned char* data = stbi_load(path_to_texture.string().c_str(), &width, &height, &nrChannels, 4);
	if (data)
	{
		GLuint texture_id;
		GL_CHECK(glGenTextures(1, &texture_id));
		GL_CHECK(glBindTexture(GL_TEXTURE_2D, texture_id));

		GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT));
		GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT));
		GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR));
		GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));

		GL_CHECK(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data));
		GL_CHECK(glGenerateMipmap(GL_TEXTURE_2D));

		stbi_image_free(data);

		return texture_id;
	}
	else
	{
		throw std::runtime_error("Failed to load texture: "+path_to_texture.string());
	}
}