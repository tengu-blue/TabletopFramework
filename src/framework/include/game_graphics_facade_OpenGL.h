#pragma once

#include "RenderAPI_OpenGL.h"
#include "configuration_file.h"
#include "camera.h"


#include <memory>
#include "string_utils.h"
#include <algorithm>
#include <vector>
#include <list>
#include <glm/gtc/type_ptr.hpp>
#include <sstream>
#include <fstream>

#include "game_object.h"
#include "Texturing_OpenGL.h"
#include "game_loading_utils.h"

#include <iostream>


template <typename FrameworkProperties>
class GameGraphicsFacade {

	using Types = FrameworkTypes<FrameworkProperties>;
	using TextureAliasRaw = typename Types::TextureAliasRaw;
	using IconsLoader = typename Types::IconsLoader;
	using IconAliasRaw = typename Types::IconAliasRaw;
	using IconAlias = typename Types::IconAlias;

private:

	struct MouseInfo {
		float lastX, lastY, nowX, nowY;
		float dx, dy;

		void update(float newX, float newY) {
			dx = lastX - nowX;
			dy = lastY - nowY;
			
			lastX = nowX;
			lastY = nowY;

			nowX = newX;
			nowY = newY;
		}
		

	} mouseInfo;

	RenderAPI& renderer;
	TextureManager<TextureAliasRaw>& texture_manager;

	// Icons

	IconManager<IconAliasRaw> icon_manager;

	// basic geometry for the "quad" and uv coords
	GLuint quad_buffer;
	std::list<ObjectsBuffer> object_buffers;	// could just use a vector (not used that often)
	
	// shader
	GLuint shaderProgram;
	// shader uniforms
	GLint u_projection, u_view, u_texture;

	// Camera
	Camera camera;

	

	// ----------------------------------------------------------------------

	void gameRenderLoop()
	{
		// bind the shader
		renderer.useProgram(shaderProgram);

		// set uniforms
		auto projMat = camera.getProjectionMatrix();
		auto viewMat = camera.getViewMatrix();

		GL_CHECK(glUniformMatrix4fv(u_view, 1, GL_FALSE, glm::value_ptr(viewMat)));
		GL_CHECK(glUniformMatrix4fv(u_projection, 1, GL_FALSE, glm::value_ptr(projMat)));

		// rebind texture only when it changes
		GLuint lastTexture = -1;	// -1 is fine here, at worst be rebind a texture more then it needed to
		// render all objects
		for (auto&& object_buffer : object_buffers) {

			// rebind texture if needed
			if (lastTexture == -1 || object_buffer.texture_id != lastTexture) {

				lastTexture = object_buffer.texture_id;

				GL_CHECK(glActiveTexture(GL_TEXTURE0));
				GL_CHECK(glBindTexture(GL_TEXTURE_2D, lastTexture));
				GL_CHECK(glUniform1i(u_texture, 0));	// 0 for the active texture being 0

			}

			// bind the buffer
			renderer.modifyVAO(object_buffer.vao_id);

			// draw all the objects in this object buffer
			GL_CHECK(glDrawArraysInstanced(GL_TRIANGLES, 0, 6, object_buffer.count));

		}

		// avoid unintentional calls to the last VAO
		renderer.stopModifyVAO();

		// ---------------------------------------------------

		if (selected) {

			renderer.useProgram(shaderProgramSelection);

			GL_CHECK(glUniformMatrix4fv(u_view2, 1, GL_FALSE, glm::value_ptr(viewMat)));
			GL_CHECK(glUniformMatrix4fv(u_projection2, 1, GL_FALSE, glm::value_ptr(projMat)));
			GL_CHECK(glUniform2fv(u_selectionCenter, 1, glm::value_ptr(selection->getPos())));
			GL_CHECK(glUniform1f(u_selectionRadius, selection->getSize()));
			GL_CHECK(glUniform1f(u_time, static_cast<float>(glfwGetTime())));
			
			renderer.modifyVAO(selection->vao_id);
			
			GL_CHECK(glDrawArrays(GL_TRIANGLES, 0, 6));

			renderer.stopModifyVAO();
		}

	}
	void windowResize(int width, int height)
	{
		camera.setOrthoDims((float) width, (float) height);
	}


