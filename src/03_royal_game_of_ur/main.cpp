#include <tabletop_framework.h>
#include <game_loading.h>

#include <iostream>
#include <string>
#include <random>


struct GameOfUrPlayerProperties {
public:
	int gamesWon;
};

struct PieceProperties {
public:
	using PieceAlias = int;
};

struct TileProperties {
	
	bool rosette;
	glm::ivec2 nextTile;

	static constexpr float TileSize = 142;
};

// ---------------------------------------------------------------

struct GameOfUrEdgeValidator; 

struct GameOfUrIconLoader {

	GameOfUrIconLoader() {}


	std::vector<IconEntry<std::string, std::string>> iconEntries
		= { {"board", "main", glm::ivec2(0, 0), glm::ivec2(2652, 1052) },
			{"piece_white", "main", glm::ivec2(1371, 1051), glm::ivec2(250, 250) },
			{"piece_black", "main", glm::ivec2(1661, 1051), glm::ivec2(250, 250) } };
	
	using IconAlias_type = std::string;

	auto iconsBegin() {
		return iconEntries.begin();
	}

	auto iconsEnd() {
		return iconEntries.end();
	}
};

struct GameOfUrTextureLoader {
	
	GameOfUrTextureLoader() {}

	std::vector<TextureEntry<std::string>> textureEntries = { {"main", "textures/game_of_ur.png"}};
	using TextureAlias_type = std::string;

	auto texturesBegin() {
		return textureEntries.begin();
	}
		
	auto texturesEnd() {
		return textureEntries.end();
	}
};

struct GameOfUrBoardLoader {

	GameOfUrBoardLoader() {}

	std::vector<TileEntry<TileProperties>> tileEntries = {
		{glm::ivec2(0, 0), glm::vec2(207, 207), {true,   {0, 1}}},
		{glm::ivec2(1, 0), glm::vec2(527, 207), {false,  {0, 0}}},
		{glm::ivec2(2, 0), glm::vec2(847, 207), {false,  {1, 0}}},
		{glm::ivec2(3, 0), glm::vec2(1167, 207), {false, {2, 0}}},
		{glm::ivec2(6, 0), glm::vec2(2127, 207), {true,  {5, 0}}},
		{glm::ivec2(7, 0), glm::vec2(2447, 207), {false, {6, 0}}},

		{glm::ivec2(0, 1), glm::vec2(207, 527), {false,  {1, 1}}},
		{glm::ivec2(1, 1), glm::vec2(527, 527), {false,  {2, 1}}},
		{glm::ivec2(2, 1), glm::vec2(847, 527), {false,  {3, 1}}},
		{glm::ivec2(3, 1), glm::vec2(1167, 527), {true,  {4, 1}}},
		{glm::ivec2(4, 1), glm::vec2(1487, 527), {false, {5, 1}}},
		{glm::ivec2(5, 1), glm::vec2(1807, 527), {false, {6, 1}}},
		{glm::ivec2(6, 1), glm::vec2(2127, 527), {false, {7, 1}}},
		{glm::ivec2(7, 1), glm::vec2(2447, 527), {false, {-1, -1}}},	// this one has two paths from it !

		{glm::ivec2(0, 2), glm::vec2(207, 847), {true,   {0, 1}}},
		{glm::ivec2(1, 2), glm::vec2(527, 847), {false,  {0, 2}}},
		{glm::ivec2(2, 2), glm::vec2(847, 847), {false,  {1, 2}}},
		{glm::ivec2(3, 2), glm::vec2(1167, 847), {false, {2, 2}}},
		{glm::ivec2(6, 2), glm::vec2(2127, 847), {true,  {5, 2}}},
		{glm::ivec2(7, 2), glm::vec2(2447, 847), {false, {6, 2}}},

		{glm::ivec2(5, 0), glm::vec2(1807, 207), {false, {-2, -2}}},	// finish
		{glm::ivec2(5, 2), glm::vec2(1807, 847), {false, {-2, -2}}}		// finish
	};

