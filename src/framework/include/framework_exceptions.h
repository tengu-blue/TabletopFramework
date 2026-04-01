#pragma once

#include <glm/glm.hpp>
#include <stdexcept>
#include <string>

class FrameworkException : public std::runtime_error {
public:
	FrameworkException(const std::string& message) : std::runtime_error(message) {}
};

class InvalidSetting : public FrameworkException {
public:
	InvalidSetting(const std::string& msg) : FrameworkException(msg) {}
};


template<typename ArgumentType>
class InvalidArgument : public FrameworkException {
private:
	ArgumentType argument;
	
	InvalidArgument(const std::string& msg, const ArgumentType& argument) : FrameworkException(msg), argument(argument) {}

public:

	const ArgumentType& getArgument() {
		return argument;
	}

	// --------------------------------------------------------------------------------

	static InvalidArgument<ArgumentType> elementDuplicateFound(const ArgumentType& element) {
		return InvalidArgument<ArgumentType>("Duplicate element of given alias already exists!", element);
	}

	static InvalidArgument<std::size_t> invalidPlayerId(std::size_t playerId) {
		return InvalidArgument<std::size_t>("The given player id is not valid!", playerId);
	}

	static InvalidArgument<ArgumentType> invalidIconName(const ArgumentType& name) {
		return InvalidArgument<ArgumentType>("The given icon name is not valid!", name);
	}

	static InvalidArgument<ArgumentType> invalidTextureName(const ArgumentType& name) {
		return InvalidArgument<ArgumentType>("The given texture name is not valid!", name);
	}

	static InvalidArgument<glm::ivec2> tileNotFound(int x, int y) {
		return InvalidArgument<glm::ivec2>("Tile not found!", glm::ivec2(x, y));
	}

	static InvalidArgument<glm::ivec2> tileNotFound(const glm::ivec2& tilePosition) {
		return InvalidArgument<glm::ivec2>("Tile not found!", tilePosition);
	}

	static InvalidArgument<ArgumentType> pieceNotFound(const ArgumentType& name) {
		return InvalidArgument<ArgumentType>("Piece not found!", name);
	}

	static InvalidArgument<glm::ivec2> pieceNotFoundAtTile(const glm::ivec2& tile) {
		return InvalidArgument<glm::ivec2>("Piece not found at the specified tile", tile);
	}
};