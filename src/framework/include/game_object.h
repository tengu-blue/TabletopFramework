#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include "object_buffer.h"

// forward declaration for friend access
template <typename FrameworkProperties>
class GameGraphicsFacade;

class GameObject {
private:
	// only accessible for reading from friends (instead of being public)
	const GLint vao;
	const GLsizei identifier;

	// private constructor to make it impossible to create without going through the Facade
	GameObject(GLint vao, GLsizei identifier) : vao(vao), identifier(identifier) {}

	// all facades are friends
	template<typename FrameworkProperties>
	friend class GameGraphicsFacade;

};