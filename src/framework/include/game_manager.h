#pragma once

#include <vector>
#include <memory>
#include <utility>
#include <unordered_map>

#include "game_object.h"
#include "game_graphics_facade_OpenGL.h"
#include "playing_board.h"
#include "game_elements.h"
#include "interactables.h"
#include "framework_exceptions.h"


struct GameSettings {

	// The number of players currently playing the game
	std::size_t currentPlayers;

	// If the number of players playing is not positive (user specified this), 
	// then automatic turn progression onto the next player won't work properly.
	GameSettings(std::size_t currentPlayers) : currentPlayers(currentPlayers) 
	{}
};


template<typename FrameworkProperties>
class GameManager {
	
	using Types = FrameworkTypes<FrameworkProperties>;
	using EventSystem = Types::EventSystem;
	using BoardProperties = Types::BoardProperties;
	using PieceAlias = Types::PieceAlias;
	using PieceProperties = Types::PieceProperties;
	using TileProperties = Types::TileProperties;
	using IconAliasRaw = Types::IconAliasRaw;
	using Pathfinder = Types::Pathfinder;
	using EdgeValidator = Types::EdgeValidator;

private:
	
	// how things actually get done 
	std::unique_ptr<GameGraphicsFacade<FrameworkProperties>> gameGraphicsFacade;
	
	std::unique_ptr<PlayingBoard<BoardProperties>> playingBoard;
	std::vector<Player<FrameworkProperties>> players;

	// board tiles to be clicked at
	std::unique_ptr<ClickableManager<glm::ivec2>> tilePieces;	

	// event callbacks for and from the user
	EventSystem* events = nullptr;

	// Lazy iterator like implementation (doesn't fully implement the iterator for ease)
	// for converting board tiles to clickable regions
	class BoardTileToRegionPasser {
	private:

		const std::unique_ptr<PlayingBoard<BoardProperties>>& playingBoard;

		class Iterator {
			using BoardIterator = typename PlayingBoard<BoardProperties>::IteratorType;
		private:
			BoardIterator boardIt;

		public:
			Iterator(BoardIterator boardIt) : boardIt(boardIt) {}

			auto operator*() {
				auto&& value = *boardIt;
				
				// the board position is centered, but clickable needs the position to be bottom left corner				
				return ClickableRegion<glm::ivec2>(value.position-value.dimensions/2.0f, value.dimensions, value.boardCoordinates);
			}

			Iterator& operator++() {
				++boardIt;
				return *this;
			}

			bool operator==(const Iterator& other) const {
				return boardIt == other.boardIt;
			}

			bool operator!=(const Iterator& other) const {
				return !(*this == other);
			}

		};

	public:

		BoardTileToRegionPasser(const std::unique_ptr<PlayingBoard<BoardProperties>>& playingBoard)
			: playingBoard(playingBoard) {}

		Iterator begin() {

			return Iterator(playingBoard->tilesBegin());
		}

		Iterator end() {
			return Iterator(playingBoard->tilesEnd());
		}
	};


	bool paused = false;

	void processMouseInput(int key, int action, int mods, float posx, float posy) {

		// don't allow clicking on tiles when paused
		if (paused)
			return;
		
		if (gameGraphicsFacade->processMouseInput(key, action, mods, posx, posy))
			return;

		// in the future could have drag and drop or click only on release
		// for now only act on press and always act
		if (action != GLFW_PRESS)
			return;

		// convert mouse clip to world coordinates
		auto pos = gameGraphicsFacade->getWorldPosition(posx, posy);
		
		switch (key)
		{
		case GLFW_MOUSE_BUTTON_LEFT: {
			auto&& tile = tilePieces->isAt(pos.x, pos.y);
			if (tile) {				
				clickAtTile(tile->x, tile->y);
			}
			// event that mouse was clicked elsewhere
			else {
				// if a piece was selected -> reset (clicked outside the board)
				deselectPiece();
				events->mousePressedAt(pos.x, pos.y);
			}
			break;
		}
		default:
			break;
		}

		DEBUG_ACTION(std::cout << "World pos:" << pos.x << " " << pos.y << std::endl);

	}

	// ----------------------------

	GameSettings currentSettings;

	std::size_t currentPlayerID = 0;	
	int currentCost = 0;	

	Piece<FrameworkProperties>* pieceSelection = nullptr;

	Pathfinder pathfinder;

