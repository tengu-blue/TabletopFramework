#pragma once

#include <iostream>
#include <memory>
#include <fstream>
#include <type_traits>

#include <glm/glm.hpp>

#include "my_utils.h"
#include "string_utils.h"
#include "error_handling_OpenGL.h"
#include "game_graphics_facade_OpenGL.h"
#include "game_manager.h"

#include "game_loading_utils.h"
#include "game_loading.h"

#include "framework_contexts.h"
#include "framework_exceptions.h"

struct FrameworkSettings {
	std::string title;								// the window title

	std::filesystem::path workingDirectory;			// base path used for relative paths

	size_t MIN_PLAYERS, MAX_PLAYERS;				

	template <typename Title, typename Work>
	FrameworkSettings(
		Title&& title,
		Work&& workingDirectory,	
		size_t MIN_PLAYERS,
		size_t MAX_PLAYERS) : 

		title(std::forward<Title>(title)),
		workingDirectory(std::forward<Work>(workingDirectory)),
		
		MIN_PLAYERS(MIN_PLAYERS),
		MAX_PLAYERS(MAX_PLAYERS)
	{
		if (MIN_PLAYERS == 0)
			throw InvalidSetting("The minimum number of players cannot be 0!");

		if (MAX_PLAYERS < MIN_PLAYERS)
			throw InvalidSetting("The maximum number of players cannot be smaller than the minimum!");
	}
};


template<FrameworkConcept FrameworkProperties>
class TabletopFramework {

	using Types = FrameworkTypes<FrameworkProperties>;
	using EventSystem = Types::EventSystem;
	using TexturesLoader = Types::TexturesLoader;
	using TextureAliasRaw = Types::TextureAliasRaw;
	using TextureAlias = Types::TextureAlias;


private:

	enum State
	{
		INIT,		// initial state 
		MENU,
		RUNNING,	// once initialized -> can be started -> is running
		PAUSED,		// pause button + menus -> doesn't update game manager probably
		OVER,		// when the game ends -> like paused but cannot be unpaused
		DEAD,		// after game is over -> framework becomes DEAD - cannot be restarted
		FAIL		// if there's an exception during creation -> FAIL state - cannot continue
	};

	State currentState = INIT;
	std::unique_ptr<EventSystem> events;

	// --------------------------------------------------------------------

	std::unique_ptr<RenderAPI> renderer;
	std::unique_ptr<TextureManager<TextureAliasRaw>> texture_manager;

	std::unique_ptr<GameManager<FrameworkProperties>> gameManager;

	// --------------------------------------------------------------------
	
	void loadTextures(TexturesLoader& loader) {
		for (auto&& it = loader.texturesBegin(); it != loader.texturesEnd(); ++it) {
			auto&& textureEntry = *it;

			std::filesystem::path texturePath = textureEntry.texturePath;
			if (texturePath.is_relative())
				texturePath = settings.workingDirectory / texturePath;

			if (!std::filesystem::exists(texturePath))
				throw ResourceDescriptionError("Texture not found. Cannot proceed!", texturePath.string());

			DEBUG_ACTION(std::cout << "loading: " << textureEntry.alias << " " << texturePath.string());

			// create the texture from the loaded file
			int w = 0, h = 0;
			auto id = renderer->generateTexture(texturePath, w, h);
			if constexpr (std::is_const_v<TextureAlias>) {
				texture_manager->addTexture(textureEntry.alias, id, w, h);				// copy const
				DEBUG_ACTION(std::cout << " copying name ");
			}
			else {
				texture_manager->addTexture(std::move(textureEntry.alias), id, w, h);	// move non const
				DEBUG_ACTION(std::cout << " stealing name ");
			}

			DEBUG_ACTION(std::cout << " -> loaded" << std::endl);
		}
	}

	void end() {
		renderer->terminate();
		changeState(DEAD);
	}

	FrameworkSettings settings;

	void restart() {
		
		std::cout << "Enter how many players will be playing this game: ";

		int val;
		std::cin >> val;
		while (val < settings.MIN_PLAYERS || val > settings.MAX_PLAYERS)
		{
			if (std::cin.fail()) {
				std::cin.clear(); 
				std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
			}

			std::cout << "The number entered is not within allowed bounds! [" << settings.MIN_PLAYERS << ", " << settings.MAX_PLAYERS << "] Enter again : ";
			std::cin >> val;
		}

		std::cout << "Starting with " << val << " players." << std::endl;

		changeState(RUNNING);
		gameManager->restart(GameSettings(val));
	}


