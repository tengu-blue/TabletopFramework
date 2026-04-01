#pragma once

#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

class Camera {
public:

	Camera() :
		left(-4), right(4), top(-3), bottom(3),
		nearPlane(0.01f), farPlane(100.0f),
		position(glm::vec3(0.0f)),
		scale(glm::vec3(1.0f)) {}

	Camera(float left, float right, float top, float bottom, float nearPlane=0.01f, float farPlane=100.0f)
		: left(left), right(right), top(top), bottom(bottom), nearPlane(nearPlane), farPlane(farPlane),
		position(glm::vec3(0.0f)),
		scale(glm::vec3(1.0f))
	{}

	void setOrthoDims(float width, float height) {
		this->left = -width/2;
		this->right = -left;
		this->top = -height/2;
		this->bottom = -top;
	}

	glm::mat4 getViewMatrix() const {
		glm::mat4 view = glm::translate(glm::mat4(1.0f), -position);
		view = glm::scale(view, scale);
		return view;
	}

	glm::mat4 getProjectionMatrix() const {
		return glm::ortho(left, right, top, bottom, nearPlane, farPlane);
	}

	void move(glm::vec3 dir) {
		position += dir;
	}

	void setScale(float val) {
		scale = { val, val, val };
	}

	float getScale() const {
		return scale.x;
	}

	void setPosition(glm::vec3&& nPos) {
		position = std::move(nPos);
	}

	glm::vec2 get2DPosition() const {
		return { position.x, position.y };
	}


private:
	float left, right, top, bottom;
	float nearPlane;
	float farPlane;

	glm::vec3
		position,
		scale;
};
