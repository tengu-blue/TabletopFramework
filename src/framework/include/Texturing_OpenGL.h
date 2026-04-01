#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <unordered_map>

#include "framework_exceptions.h"

/// <summary>
/// Icon internally is a subsection of a texture. In OpenGL this corresponds to uv coordinates which are bottom up.
/// </summary>
struct Icon {
public:
	const GLuint texture_id;
	const glm::vec2 pos, dim;

	Icon(GLuint texture_id, glm::vec2 pos, glm::vec2 dim) : 
		texture_id(texture_id), pos(pos), dim(dim) 
	{}
	
};

/// <summary>
/// Icons are sections of textures that contain meaningful pictures. They are held here with their name aliases.
/// </summary>
/// <typeparam name="IconName"> The type for icon name alias</typeparam>
template<typename IconName>
class IconManager {
private:
	
	std::unordered_map<IconName, Icon> icons;

public:


	inline const Icon& getIcon(const IconName& name) {
		if (icons.find(name) != icons.end())
			return icons.at(name);
		else
			throw InvalidArgument<IconName>::invalidIconName(name);
	}

	// --------------------------------------------------

	inline void addIcon(IconName&& name, Icon&& icon) {
		if (icons.find(name) == icons.end())
			icons.emplace(std::move(name), std::move(icon));
		else
			throw InvalidArgument<IconName>::elementDuplicateFound(name);
	}

	inline void addIcon(const IconName& name, Icon&& icon) {
		if (icons.find(name) == icons.end())
			icons.emplace(name, std::move(icon));
		else
			throw InvalidArgument<IconName>::elementDuplicateFound(name);
	}
	
};

// openGL specific uv mapped icons (y axis might be flipped etc.)
// preparation for other systems

static Icon createIconFrom(GLuint texture_id, int image_width, int image_height, int x, int y, int icon_width, int icon_height) {
	const float width = static_cast<float>(image_width);
	const float height = static_cast<float>(image_height);
	
	return { 
		texture_id,
		glm::vec2(x / width, y / height),
		glm::vec2(icon_width / width, icon_height / height)
	};
}

/// <summary>
/// Texture is an image loaded into the GPU via OpenGL.
/// Represented via the id. Contains width and height of the original image.
/// </summary>
struct Texture {
public:
	const int width, height;
	const GLuint id;

	Texture(int width, int height, GLuint id) : width(width), height(height), id(id) {}
};

/// <summary>
/// Textures are actual image files that are loaded by other means and then stored and managed here. Acts as a translator between TextureName type and the actual Textures.
/// </summary>
/// <typeparam name="TextureName">The type for texture names (aliases)</typeparam>
template<typename TextureName>
class TextureManager {
private:
	std::unordered_map<TextureName, Texture> textures;

public:

	inline const Texture& getTexture(const TextureName& name) const {
		if (textures.find(name) != textures.end())
			return textures.at(name);
		else
			throw InvalidArgument<TextureName>::invalidTextureName(name);
	}

	// --------------------------------------------------

	inline void addTexture(TextureName&& name, GLuint textureID, int width, int height) {
		if (textures.find(name) == textures.end())
			textures.emplace(std::move(name), Texture(width, height, textureID));
		else
			throw InvalidArgument<TextureName>::elementDuplicateFound(name);
	}

	inline void addTexture(const TextureName& name, GLuint textureID, int width, int height) {
		if (textures.find(name) == textures.end())
			textures.emplace(name, Texture(width, height, textureID));
		else
			throw InvalidArgument<TextureName>::elementDuplicateFound(name);
	}

};