	ObjectsBuffer& enlistObjectBuffer(GLuint vao, GLuint texture_id, GLuint pos, GLuint dim, GLuint uv_pos, GLuint uv_dim) {
		
		DEBUG_ACTION(int i = 0);
		// with list, we try to queue the new buffer close to an existing one with the same texture_id
		// if we don't find it, we place it at the end

		auto it = object_buffers.begin();
		while (it != object_buffers.end()) {
			// find the section of buffers for this texture_id and insert before it (all the full ones are gonna be after it)
			if (it->texture_id == texture_id) {
				
				// remember it
				break;
			}

			DEBUG_ACTION(i++);
			++it;
		}
		DEBUG_ACTION(std::cout << "will place object at: " << i << std::endl);

		// either it is first match texture_id
		// or
		// didn't find any matching texture_id -> place at the end
		return *object_buffers.emplace(it, vao, texture_id, pos, dim, uv_pos, uv_dim);
	}

	// change to return a reference to the created buffer (for adding objects to it)
	// add texture id as argument for sorting 
	ObjectsBuffer& allocateObjectBuffer(GLuint texture_id)
	{
		// allocate VAO
		auto vao = renderer.createVAO();
		renderer.modifyVAO(vao);

		// bind the geometry and setup pointers
		renderer.setPointer(quad_buffer, 0, 3, GL_FLOAT, 5 * sizeof(float), 0, false);	// x, y, z
		renderer.setPointer(quad_buffer, 1, 2, GL_FLOAT, 5 * sizeof(float), 3 * sizeof(float), false);	// u, v

		// allocate buffers for per object data (position, dimension, uv_position, uv_dimension, object_id=int)
		// setup their pointers

		auto pos = renderer.createBuffer(sizeof(glm::vec3) * DEFAULT_ARRAY_SIZE);
		renderer.setPointer(pos, 2, 3, GL_FLOAT, 3 * sizeof(float), 0, true);
		
		auto dim = renderer.createBuffer(sizeof(glm::vec2) * DEFAULT_ARRAY_SIZE);
		renderer.setPointer(dim, 3, 2, GL_FLOAT, 2 * sizeof(float), 0, true);

		auto uv_pos = renderer.createBuffer(sizeof(glm::vec2) * DEFAULT_ARRAY_SIZE);
		renderer.setPointer(uv_pos, 4, 2, GL_FLOAT, 2 * sizeof(float), 0, true);

		auto uv_dim = renderer.createBuffer(sizeof(glm::vec2) * DEFAULT_ARRAY_SIZE);
		renderer.setPointer(uv_dim, 5, 2, GL_FLOAT, 2 * sizeof(float), 0, true);

		// remember the texture id for this one -> try to sort all object_buffers / keep in the list

		renderer.stopModifyVAO();
		
		// manage the newly created buffer
		return enlistObjectBuffer(vao, texture_id, pos, dim, uv_pos, uv_dim);
	};

	/// <summary>
	/// Retrieves the ObjectBuffer in which the specified object can be found.
	/// </summary>
	/// <param name="object">Object whose buffer we are looking for.</param>
	/// <returns></returns>
	ObjectsBuffer& findObjectBuffer(const GameObject& object) {
		auto it = std::find_if(object_buffers.begin(), object_buffers.end(),
			[&](const ObjectsBuffer& elem) {
				return elem.vao_id == object.vao;
			});

		// should not happen -> strange behaviour
		if (it == object_buffers.end()) {
			throw std::runtime_error("Game object is not inside an active buffer");
		}

		return *it;
	}