	// ----------------------------

	// beginning of each player's turn
	void turnBegin() {
		
		events->turnBegin();
		std::cout << "Player " << currentPlayerID << "'s turn begin." << std::endl;
	}

	// beginning of each game
	void gameBegin() {
		
		currentPlayerID = events->gameBegin();
		currentCost = 0;

		deselectPiece();

		turnBegin();
	}

	bool validPlayer(std::size_t id) const {
		return id >= 0 && id < players.size();
	}

	Player<FrameworkProperties>& getPlayer(std::size_t id) {
		if (validPlayer(id)) {
			return players.at(id);
		}
		else {
			throw InvalidArgument<std::size_t>::invalidPlayerId(id);
		}
	}

	const Player<FrameworkProperties>& getPlayer(std::size_t id) const {
		if (validPlayer(id)) {
			return players.at(id);
		}
		else {
			throw InvalidArgument<std::size_t>::invalidPlayerId(id);
		}
	}

	// ----------------------------------------------------------------------

	void selectPiece(Piece<FrameworkProperties>& piece) {

		pieceSelection = &piece;
		gameGraphicsFacade->select(piece.getPosition(), piece.getSize());
	}

public:

	GameManager(
		std::unique_ptr<GameGraphicsFacade<FrameworkProperties>>&& gameFacade) : 

		gameGraphicsFacade(std::move(gameFacade)),
		currentSettings(0)
	{
		playingBoard = std::make_unique<PlayingBoard<BoardProperties>>(*gameGraphicsFacade.get(), EdgeValidator(GameContext<FrameworkProperties>(*this)));
		
		// enable clicking on tile pieces
		tilePieces = std::make_unique<ClickableManager<glm::ivec2>>(BoardTileToRegionPasser(playingBoard));
	
		// detecting clicking on tiles
		gameGraphicsFacade->addMouseInput([this](int key, int action, int mods, float posx, float posy) { 
			processMouseInput(key, action, mods, posx, posy); });
	}
	
	PlayingBoard<BoardProperties>& getPlayingBoard() {
		return *playingBoard.get();
	}

	Pathfinder& getPathfinder() {
		return pathfinder;
	}

	const Pathfinder& getPathfinder() const {
		return pathfinder;
	}

	int getCurrentCost() const {
		return currentCost;
	}

	void setCost(int newCost) {
		currentCost = newCost;
	}

	std::size_t getCurrentPlayer() const {
		return currentPlayerID;
	}

	void setCurrentPlayer(std::size_t playerId) {
		currentPlayerID = playerId % currentSettings.currentPlayers;
	}

	auto& getPlayerProperties(std::size_t playerId) {
		return getPlayer(playerId).getProperties();
	}

	const auto& getPlayerProperties(std::size_t playerId) const {
		return getPlayer(playerId).getProperties();
	}

	auto& getPieceProperties(std::size_t playerId, const PieceAlias& alias) {
		return getPlayer(playerId).getPiece(alias).getProperties();
	}

	const auto& getPieceProperties(std::size_t playerId, const PieceAlias& alias) const {
		return getPlayer(playerId).getPiece(alias).getProperties();
	
	}

	const auto& getPiecePosition(std::size_t playerId, const PieceAlias& alias) const {
		return getPlayer(playerId).getPiece(alias).getPosition();
	}

	const auto& getPieceSize(std::size_t playerId, const PieceAlias& alias) const {
		return getPlayer(playerId).getPiece(alias).getSize();
	}

	// -----------------------------------------------------------------------------------------------	

	void createPiece(
		std::size_t playerId, 
		const PieceAlias& alias,
		PieceProperties&& properties,

		glm::vec2 position, 
		glm::vec2 size, 
		const IconAliasRaw& iconName
		) {

		getPlayer(playerId).addPiece(alias, std::move(properties), position, size, iconName);
		// !! deselect piece !! - reference might be invalidated
		deselectPiece();

		events->pieceCreatedAt(playerId, alias, position);
	}

	void createPieceAt(
		std::size_t playerId,
		const PieceAlias& alias,
		PieceProperties&& properties,

		int x, int y, 
		glm::vec2 size, 
		const IconAliasRaw& iconName
		) {

		auto&& tile = playingBoard->getTile(x, y);
		auto&& piece = getPlayer(playerId).addPieceAt(alias, std::move(properties), tile, size, iconName);
		// !! deselect piece !! - reference might be invalidated
		deselectPiece();

		events->pieceCreatedAt(playerId, alias, tile.boardCoordinates);
	}

