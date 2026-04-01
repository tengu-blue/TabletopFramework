#pragma once
#include <type_traits>
#include "game_loading_utils.h"


template<FrameworkConcept FrameworkProperties>
class TabletopFramework;

template<typename FrameworkProperties>
class GameManager;

template<typename BoardProperties>
class PlayingBoard;


template<typename FrameworkProperties>
class GameContext {
	
	using Types = FrameworkTypes<FrameworkProperties>;
	using IconAliasRaw = typename Types::IconAliasRaw;
	using PieceAlias = typename Types::PieceAlias;
	using PieceProperties = typename Types::PieceProperties;
	using TileProperties = typename Types::TileProperties;
	using BoardProperties = typename Types::BoardProperties;

private:

	GameManager<FrameworkProperties>& gameManager;
	PlayingBoard<BoardProperties>& playingBoard;

public:

	GameContext(GameManager<FrameworkProperties>& gameManager) :
		gameManager(gameManager),
		playingBoard(gameManager.getPlayingBoard())
	{}

	std::size_t getCurrentPlayer() const {
		return gameManager.getCurrentPlayer();
	}

	int getCurrentCost() const {
		return gameManager.getCurrentCost();
	}

	void setCost(int newCost) {
		gameManager.setCost(newCost);
	}

	void setCurrentPlayer(std::size_t playerId) {
		gameManager.setCurrentPlayer(playerId);
	}

	auto& getEdgeValidator() {
		return playingBoard.getEdgeValidator();
	}

	const auto& getEdgeValidator() const {
		return playingBoard.getEdgeValidator();
	}

	auto& getPathfinder() {
		return gameManager.getPathfinder();
	}

	const auto& getPathfinder() const {
		return gameManager.getPathfinder();
	}

	// ------------------------------------------------------------------------------------------

	void createPiece(
		std::size_t playerID, 
		const PieceAlias& pieceAlias,
		PieceProperties&& pieceProperties,

		glm::vec2 position, 
		glm::vec2 size, 
		const IconAliasRaw& iconName) {
		
		gameManager.createPiece(playerID, pieceAlias, std::move(pieceProperties), position, size, iconName);
	}

	void createPieceAtTile(
		std::size_t playerID,
		const PieceAlias& pieceAlias,
		PieceProperties&& pieceProperties,

		int x, int y, 
		glm::vec2 size, 
		const IconAliasRaw& iconName) 
	{
		
		gameManager.createPieceAt(playerID, pieceAlias, std::move(pieceProperties), x, y, size, iconName);
	}

	void movePiece(
		std::size_t playerID,
		const PieceAlias& pieceAlias,

		const glm::vec2& position
	) {
		gameManager.movePiece(playerID, pieceAlias, position);
	}

	void movePieceToTile(
		std::size_t playerID,
		const PieceAlias& pieceAlias,

		int x, int y
	) {
		gameManager.movePieceTo(playerID, pieceAlias, x, y);
	}

	void setPieceVisibility(
		std::size_t playerID,
		const PieceAlias& pieceAlias, 
		
		bool visible
	) {
		gameManager.setPieceVisibility(playerID, pieceAlias, visible);
	}

	void setPieceIcon(
		std::size_t playerID,
		const PieceAlias& pieceAlias,
		const IconAliasRaw& icon
	) {
		gameManager.setPieceIcon(playerID, pieceAlias, icon);
	}

	// ------------------------------------------------------------------------------------------

	
	bool anyPieceOnTile(const glm::ivec2& tile) const {
		return gameManager.anyPieceOnTile(tile);
	}

	bool playerHasPieceOnTile(std::size_t playerId, const glm::ivec2& tile) const {
		return gameManager.playerPieceOnTile(playerId, tile);
	}

	const PieceAlias& getPieceAt(std::size_t playerId, const glm::ivec2& tile) {
		return gameManager.getPieceAt(playerId, tile);
	}
	
	bool isPieceAtTile(std::size_t playerId, const PieceAlias& piece) {
		return gameManager.isPieceAtTile(playerId, piece);
	}