	ObjectsBuffer& getBufferFor(const GLuint texture_id) {
		DEBUG_ACTION(int i = 0);

		auto it = object_buffers.begin();
		while (it != object_buffers.end()) {
			// we know here that the first one found is either partially empty, or if not a new one has to be created
			// this is all implementation depended, saves us time iterating though the whole list
			if (it->texture_id == texture_id) {
				if (it->count != DEFAULT_ARRAY_SIZE) {
					DEBUG_ACTION(std::cout << "found not full buffer at: " << i << " with " << (*it).count << " objects" << std::endl);
					
					return *it;
				}
				else {
					DEBUG_ACTION(std::cout << "could not find non full buffer at: " << i << std::endl);
					break;	// create a new one
				}
			}

			DEBUG_ACTION(++i);
			++it;
		}

		DEBUG_ACTION(std::cout << "creating a new buffer at: " << i << std::endl);

		// couldn't find a suitable one -> create a new one
		return allocateObjectBuffer(texture_id);
	}

	inline void loadIcons(IconsLoader& loader) {
		
		for (auto&& it = loader.iconsBegin(); it != loader.iconsEnd(); ++it) {
			auto&& textureEntry = *it;	// should be IconEntry (or like it)
			
			// find the texture for the icon
			auto&& texture = texture_manager.getTexture(textureEntry.textureAlias);

			DEBUG_ACTION(std::cout << "Storing icon: " << textureEntry.alias << " with " << textureEntry.pos.x << ' ' << textureEntry.pos.y << ' ' << textureEntry.dim.x << ' ' << textureEntry.dim.y);
			
			// based on constness of IconAlias_type steal or copy the alias name
			if constexpr (std::is_const_v<IconAlias>) {
				icon_manager.addIcon(textureEntry.alias,
					createIconFrom(texture.id, texture.width, texture.height, textureEntry.pos.x, textureEntry.pos.y, textureEntry.dim.x, textureEntry.dim.y)
				); // copy const
				DEBUG_ACTION(std::cout << " copying name ");
			}
			else {
				icon_manager.addIcon(std::move(textureEntry.alias),
					createIconFrom(texture.id, texture.width, texture.height, textureEntry.pos.x, textureEntry.pos.y, textureEntry.dim.x, textureEntry.dim.y)
				); // move non const
				DEBUG_ACTION(std::cout << " stealing name ");
			}

			DEBUG_ACTION(std::cout << " -> stored" << std::endl);
		}
		
	}


	bool cameraMoving = false;

	void mouseMoved(float x, float y) {
		mouseInfo.update(x, y);

		if (paused)
			return;

		if (cameraMoving) {
			camera.move({ mouseInfo.dx, -mouseInfo.dy, 0 });
		}
	}

	inline void processMouseWheel(float x, float y) {
		if (paused)
			return;
		
		auto scaleLevel = camera.getScale();

		// faster the closer we are
		float val = y * ((scaleLevel - MAX_ZOOM) * (scaleLevel + MAX_ZOOM - 2 * MIN_ZOOM) * ZOOM_FACTOR + MAX_ZOOM_SPEED);
		
		// make sure to limit the zoom (from constants)
		float newVal = std::max(std::min(scaleLevel + val, MAX_ZOOM), MIN_ZOOM);

		camera.setScale(newVal);
	}

	bool paused = false;

	// -------------------------------------------

	bool selected = false;
	
	GLuint shaderProgramSelection;
	
	GLint u_view2;
	GLint u_projection2;
	GLint u_selectionCenter;
	GLint u_selectionRadius;
	GLint u_time;

	std::unique_ptr<SelectionBuffer> selection;