	std::vector<EdgeEntry> edgeEntries = {
		{{4, 0}, {3, 0}, 1 },
		{{3, 0}, {2, 0}, 1 },
		{{2, 0}, {1, 0}, 1 },
		{{1, 0}, {0, 0}, 1 },
		{{0, 0}, {0, 1}, 1 },

		{{4, 2}, {3, 2}, 1 },
		{{3, 2}, {2, 2}, 1 },
		{{2, 2}, {1, 2}, 1 },
		{{1, 2}, {0, 2}, 1 },
		{{0, 2}, {0, 1}, 1 },

		{{0, 1}, {1, 1}, 1 },
		{{1, 1}, {2, 1}, 1 },
		{{2, 1}, {3, 1}, 1 },
		{{3, 1}, {4, 1}, 1 },
		{{4, 1}, {5, 1}, 1 },
		{{5, 1}, {6, 1}, 1 },
		{{6, 1}, {7, 1}, 1 },
		
		{{7, 1}, {7, 0}, 1 },	// only for one player -> see validator
		{{7, 1}, {7, 2}, 1 },	// only for one player -> see validator

		{{7, 0}, {6, 0}, 1 },
		{{6, 0}, {5, 0}, 1 },

		{{7, 2}, {6, 2}, 1 },
		{{6, 2}, {5, 2}, 1 },
	};

	auto tilesBegin() {
		return tileEntries.begin();
	}

	auto tilesEnd() {
		return tileEntries.end();
	}

	// ----------------------------------------------

	auto edgesBegin() {
		return edgeEntries.begin();
	}

	auto edgesEnd() {
		return edgeEntries.end();
	}
};

// ------------------------------------------------------


struct BoardProperties {
public:
	static constexpr float WIDTH = 2652.0f;
	static constexpr float HEIGHT = 1052.0f;

	std::string boardIconName() {
		return "board";
	}

	using TileProperties = TileProperties;
	using BoardLoader = GameOfUrBoardLoader;
	using EdgeValidator = GameOfUrEdgeValidator;
};

struct GameOfUrPathfinder;

struct GameProperties {
public:
	using BoardProperties = BoardProperties;
	using PieceProperties = PieceProperties;
	
	using PlayerProperties = GameOfUrPlayerProperties;

	using Pathfinder = GameOfUrPathfinder;
};


// ------------------------------------------------------------------------------------

class GameOfUrGameEvents;

// ------------------------------------------------------------------------------------

struct FrameworkProperties {
public:
	
	using TextureLoader = GameOfUrTextureLoader;
	using IconLoader = GameOfUrIconLoader;

	using GameProperties = GameProperties;
	using EventSystem = GameOfUrGameEvents;
};

struct GameOfUrEdgeValidator {
private:
	GameContext<FrameworkProperties> gameContext;
public:

	GameOfUrEdgeValidator(GameContext<FrameworkProperties> context) :
		gameContext(context)
	{

	}

	bool isEdgeValid(const Edge& edge) const {
		auto playerId = gameContext.getCurrentPlayer();
		if (edge.from.x == 7 && edge.from.y == 1) {
			return (playerId == 0 && edge.to.y == 0)	// player 0 can move down
				|| (playerId == 1 && edge.to.y == 2);	// player 1 can move up
		}
		else {
			return true;
		}
	}
};


struct GameOfUrPathfinder {

	GameOfUrPathfinder() {}

	PiecePath getPathBetween(const glm::ivec2& from, const glm::ivec2& to, PathfinderContext<FrameworkProperties> context) {

		auto cost = context.getCost();
		if (cost > 0) {

			std::vector<glm::ivec2> candidates;
			glm::ivec2 last = from;

			// step forward cost times and see if that tile's pos == to
			int steps = 0;
			for (int i = 0; i < cost; ++i) {
				auto it = context.edgesFromBegin(last);
				while (it != context.edgesFromEnd(last)) {
					if (context.isValidEdgeBetween(last, it->to)) {
						candidates.push_back(it->to);
						last = it->to;
						steps++;
						break;
					}
					++it;
				}
			}

			if (last == to && cost == steps) {
				return PiecePath(std::move(candidates));
			}

		}

		return PiecePath();
	}

};



class GameOfUrGameEvents {
private:

	GameContext<FrameworkProperties> gameContext;


	std::vector<int> white_player;	// player id = 0
	std::vector<int> black_player;	// player id = 1

	std::mt19937 gen;
	std::uniform_int_distribution<int> distr;

	bool canRoll = true;

	int white = 0;
	int black = 0;

public:

	GameOfUrGameEvents(GameContext<FrameworkProperties> context) :
		gameContext(context),
		white_player({ 1, 2, 3, 4, 5, 6, 7 }),
		black_player({ 1, 2, 3, 4, 5, 6, 7 }),
		gen(std::random_device{}()),
		distr(0, 1)
	{

	}