	void movePieceTo(
		std::size_t playerId,
		const PieceAlias& pieceAlias,

		int x, int y
	) {
		
		auto&& tile = playingBoard->getTile(x, y);
		getPlayer(playerId).getPiece(pieceAlias).moveTo(&tile);

		events->pieceMovedTo(playerId, pieceAlias, tile.boardCoordinates);
	}

	void movePiece(
		std::size_t playerId,
		const PieceAlias& pieceAlias,

		const glm::vec2& position
	) {

		getPlayer(playerId).getPiece(pieceAlias).moveTo(position);

		events->pieceMovedTo(playerId, pieceAlias, position);
	}
	
	void setPieceVisibility(
		std::size_t playerId,
		const PieceAlias& pieceAlias,

		bool visible
	) {
		
		getPlayer(playerId).getPiece(pieceAlias).setVisible(visible);
		
		events->pieceVisibilityChanged(playerId, pieceAlias, visible);
	}

	void setPieceIcon(
		std::size_t playerId,
		const PieceAlias& pieceAlias,

		const IconAliasRaw& icon
	) {
		
		getPlayer(playerId).getPiece(pieceAlias).setIcon(icon);

		events->pieceIconChanged(playerId, pieceAlias, icon);
	}

	void setPieceSize(
		std::size_t playerId,
		const PieceAlias& pieceAlias,

		const glm::vec2& size) 
	{
		
		getPlayer(playerId).getPiece(pieceAlias).setSize(size);
		
		events->pieceSizeChanged(playerId, pieceAlias, size);
	}

	// -----------------------------------------------------------------------------------------------	

	bool isPieceAtTile(std::size_t playerId, const PieceAlias& alias) {
		auto&& piece = getPlayer(playerId).getPiece(alias);
		return piece.isAtTile();
	}

	glm::ivec2 getTilePosition(std::size_t playerId, const PieceAlias& alias) const {
		auto&& piece = getPlayer(playerId).getPiece(alias);
		return piece.getTilePosition();
	}

	bool isPieceSelected() {
		return (pieceSelection != nullptr);
	}

	bool selectPiece(
		std::size_t playerId, 
		const PieceAlias& pieceAlias) 
	{
		// !! select only if that piece is at a tile !! 
		auto&& piece = getPlayer(playerId).getPiece(pieceAlias);
		if (piece.isAtTile()) {
			selectPiece(piece);
			return true;
		}

		return false;
	}

	void deselectPiece() {
		if (pieceSelection)
			pieceSelection = nullptr;
		
		gameGraphicsFacade->deselect();
	}

	// -----------------------------------------------------------------------------------------------	

	bool playerPieceOnTile(std::size_t playerId, const glm::ivec2& tile) const {
		return getPlayer(playerId).getPieceAt(tile.x, tile.y).isValid();
	}

	bool anyPieceOnTile(const glm::ivec2& tile) const {
		for (std::size_t id= 0; id< players.size(); ++id)
		{
			if (playerPieceOnTile(id, tile))
				return true;
		}

		return false;
	}

	const PieceAlias& getPieceAt(std::size_t playerId, const glm::ivec2& tile) {
		return getPlayer(playerId).getPieceAt(tile.x, tile.y)->getId();
	}
	
	auto piecePropertiesBegin(std::size_t playerId) {
		return getPlayer(playerId).piecePropertiesBegin();
	}

	auto piecePropertiesEnd(std::size_t playerId) {
		return getPlayer(playerId).piecePropertiesEnd();
	}

	auto piecesBegin(std::size_t playerId) const {
		return getPlayer(playerId).begin();
	}

	auto piecesEnd(std::size_t playerId) const {
		return getPlayer(playerId).end();
	}

	// ----------------------------------------------------------------------------

	void pause() {
		paused = true;
		gameGraphicsFacade->pause();
	}

	void unpause() {
		paused = false;
		gameGraphicsFacade->unpause();
	}

	void over() {
		events->gameOver();
	}


	// Called by the framework at the start only once
	void init(EventSystem* events, size_t MAX_PLAYERS) {
		this->events = events;

		// setup players
		for (std::size_t i = 0; i < MAX_PLAYERS; ++i) {
			players.emplace_back(*gameGraphicsFacade.get());
		}

		events->init();
	}