	void prepareSelectionBuffer() {
		
#if defined(DEBUG) || defined(_DEBUG)
		shaderProgramSelection = renderer.createShaderFrom("../shaders/tabletop_selection_vertex.vert.glsl", "../shaders/tabletop_selection_fragment.frag.glsl");
#else
		shaderProgramSelection = renderer.createShaderFromSource(selectionVertexShader, selectionFragmentShader);
#endif // DEBUG

		
		
		auto vao = renderer.createVAO();
		renderer.modifyVAO(vao);

		renderer.setPointer(quad_buffer, 0, 3, GL_FLOAT, 5 * sizeof(float), 0, false); // x, y, z
		
		auto pos = renderer.createBuffer(sizeof(glm::vec3) * DEFAULT_ARRAY_SIZE);
		renderer.setPointer(pos, 1, 3, GL_FLOAT, 3 * sizeof(float), 0, true);

		auto dim = renderer.createBuffer(sizeof(glm::vec2) * DEFAULT_ARRAY_SIZE);
		renderer.setPointer(dim, 2, 2, GL_FLOAT, 2 * sizeof(float), 0, true);

		renderer.stopModifyVAO();

		selection = std::make_unique<SelectionBuffer>(vao, pos, dim);
		
		u_view2 = renderer.getUniform(shaderProgramSelection, "u_view");
		u_projection2 = renderer.getUniform(shaderProgramSelection, "u_projection");
		u_selectionCenter = renderer.getUniform(shaderProgramSelection, "u_selectionCenter");
		u_selectionRadius = renderer.getUniform(shaderProgramSelection, "u_selectionRadius");
		u_time = renderer.getUniform(shaderProgramSelection, "u_time");

	}

public:

