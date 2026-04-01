#pragma once

#include <string>
#include <filesystem>

// This file defines wrappers around data needed for framework / game parts loading.
// They serve as intermediaries between the user and the framework's actual types. 
// They will be stolen from.

// TOOD: where the texture alias requires -> hashable etc.

template<typename TextureAlias>				
struct TextureEntry {
	TextureAlias alias;						
	std::filesystem::path texturePath;
};

// ---------------------------------------------


template<typename IconAlias, typename TextureAlias>
struct IconEntry {
	IconAlias alias;
	TextureAlias textureAlias;

	glm::ivec2 pos, dim;
};

// ---------------------------------------------

template<typename TileProperties>
struct TileEntry {

	glm::ivec2 pos;			// integer position used for this tile (user defined alias)
	glm::vec2 offset;		// offset from the bottom left hand corner of the board for the actual position of this tile
	glm::vec2 dim;			// size of the tile for being clickable

	TileProperties properties;

	TileEntry(glm::ivec2&& pos, glm::vec2&& offset, TileProperties&& properties) :
		pos(std::move(pos)), offset(std::move(offset)), dim(TileProperties::TileSize, TileProperties::TileSize), properties(std::move(properties))
	{}

	TileEntry(glm::ivec2&& pos, glm::vec2&& offset, glm::vec2&& dim, TileProperties&& properties) :
		pos(std::move(pos)), offset(std::move(offset)), dim(std::move(dim)), properties(std::move(properties))
	{}
};


struct EdgeEntry {
	glm::ivec2 from;
	glm::ivec2 to;

	int cost;	
};

