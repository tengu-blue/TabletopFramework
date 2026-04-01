#pragma once

#include "game_loading_defaults.h"
#include <concepts>

// --------------------------------------------------------------------------------

template<typename BoardSettings>
struct EdgeValidator {
	using EdgeValidator_type = DefaultEdgeValidator;		// default
};

template<typename BoardSettings>
	requires requires { typename BoardSettings::EdgeValidator; }
struct EdgeValidator<BoardSettings>
{
	using EdgeValidator_type = BoardSettings::EdgeValidator;
};

// --------------------------------------------------------------------------------

template<typename GameSettings>
struct Pathfinder {
	using Pathfinder_type = DefaultPathFinder;		// default
};

template<typename GameSettings>
	requires requires { typename GameSettings::Pathfinder; }
struct Pathfinder<GameSettings>
{
	using Pathfinder_type = GameSettings::Pathfinder;
};



template<typename EventSystem, typename FrameworkProperties, typename PieceAlias, typename IconAlias>
concept EventSystemConcept = requires (
	GameContext<FrameworkProperties> context,
	EventSystem events,
	float f,
	int i,
	bool b,
	std::size_t playerId,
	const PieceAlias & pieceAlias,
	const glm::vec2 & vec2,
	const glm::ivec2 & ivec2,
	const IconAlias & iconAlias) {

	// ---------------------------------------------------------------------------

	EventSystem(context);									// constructor which takes the game context

	{ events.init() } -> std::same_as<void>;
	{ events.turnBegin() } -> std::same_as<void>;
	{ events.turnEnd() } -> std::same_as<void>;
	{ events.gameBegin() } -> std::same_as<std::size_t>;
	{ events.gameOver() } -> std::same_as<void>;
	{ events.restart() } -> std::same_as<void>;

	{ events.mousePressedAt(f, f) } -> std::same_as<void>;
	{ events.mousePressedAtTile(i, i) } -> std::same_as<void>;

	{ events.generateCost() } -> std::same_as<int>;

	{ events.isGameOver() } -> std::same_as<bool>;
	{ events.canGenerateCost() } -> std::same_as<bool>;
	{ events.canSelectPiece() } -> std::same_as<bool>;
	{ events.canEndTurn() } -> std::same_as<bool>;
	{ events.shouldProcessTilePress(i, i) } -> std::same_as<bool>;
	{ events.passTile(pieceAlias, ivec2) } -> std::same_as<bool>;
	{ events.movePieceToTile(pieceAlias, ivec2) } -> std::same_as<bool>;

	{ events.pieceCreatedAt(playerId, pieceAlias, vec2) } -> std::same_as<void>;
	{ events.pieceCreatedAt(playerId, pieceAlias, ivec2) } -> std::same_as<void>;
	{ events.pieceMovedTo(playerId, pieceAlias, vec2) } -> std::same_as<void>;
	{ events.pieceMovedTo(playerId, pieceAlias, ivec2) } -> std::same_as<void>;
	{ events.pieceVisibilityChanged(playerId, pieceAlias, b) } -> std::same_as<void>;
	{ events.pieceIconChanged(playerId, pieceAlias, iconAlias) } -> std::same_as<void>;
	{ events.pieceSizeChanged(playerId, pieceAlias, vec2) } -> std::same_as<void>;
};

template<typename T, typename ReturnType>
concept MinimalistIterator = requires(
	T it)
{
	{ ++it } -> std::same_as<T&>;
	{ *it } -> std::same_as<ReturnType&>;
	{ it != it } -> std::convertible_to<bool>;
};

template<typename TextureLoader>
concept TextureLoaderConcept = requires(
	TextureLoader loader) {

	TextureLoader();

	typename TextureLoader::TextureAlias_type;

	{ loader.texturesBegin() } -> MinimalistIterator<TextureEntry<raw_type<typename TextureLoader::TextureAlias_type>>>;

	{ loader.texturesEnd() } -> MinimalistIterator<TextureEntry<raw_type<typename TextureLoader::TextureAlias_type>>>;

};

template<typename IconLoader, typename TextureAliasRaw>
concept IconLoaderConcept = requires(
	IconLoader loader) {

	IconLoader();

	typename IconLoader::IconAlias_type;

	{ loader.iconsBegin() } -> MinimalistIterator<IconEntry<raw_type<typename IconLoader::IconAlias_type>, TextureAliasRaw>>;

	{ loader.iconsEnd() } -> MinimalistIterator<IconEntry<raw_type<typename IconLoader::IconAlias_type>, TextureAliasRaw>>;

};