	GameGraphicsFacade(
		RenderAPI& renderer,
		TextureManager<TextureAliasRaw>& texture_manager)
		:
		renderer(renderer), texture_manager(texture_manager)
	{
		this->renderer.addRenderingPass([this]() {this->gameRenderLoop(); });
		this->renderer.addWindowResize([this](int width, int height) {this->windowResize(width, height); });
		this->renderer.addMouseMove([this](float x, float y) {this->mouseMoved(x, y); });
		this->renderer.addMouseWheel([this](float x, float y) {this->processMouseWheel(x, y);  });

		// load icons
		IconsLoader il;
		loadIcons(il);

		// quad geometry
		const float vertices[] = {
			//    X     Y   Z  U  V
				-0.5, -0.5, 0, 0, 1,
				 0.5, -0.5, 0, 1, 1,
				-0.5,  0.5, 0, 0, 0,

				 0.5, -0.5, 0, 1, 1,
				 0.5,  0.5, 0, 1, 0,
				-0.5,  0.5, 0, 0, 0
		};

		quad_buffer = renderer.createBuffer(sizeof(vertices), &vertices);
		// all_managed_buffers.push_back(quad_buffer);	// keep out of for easier reset manipulation

#if defined(DEBUG) || defined(_DEBUG)
		shaderProgram = renderer.createShaderFrom("../shaders/tabletop_vertex.vert.glsl", "../shaders/tabletop_fragment.frag.glsl");
#else
		shaderProgram = renderer.createShaderFromSource(mainVertexShader, mainFragmentShader);
#endif // DEBUG

		prepareSelectionBuffer();

		// uniforms
		u_view = renderer.getUniform(shaderProgram, "u_view");
		u_projection = renderer.getUniform(shaderProgram, "u_projection");
		u_texture = renderer.getUniform(shaderProgram, "u_texture");

		camera.setPosition(glm::vec3(0, 0, 1));
		camera.setScale(100.0f);
		auto dim = renderer.size();
		camera.setOrthoDims((float) dim.x, (float) dim.y);

		// enable transparency
		GL_CHECK(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
		glEnable(GL_BLEND);
	}


	glm::vec2 getWorldPosition(float screenX, float screenY) {
		// camera pos is always middle of the screen
		auto dim = renderer.size();
		auto camPos = camera.get2DPosition();
		auto camScale = camera.getScale();

		float X = ((screenX - dim.x / 2.0f) + camPos.x) / camScale;
		float Y = ((dim.y - screenY - dim.y / 2.0f) + camPos.y) / camScale;	// flip vertically (up is down)

		return { X , Y };
	}
	

	void addMouseInput(std::function<void(int, int, int, float, float)> callback) {
		renderer.addMouseInput(callback);
	}

	bool processMouseInput(int key, int action, int mods, float posx, float posy) {
		if(key == GLFW_MOUSE_BUTTON_RIGHT) {
			// when clicked -> begin scrolling (moving the board = camera)
			cameraMoving = (action == GLFW_PRESS);
			// when released -> stop
			return true;
		}

		return false;
	}

	void pause() {
		paused = true;
	}

	void unpause() {
		paused = false;
	}



	// -------------------------------------------------------------------------------

	GameObject addObject(glm::vec3 position, glm::vec2 dimensions, const IconAliasRaw& icon_name)
	{
		// find a buffer based on the texture id
		auto& icon = icon_manager.getIcon(icon_name);

		auto& buffer = getBufferFor(icon.texture_id);
		
		GameObject object(buffer.vao_id, buffer.count);
		
		// setup object properties
		renderer.modifyBuffer(buffer.positions, object.identifier * sizeof(glm::vec3), 1 * sizeof(glm::vec3), &position);
		renderer.modifyBuffer(buffer.dimensions, object.identifier * sizeof(glm::vec2), 1 * sizeof(glm::vec2), &dimensions);
		renderer.modifyBuffer(buffer.uv_positions, object.identifier * sizeof(glm::vec2), 1 * sizeof(glm::vec2), &icon.pos);
		renderer.modifyBuffer(buffer.uv_dimensions, object.identifier * sizeof(glm::vec2), 1 * sizeof(glm::vec2), &icon.dim);

		buffer.count++;

		return object;
	}
	
	// ------------------------------------------------------------------------------------------

	void moveObject(const GameObject& object, glm::vec3 where) {
		// find the appropriate buffer
		auto&& buffer = findObjectBuffer(object);

		// modify position
		renderer.modifyBuffer(buffer.positions, object.identifier * sizeof(glm::vec3), 1 * sizeof(glm::vec3), &where);
	}

	void changeIcon(const GameObject& object, const IconAliasRaw& iconName) {
		// find the buffer
		auto&& buffer = findObjectBuffer(object);

		auto&& icon = icon_manager.getIcon(iconName);

		// modify uv = icon
		renderer.modifyBuffer(buffer.uv_positions, object.identifier * sizeof(glm::vec2), 1 * sizeof(glm::vec2), &icon.pos);
		renderer.modifyBuffer(buffer.uv_dimensions, object.identifier * sizeof(glm::vec2), 1 * sizeof(glm::vec2), &icon.dim);
	}

	void changeScale(const GameObject& object, glm::vec2 newScale) {
		// find the buffer
		auto&& buffer = findObjectBuffer(object);

		// modify scale
		renderer.modifyBuffer(buffer.dimensions, object.identifier * sizeof(glm::vec2), 1 * sizeof(glm::vec2), &newScale);

	}

	
	
	// -----------------------------------------------------------------------------------------

	// Draws  
	void select(glm::vec2 position, glm::vec2 size) {
		
		glm::vec3 p(position.x, position.y, 0);
		glm::vec2 s(size.x * 2 * SCALE_MULTIPLIER, size.y * 2 * SCALE_MULTIPLIER);

		renderer.modifyBuffer(selection->positions, 0, 1 * sizeof(glm::vec3), &p);
		renderer.modifyBuffer(selection->dimensions, 0, 1 * sizeof(glm::vec2), &s);
		
		selected = true;

		selection->set({ p.x, p.y }, std::min(size.x, size.y) * 0.5f * SCALE_MULTIPLIER);
	}

	void deselect() {
		selected = false;
	}

	const glm::vec2& currentSelection() {
		return selection->getPos();
	}
};