	void init() {

		glm::vec2 offset(0.5, 0);
		glm::vec2 white_pos(-6, -3.2);

		for (auto&& white : white_player) {
			gameContext.createPiece(0, white, PieceProperties(), white_pos, glm::vec2(1, 1), "piece_white");
			white_pos += offset;
		}

		glm::vec2 black_pos(-6, 3.2);
		for (auto&& black : black_player) {
			gameContext.createPiece(1, black, PieceProperties(), black_pos, glm::vec2(1, 1), "piece_black");
			black_pos += offset;
		}

	}

	std::size_t gameBegin() {

		white = 0;
		black = 0;

		white_player.clear();
		black_player.clear();

		for (int i = 1; i <= 7; ++i)
		{
			white_player.push_back(i);
			black_player.push_back(i);
		}

		// random player at the start
		return distr(gen);
	}


	bool isGameOver() const {
		return white == 7 || black == 7;
	}

	void gameOver() {
		gameContext.getPlayerProperties(white == 7 ? 0 : 1).gamesWon++;

		std::cout << "Overall player scores: player 0: " << gameContext.getPlayerProperties(0).gamesWon
			<< " player 1: " << gameContext.getPlayerProperties(1).gamesWon << std::endl;

		white = 0;
		black = 0;
	}


	void mousePressedAt(float x, float y) {
		
	}

	void mousePressedAtTile(int x, int y) {}

	bool shouldProcessTilePress(int x, int y) {

		if (!gameContext.isPieceSelected() && gameContext.getCurrentCost() != 0 && (4 - x) == (int)gameContext.getCurrentCost()) {

			if (y == 0 && white_player.size() > 0 && gameContext.getCurrentPlayer() == 0) {
				// check if that tile is free
				if (!gameContext.playerHasPieceOnTile(0, { x, y })) {

					// move first available piece there
					gameContext.movePieceToTile(0, white_player[white_player.size() - 1], x, y);
					white_player.pop_back();

					// that was the move cost from that
					gameContext.setCost(0);

					return false;
				}
			}
			else
				if (y == 2 && black_player.size() > 0 && gameContext.getCurrentPlayer() == 1) {
					// check if that tile is free
					if (!gameContext.playerHasPieceOnTile(1, { x, y })) {

						// move first available piece there
						gameContext.movePieceToTile(1, black_player[black_player.size() - 1], x, y);
						black_player.pop_back();

						// that was the move cost from that
						gameContext.setCost(0);

						return false;
					}
				}

		}


		return true;
	}


	// -------------------------------------------------


	bool passTile(const int pieceId, const glm::ivec2& tilePos) {

		return true;
	}

	bool movePieceToTile(const int pieceId, const glm::ivec2& tilePos) {

		// cannot move onto own piece
		if (gameContext.playerHasPieceOnTile(gameContext.getCurrentPlayer(), tilePos))
			return false;

		// enemy piece that is on a rosette
		if (gameContext.playerHasPieceOnTile(1 - gameContext.getCurrentPlayer(), tilePos) && gameContext.getTileProperties(tilePos).rosette)
			return false;

		return true;
	}

	// ------------------------------------------------------------------------------------


	void turnBegin() {
		canRoll = true;
	}

	void turnEnd() {}

	bool canGenerateCost() const {
		return canRoll;
	}

	bool canSelectPiece() {
		return true;
	}

	int generateCost() {

		canRoll = false;

		// flip 4 coins
		return distr(gen) + distr(gen) + distr(gen) + distr(gen);
	}