	// Called by the framework when the game is reset in game
	void reset() {
		
		// reset pieces and move them from tiles (= reset)
		for (auto&& player : players) {
			for (auto&& pieceAlias : player) {
				player.getPiece(pieceAlias).reset();
			}
		}

		std::cout << "Game reset." << std::endl;

		gameBegin();
	}

	// Called by the framework when the game is started from the menu or restarted
	void restart(GameSettings settings) {

		// update settings
		currentSettings = settings;

		for (auto&& player : players) {
			for (auto&& pieceAlias : player) {
				player.getPiece(pieceAlias).reset();
			}
		}
		
		// hide inactive players
		for (std::size_t playerIndex = currentSettings.currentPlayers; playerIndex < players.size(); ++playerIndex) {
			for (auto&& pieceAlias : players[playerIndex]) {
				players[playerIndex].getPiece(pieceAlias).setVisible(false);
			}
		}

		events->restart();
		
		gameBegin();
	}


	// Called by the framework when the player requests to generate (roll)
	void generateCost() {

		if (paused)
			return;

		if (events->canGenerateCost()) {
			currentCost += events->generateCost();

			std::cout << "Player " << currentPlayerID << " has " << currentCost << std::endl;
		}
	}
	
	// Called by the framework when the player requests to end their turn
	void endTurn() {

		if (paused)
			return;

		if (events->canEndTurn()) {
			
			events->turnEnd();

			std::cout << "Player " << currentPlayerID << "'s turn ended." << std::endl;

			// begin next turn
			currentPlayerID = (currentPlayerID + 1) % currentSettings.currentPlayers;
			deselectPiece();
			
			turnBegin();	
		}

	}

	void tick() {
		
		if (pieceSelection)
			if (gameGraphicsFacade->currentSelection() != pieceSelection->getPosition())
				gameGraphicsFacade->select(pieceSelection->getPosition(), pieceSelection->getSize());
	}

	void clickAtTile(int x, int y) {

		// ask the user if the tile click should be processed by the framework
		if (events->shouldProcessTilePress(x, y)) {

			// check if there's a piece at this tile that the current player owns
			// doing it this way, because in the future it would be possible to have multiple pieces at the same
			// tile, and having a [tile map -> pieces] would be troublesome

			// clicked at a piece before 
			if (pieceSelection) {
				// try to find a path from that piece to this point
				auto path = pathfinder.getPathBetween(
					pieceSelection->getTilePosition(),
					glm::ivec2{ x, y },
					PathfinderContext<FrameworkProperties>(*this));


				// found a path
				// 
				// trigger events for each tile at that path by this player and this piece	
				// pass context for any additional operations that the user might want to do

				// finally move the piece to the last tile, but first trigger a move event

				// finally move this piece to that last tile, removing it from the current one 
				if (path.getLength() != 0) {
					bool skip = false;
					int pathCost = 0;
					auto currentPos = pieceSelection->getTilePosition();
					for (auto&& tilePos : path) {

						// if false -> user requested to stop the movement
						if (!events->passTile(pieceSelection->getId(), tilePos))
						{
							skip = true;
							break;
						}
						else {
							// cumulative cost for all edges
							pathCost += playingBoard->getEdge(currentPos, tilePos).cost;
							currentPos = tilePos;
						}
					}

					// user requested to stop the move
					if (!skip) {
						auto&& lastPos = *(path.last());

						// last chance for the user to stop the move
						if (events->movePieceToTile(pieceSelection->getId(), lastPos)) {

							auto&& tile = playingBoard->getTile(lastPos);
							pieceSelection->moveTo(&tile);

							currentCost -= pathCost;

							events->pieceMovedTo(currentPlayerID, pieceSelection->getId(), lastPos);
						}

					}
				}

			}
			else {

				if (events->canSelectPiece()) {
					// check if there's a piece at this tile -> select it
					// NOTE: no support (currently) for multiple pieces (of the current player) at the same tile
					auto pieceCarrier = players[currentPlayerID].getPieceAt(x, y);
					if (pieceCarrier.isValid()) {
						selectPiece(*pieceCarrier);
						DEBUG_ACTION(std::cout << "selected piece: " << pieceSelection->getId() << std::endl);
					}
				}
			}

		}
	
		events->mousePressedAtTile(x, y);
	}

};