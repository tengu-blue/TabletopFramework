#pragma once

#include <glm/glm.hpp>


template <typename TileProperties>
class Tile {
private:
	TileProperties properties;
public:
	const glm::vec2 position, dimensions;
	const glm::ivec2 boardCoordinates;

	Tile(glm::vec2 position, glm::vec2 dimensions, glm::ivec2 boardCoordinates, TileProperties&& properties)
		: properties(properties), position(position), dimensions(dimensions), boardCoordinates(boardCoordinates)
	{

	}

	TileProperties& getProperties() {
		return properties;
	}

	const TileProperties& getProperties() const {
		return properties;
	}

};

struct Edge {
	const int cost;
	const glm::ivec2 from, to;
};