template<typename BoardLoader, typename TileProperties>
concept BoardLoaderConcept = requires(
	BoardLoader loader) {
	
	BoardLoader();

	{ loader.tilesBegin() } -> MinimalistIterator<TileEntry<TileProperties>>;

	{ loader.tilesEnd() } -> MinimalistIterator<TileEntry<TileProperties>>;


	{ loader.edgesBegin() } -> MinimalistIterator<EdgeEntry>;

	{ loader.edgesEnd() } -> MinimalistIterator<EdgeEntry>;

};

template<typename EdgeValidator, typename FrameworkProperties>
concept EdgeValidatorConcept = requires(
	EdgeValidator validator,
	const Edge& edge,
	GameContext<FrameworkProperties> context) {
	
	EdgeValidator(context);

	{ validator.isEdgeValid(edge) } -> std::same_as<bool>;
};

template<typename Pathfinder, typename FrameworkProperties>
concept PathfinderConcept = requires(
	Pathfinder pathfinder,
	const glm::ivec2 & ivec2,
	PathfinderContext<FrameworkProperties> context) {

	Pathfinder();
	{ pathfinder.getPathBetween(ivec2, ivec2, context) } -> std::same_as<PiecePath>;

};


template<typename FrameworkProperties>
concept FrameworkConcept = requires(FrameworkProperties::GameProperties::BoardProperties board) {
	typename FrameworkProperties::EventSystem;
	typename FrameworkProperties::TextureLoader;
	typename FrameworkProperties::TextureLoader::TextureAlias_type;
	typename FrameworkProperties::GameProperties;
	typename FrameworkProperties::IconLoader;
	typename FrameworkProperties::IconLoader::IconAlias_type;
	typename FrameworkProperties::GameProperties::BoardProperties;
	typename FrameworkProperties::GameProperties::BoardProperties::TileProperties;
	typename FrameworkProperties::GameProperties::PlayerProperties;
	typename FrameworkProperties::GameProperties::PieceProperties;
	typename FrameworkProperties::GameProperties::PieceProperties::PieceAlias;

		requires EventSystemConcept<
			typename FrameworkProperties::EventSystem,
			FrameworkProperties,
			typename FrameworkProperties::GameProperties::PieceProperties::PieceAlias,
			raw_type<typename FrameworkProperties::IconLoader::IconAlias_type>>;

		requires TextureLoaderConcept<typename FrameworkProperties::TextureLoader>;

		requires IconLoaderConcept<
			typename FrameworkProperties::IconLoader,
			raw_type<typename FrameworkProperties::TextureLoader::TextureAlias_type>>;

		requires BoardLoaderConcept<
			typename FrameworkProperties::GameProperties::BoardProperties::BoardLoader, 
			raw_type<typename FrameworkProperties::GameProperties::BoardProperties::TileProperties >>;

		requires EdgeValidatorConcept<
			typename EdgeValidator<typename FrameworkProperties::GameProperties::BoardProperties>::EdgeValidator_type,
			FrameworkProperties>;

		requires PathfinderConcept< 
			raw_type<typename Pathfinder<typename FrameworkProperties::GameProperties>::Pathfinder_type>,
			FrameworkProperties>;

	{ FrameworkProperties::GameProperties::BoardProperties::WIDTH } -> std::same_as<const float&>;
	{ FrameworkProperties::GameProperties::BoardProperties::HEIGHT } -> std::same_as<const float&>;
			
	{ board.boardIconName() } -> std::same_as<raw_type<typename FrameworkProperties::IconLoader::IconAlias_type>>;
		
};


/// <summary>
/// A helper struct which defines all required types for the framework to function.
/// Makes it easier for other classes to access these types.
/// </summary>
/// <typeparam name="FrameworkProperties"></typeparam>
template<typename FrameworkProperties>
struct FrameworkTypes {

	using EventSystem = raw_type<typename FrameworkProperties::EventSystem>;

	using TexturesLoader = raw_type<typename FrameworkProperties::TextureLoader>;
	using TextureAlias = typename TexturesLoader::TextureAlias_type;	// we care about const vs non const
	using TextureAliasRaw = raw_type<TextureAlias>;

	using GameProperties = raw_type<typename FrameworkProperties::GameProperties>;

	using IconsLoader = raw_type<typename FrameworkProperties::IconLoader>;
	using IconAlias = typename IconsLoader::IconAlias_type;			// we care about const vs non const
	using IconAliasRaw = raw_type<IconAlias>;


	using BoardProperties = raw_type<typename GameProperties::BoardProperties>;
	using TileProperties = raw_type<typename BoardProperties::TileProperties>;

	using PlayerProperties = raw_type<typename GameProperties::PlayerProperties>;

	using PieceProperties = raw_type<typename GameProperties::PieceProperties>;
	using PieceAlias = raw_type<typename PieceProperties::PieceAlias>;

	using Pathfinder = raw_type<typename Pathfinder<GameProperties>::Pathfinder_type>;
	using EdgeValidator = raw_type<typename EdgeValidator<BoardProperties>::EdgeValidator_type>;

};