	// ------------------------------------------------------------------------

	
	void changeState(State newState) {
		
		DEBUG_ACTION(std::cout << "changing to " << newState << std::endl);

		if (newState == PAUSED) {
			gameManager->pause();
			std::cout << "Game paused. Press [SPACE - to un-pause] [R - reset] [M - menu]" << std::endl;
		}
		else if (newState == RUNNING) {
			gameManager->unpause();
			std::cout << "Game un-paused. Press [SPACE - to pause] [D - to roll] [S - end turn]" << std::endl;
		}

		if (newState == OVER) {
			gameManager->over();
			std::cout << "Game paused. Press [R - reset] [M - menu]" << std::endl;
		}

		currentState = newState;

	}
	
	void processKeyAction(int key, int scancode, int action, int mods) {

		DEBUG_ACTION(std::cout << "key event: " << key << std::endl );

		// act only on press
		if (action != GLFW_PRESS)
			return;

		switch (key)
		{

		// pause - unpause
		case GLFW_KEY_SPACE: {
			if (currentState == RUNNING)
				changeState(PAUSED);
			else if (currentState == PAUSED) {
				changeState(RUNNING);
			}
			break;
		}
		
		// when paused -> reset
		case GLFW_KEY_R: {
			if (currentState == PAUSED || currentState == OVER)
			{
				changeState(RUNNING);
				gameManager->reset();
			}
			
			break;
		}

		// when paused -> menu
		case GLFW_KEY_M: {
			if (currentState == PAUSED || currentState == OVER)
			{
				changeState(MENU);
			}

			break;
		}

		case GLFW_KEY_D: {
			gameManager->generateCost();

			break;
		}

		case GLFW_KEY_S: {
			gameManager->endTurn();

			break;
		}

		default:
			break;
		}

	}

public:

	
	// init
	TabletopFramework(int width, int height, FrameworkSettings&& frameworkSettings) :
		settings(std::move(frameworkSettings)) {
		
		try {
			renderer = std::make_unique<RenderAPI>(width, height, settings.title);

			texture_manager = std::make_unique<TextureManager<TextureAliasRaw>>();
			
			TexturesLoader tl;
			loadTextures(tl);


			auto facade = std::make_unique<GameGraphicsFacade<FrameworkProperties>>(
				*renderer.get(),
				*texture_manager.get());
			
			gameManager = std::make_unique<GameManager<FrameworkProperties>>(
				std::move(facade));	

			renderer->addKeyEvent([this](int key, int scancode, int action, int mods) { processKeyAction(key, scancode, action, mods); });

			events = std::make_unique<EventSystem>(GameContext<FrameworkProperties>(*gameManager.get()));
			gameManager->init(events.get(), settings.MAX_PLAYERS);

			changeState(MENU);

		}
		catch (OpenGLError& exc) {
			std::cerr << "OpenGL error: " << exc.what() << std::endl;
			currentState = FAIL;
		}
		catch (ResourceDescriptionError& exc) {
			std::cerr << "Error loading resource: " << exc.what() << std::endl;
			currentState = FAIL;
		}
		catch (std::exception& exc) {
			std::cerr << "Error: " << exc.what() << std::endl;
			currentState = FAIL;
		}

	}

	bool isValid() {
		return currentState != FAIL && currentState != DEAD;
	}

	// -------------------------------------------------------------------

	void start() {

		if(currentState == DEAD)
			throw std::runtime_error("The framework cannot be started again.");

		if (currentState == FAIL)
			throw std::runtime_error("The framework cannot be started because of initialization failure.");

		try {
			while (!renderer->shouldClose()) {
				
				if (currentState == MENU)
					restart();
				
				if (currentState == RUNNING) {
					gameManager->tick();
					
					if(events->isGameOver()) {
						changeState(OVER);
					}
				}
				renderer->renderingTick();
			}
		}
		catch (OpenGLError& exc) {
			std::cerr << "OpenGL error: " << exc.what() << std::endl;
		}
		catch (std::exception& exc) {
			std::cerr << "Error: " << exc.what() << std::endl;
		}
			
		end();
	}

};