	bool canEndTurn() {

		if (canRoll)
			return false;


		// cannot roll
		auto cost = gameContext.getCurrentCost();
		if (cost == 0)
			return true;


		// current cost > 0 -> check if there's any piece that can be moved

		// check if a new piece can be spawned
		if (gameContext.getCurrentPlayer() == 0) {
			if (white_player.size() > 0) {
				// doesn't have a piece in the way
				if (!gameContext.playerHasPieceOnTile(0, { 4 - cost, 0 }))
					return false;
			}
		}
		else {
			if (black_player.size() > 0) {
				if (black_player.size() > 0) {
					// doesn't have a piece in the way
					if (!gameContext.playerHasPieceOnTile(1, { 4 - cost, 2 }))
						return false;
				}
			}
		}


		// check all pieces that are on the board for if they can be moved
		auto currentPlayer = gameContext.getCurrentPlayer();

		auto it = gameContext.piecesBegin(currentPlayer);
		while (it != gameContext.piecesEnd(currentPlayer)) {

			// piece is on the playing board
			if (gameContext.isPieceAtTile(currentPlayer, *it)) {

				auto tilePos = gameContext.getTilePosition(currentPlayer, *it);
				// step forward using the tile properties -> next tile
				for (int i = 0; i < cost; ++i) {

					// the only tile that has two possible movement directions based on the actual current player
					if (tilePos.x == -1) {
						if (currentPlayer == 0) {
							tilePos = glm::ivec2(7, 0);
						}
						else {
							tilePos = glm::ivec2(7, 2);
						}
					}
					else if (tilePos.x == -2) {
						break;
					}
					else
						tilePos = gameContext.getTileProperties(tilePos).nextTile;

				}

				// if this piece cannot reach the end by spending all cost -> might return true
				// if that tile has an enemy piece standing on a rosette -> might return true
				// check if the last tilePos doesn't have a friendly piece 
				if (
					tilePos.x != -2 &&
					!(gameContext.playerHasPieceOnTile(1 - currentPlayer, tilePos) && gameContext.getTileProperties(tilePos).rosette) &&

					!gameContext.playerHasPieceOnTile(currentPlayer, tilePos))
					return false;
			}

			++it;
		}


		// no pieces can be moved (cost cannot be spent)
		return true;
	}


	void pieceCreatedAt(std::size_t playerId, const int pieceAlias, const glm::vec2& position) {}
	void pieceCreatedAt(std::size_t playerId, const int pieceAlias, const glm::ivec2& tilePosition) {}
	void pieceMovedTo(std::size_t playerId, const int pieceAlias, const glm::vec2& position) {}

	void pieceMovedTo(std::size_t playerId, const int pieceAlias, const glm::ivec2& tilePosition) {

		// player piece of the board at the end
		if (tilePosition.x == 5) {
			if (tilePosition.y == 0) {
				white++;
				gameContext.movePiece(playerId, pieceAlias, { 2 + white * 0.5,  -3.2 });
				gameContext.deselectPiece();
			}
			else if (tilePosition.y == 2) {
				black++;
				gameContext.movePiece(playerId, pieceAlias, { 2 + black * 0.5,  3.2 });
				gameContext.deselectPiece();
			}
		}

		auto otherPlayer = 1 - gameContext.getCurrentPlayer();
		if (gameContext.playerHasPieceOnTile(otherPlayer, tilePosition)) {
			auto&& otherPiece = gameContext.getPieceAt(otherPlayer, tilePosition);

			// move the enemy player piece off
			// add to vector again
			if (gameContext.getCurrentPlayer() == 0) {

				gameContext.movePiece(otherPlayer, otherPiece, { -6 + black_player.size() * 0.5, 3.2 });
				black_player.push_back(otherPiece);
			}
			else {

				gameContext.movePiece(otherPlayer, otherPiece, { -6 + white_player.size() * 0.5, -3.2 });
				white_player.push_back(otherPiece);
			}

			std::cout << "Captured " << otherPiece << " at " << tilePosition.x << " " << tilePosition.y << std::endl;
		}


		if (gameContext.getTileProperties(tilePosition).rosette) {
			canRoll = true;
			gameContext.deselectPiece();	// for ease
		}
	}

	void restart() {

		// reset player properties
		gameContext.getPlayerProperties(0).gamesWon = 0;
		gameContext.getPlayerProperties(1).gamesWon = 0;
	}

	void pieceVisibilityChanged(std::size_t playerId, const int pieceAlias, bool visible) {}
	void pieceIconChanged(std::size_t playerId, const int pieceAlias, const std::string& icon) {};
	void pieceSizeChanged(std::size_t playerId, const int pieceAlias, const glm::vec2 size) {};

};



int main() {
	
#if defined(DEBUG) || defined(_DEBUG)
	TabletopFramework<FrameworkProperties> my_game(800, 600, 
		{
			FrameworkSettings(
				"Royal game of Ur",
				"./../../../../03_royal_game_of_ur",
				2, 
				2)
		}
	);
#else
	TabletopFramework<FrameworkProperties> my_game(800, 600,
		{
			FrameworkSettings(
				"Royal game of Ur",
				"./..",
				2,
				2)
		}
	);

#endif	// DEBUG

	if(my_game.isValid())
		my_game.start();

	return 0;
}