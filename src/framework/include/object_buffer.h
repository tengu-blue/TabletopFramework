#pragma once

struct ObjectsBuffer {

public:
	const GLint vao_id;
	const GLuint texture_id;
	GLsizei count;

	const GLint positions, dimensions, uv_positions, uv_dimensions;		// etc

	ObjectsBuffer(
		GLint vao_id, 
		GLuint texture_id,
		GLint positions, 
		GLint dimensions, 
		GLint uv_positions, 
		GLint uv_dimensions ) :

		vao_id(vao_id),
		texture_id(texture_id),
		count(0),
		positions(positions), 
		dimensions(dimensions), 
		uv_positions(uv_positions), 
		uv_dimensions(uv_dimensions) {}

};

struct SelectionBuffer {
private:
	glm::vec2 position;
	float size;

public:
	const GLint vao_id;
	const GLint positions, dimensions;

	SelectionBuffer(
		GLint vao_id,
		GLint positions,
		GLint dimensions) :
		
		position(0, 0),
		size(0),
		vao_id(vao_id),
		positions(positions),
		dimensions(dimensions) {}

	void set(glm::vec2 newPos, float newSize) {
		position = newPos;
		size = newSize;
	}

	const glm::vec2& getPos() const {
		return position;
	}

	float getSize() const {
		return size;
	}
};