	glm::ivec2 getTilePosition(std::size_t playerId, const PieceAlias& piece) const {
		return gameManager.getTilePosition(playerId, piece);
	}

	bool isPieceSelected() {
		return gameManager.isPieceSelected();
	}

	bool selectPiece(std::size_t playerId, const PieceAlias& piece) {
		return gameManager.selectPiece(playerId, piece);
	}

	void deselectPiece() {
		gameManager.deselectPiece();
	}
	


	// ------------------------------------------------------------------------------------------


	const auto& getPiecePosition(std::size_t playerId, const PieceAlias& piece) const {
		gameManager.getPiecePosition();
	}

	const auto& getPieceSize(std::size_t playerId, const PieceAlias& piece) const {
		gameManager.getPiecePosition();
	}


	auto& getPlayerProperties(std::size_t playerId) {
		return gameManager.getPlayerProperties(playerId);
	}

	const auto& getPlayerProperties(std::size_t playerId) const {
		return gameManager.getPlayerProperties(playerId);
	}

	TileProperties& getTileProperties(const glm::ivec2& tile) {
		return playingBoard.getTileProperties(tile);
	}

	const TileProperties& getTileProperties(const glm::ivec2& tile) const {
		return playingBoard.getTileProperties(tile);
	}

	PieceProperties& getPieceProperties(std::size_t playerId, const PieceAlias& alias) {
		return gameManager.getPieceProperties(playerId, alias);
	}

	const PieceProperties& getPieceProperties(std::size_t playerId, const PieceAlias& alias) const {
		return gameManager.getPieceProperties(playerId, alias);

	}

	// ---------------------------------------------------------------------

	
	auto tilePropertiesBegin() {
		return playingBoard.tilePropertiesBegin();
	}

	auto tilePropertiesEnd() {
		return playingBoard.tilePropertiesEnd();
	}

	auto tilePropertiesBegin() const {
		return playingBoard.tilePropertiesBegin();
	}

	auto tilePropertiesEnd() const {
		return playingBoard.tilePropertiesEnd();
	}

	auto piecePropertiesBegin(std::size_t playerId) {
		return gameManager.piecePropertiesBegin(playerId);
	}

	auto piecePropertiesEnd(std::size_t playerId) {
		return gameManager.piecePropertiesEnd(playerId);
	}

	auto piecesBegin(std::size_t playerId) const {
		return gameManager.piecesBegin(playerId);
	}

	auto piecesEnd(std::size_t playerId) const {
		return gameManager.piecesEnd(playerId);
	}

};


// ------------------------------------

// pathfinder context

template<typename FrameworkProperties>
class PathfinderContext {

	using BoardProperties = FrameworkProperties::GameProperties::BoardProperties;

private:
	
	GameManager<FrameworkProperties>& gameManager;
	PlayingBoard<BoardProperties>& playingBoard;

	PathfinderContext(
		GameManager<FrameworkProperties>& gameManager) :

		gameManager(gameManager), 
		playingBoard(gameManager.getPlayingBoard())
	{}

	friend class GameManager<FrameworkProperties>;


public:

	int getCost() const {
		return gameManager.getCurrentCost();
	}

	std::size_t getCurrentPlayer() const {
		return gameManager.getCurrentPlayer();
	}




	bool isValidEdgeBetween(const glm::ivec2& from, const glm::ivec2& to) {
		return playingBoard.isEdgeBetween(from, to);
	}

	int getEdgeCost(const glm::ivec2& from, const glm::ivec2& to) {
		return playingBoard.getEdge(from, to).cost;
	}

	bool anyPieceOnTile(const glm::ivec2& tile) const {
		return gameManager.anyPieceOnTile(tile);
	}
	
	bool playerHasPieceOnTile(std::size_t playerId, const glm::ivec2& tile) const {
		return gameManager.playerPieceOnTile(playerId, tile);
	}




	auto edgesFromBegin(const glm::ivec2 from) const {
		return playingBoard.edgesFromBegin(from);
	}

	auto edgesFromEnd(const glm::ivec2 from) const {
		return playingBoard.edgesFromEnd(from);
	}

};