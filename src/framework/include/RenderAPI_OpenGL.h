#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include <vector>
#include <string>
#include <array>
#include <functional>
#include <forward_list>
#include <filesystem>

#include "error_handling_OpenGL.h"


namespace fs = std::filesystem;

/// <summary>
/// OpenGL implementation of low-level-ish utilities and functions. Used by the the GameFacade.
/// Facilitates rendering related functionality.
/// </summary>
class RenderAPI {
private:
	// window 
	GLFWwindow* mWindow;

	// rendering subroutines
	std::vector<std::function<void(void)>> renderingPasses;

	// add function calls for resize and key / mouse
	std::vector<std::function<void(int, int)>> windowResizeEvents;

	std::vector<std::function<void(int, int, int, float, float)>> mouseClickedEvents;

	std::vector<std::function<void(float, float)>> mouseMovedEvents;

	std::vector<std::function<void(float, float)>> mouseWheelEvents;

	std::vector<std::function<void(int, int, int, int)>> keyEvents;

public:
	RenderAPI(int width, int height, const std::string& title);

	// --------------------------------------------------------------------------------

	// window state

	bool shouldClose();

	glm::ivec2 size();


	// events and actions

	void renderingTick();

	void addRenderingPass(std::function<void(void)> renderPass);

	void addWindowResize(std::function<void(int, int)> windowResize);

	void addMouseInput(std::function<void(int, int, int, float, float)> mouseInput);

	void addMouseMove(std::function<void(float, float)> mouseMove);

	void addMouseWheel(std::function<void(float, float)> mouseWheel);

	void addKeyEvent(std::function<void(int, int, int, int)> keyEvent);

	void onResizeProcess(int width, int height);

	void onMouseInputProcess(int button, int action, int mods, float mousex, float mousey);

	void onMouseMove(float xpos, float ypos);

	void onWheelScroll(float xoffset, float yoffset);

	void onKeyEvent(int key, int scancode, int action, int mods);


	void terminate();

	// buffers and other objects
	
	GLuint createVAO();
	void modifyVAO(GLuint id);
	void stopModifyVAO();
	void deleteVAO(GLuint id);

	GLuint createBuffer(int size);
	GLuint createBuffer(int size, const void* data);
	void modifyBuffer(GLuint id, GLintptr offset, GLsizeiptr size, const void* data);
	void deleteBuffer(GLuint id);

	/// <summary>
	/// NOTE: Requires a proper VAO to be set manually beforehand.
	/// </summary>
	void setPointer(GLuint id, GLuint index, GLint size, GLenum type, GLsizei stride, size_t offset, bool instanced);

	GLuint createShaderFrom(const fs::path& vertexShaderPath, const fs::path& fragmentShaderPath);
	GLuint createShaderFromSource(std::string vertexShader, std::string fragmentShader);
	void useProgram(GLuint id);
	GLint getUniform(GLuint shader_id, const GLchar* name);

	GLuint generateTexture(const fs::path& path_to_texture, int& width, int& height);
};

