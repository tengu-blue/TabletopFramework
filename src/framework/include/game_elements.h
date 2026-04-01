#pragma once

#include <unordered_map>

#include <limits>
#include "game_object.h"
#include "playing_board_elements.h"
#include "framework_exceptions.h"

template<typename FrameworkProperties>
class Piece {

	using Types = FrameworkTypes<FrameworkProperties>;
	using PieceProperties = Types::PieceProperties;
	using PieceAlias = Types::PieceAlias;
	using TileProperties = Types::TileProperties;
	using IconAliasRaw = Types::IconAliasRaw;

private:

	GameGraphicsFacade<FrameworkProperties>& myFacade;

	GameObject me;
	PieceProperties properties;

	// the tile where this piece currently is
	// raw pointer is safe because tiles don't ever move
	const Tile<TileProperties>* where = nullptr;

	PieceAlias id;

	bool visible = true;
	glm::vec2 position, originalPosition, size, originalSize;

	float z() {
		return visible ? 0.0f : 100.0f;
	}

public:

	Piece(
		GameGraphicsFacade<FrameworkProperties>& gameGraphicsFacade,
		PieceProperties&& properties,
		const PieceAlias& id,
		
		glm::vec2 position,
		glm::vec2 size,

		const IconAliasRaw& iconName
		) : 
		
		myFacade(gameGraphicsFacade),
		me(myFacade.addObject({ position.x, position.y, z() }, size, iconName)),
		properties(std::move(properties)), 
		id(id),
		position(position),
		originalPosition(position),
		size(size),
		originalSize(size)
	{
		
	}

	// Creates the new piece at the given tile.
	Piece(
		GameGraphicsFacade<FrameworkProperties>& gameGraphicsFacade,
		PieceProperties&& properties,
		const PieceAlias& id,

		const Tile<TileProperties>& tile,
		glm::vec2 size,

		const IconAliasRaw& iconName
	) :

		myFacade(gameGraphicsFacade),
		me(myFacade->addObject({ tile.position.x, tile.position.y, z() }, size, iconName)),
		properties(std::move(properties)),
		id(id),
		position(tile.position),
		originalPosition(position),
		size(size),
		originalSize(size)
	{
		where = &tile;
	}

	inline PieceProperties& getProperties() {
		return properties;
	}

	inline const PieceProperties& getProperties() const {
		return properties;
	}

	inline const PieceAlias& getId() const {
		return id;
	}

	const glm::vec2 getPosition() const {
		return position;
	}

	const glm::vec2 getSize() const {
		return size;
	}

	// ---------------------------------------------------------------------------

	bool isAt(int x, int y) const {
		if (where) {
			auto pos = where->boardCoordinates;
			return pos.x == x && pos.y == y;
		}
		else {
			return false;
		}
	}

	bool isAtTile() {
		return (where != nullptr);
	}

	glm::ivec2 getTilePosition() const {
		if (where) {
			return where->boardCoordinates;
		}
		else {
			return { INT_MAX, INT_MAX };		// nonsense
		}
	}

	// ------------------------------------------------------------------------

	void moveTo(const glm::vec2& newPosition) {
		where = nullptr;
		position = newPosition;
		myFacade.moveObject(me, { position.x, position.y, z() });
	}

	void moveTo(const Tile<TileProperties>* newTile) {
		where = newTile;
		position = where->position;
		myFacade.moveObject(me, { position.x, position.y, z() });
	}

	void removeFromTile() {
		where = nullptr;
	}

	void setVisible(bool visible) {
		this->visible = visible;

		// if turns invisible -> remove from tile
		if (!visible)
			where = nullptr;

		myFacade.moveObject(me, { position.x, position.y, z() });
	}

	void setIcon(const IconAliasRaw& icon) {
		myFacade.changeIcon(me, icon);
	}

	void setSize(const glm::vec2& scale) {
		myFacade.changeScale(me, scale);
	}

	// ---------------------------------------------------

	void reset() {
		moveTo(originalPosition);
		setSize(originalSize);
		setVisible(true);
	}
};

// --------------------------------------------------------------------------------

template<typename FrameworkProperties>
/// <summary>
/// Each player has their own pieces.
/// </summary>
class Player {

	using Types = FrameworkTypes<FrameworkProperties>;
	using PieceAlias = typename Types::PieceAlias;
	using PieceProperties = typename Types::PieceProperties;
	using IconAliasRaw = typename Types::IconAliasRaw;
	using TileProperties = typename Types::TileProperties;
	using PlayerProperties = typename Types::PlayerProperties;

private:

	PlayerProperties properties;
	GameGraphicsFacade<FrameworkProperties>& myFacade;

	std::unordered_map<PieceAlias, Piece<FrameworkProperties>> gamePieces;
	
	using PieceIt = decltype(gamePieces.begin());
	using ConstPieceIt = decltype(gamePieces.cbegin());

	// utility carrier types to avoid using raw pointers -> instead throw exceptions when trying to access invalid terms
	struct PieceCarrier {
	private:
		Piece<FrameworkProperties>* piece;
		glm::ivec2 tilePos;
	public:
		PieceCarrier(Piece<FrameworkProperties>* piece, glm::ivec2&& tilePos) :
			piece(piece),
			tilePos(std::move(tilePos))
		{

		}

		bool isValid() {
			return piece != nullptr;
		}

		Piece<FrameworkProperties>& operator*() {
			if (piece)
				return *piece;
			else
				throw InvalidArgument<glm::ivec2>::pieceNotFoundAtTile(tilePos);
		}

		Piece<FrameworkProperties>* operator->() {
			return piece;
		}

		const Piece<FrameworkProperties>& operator*() const {
			if (piece)
				return *piece;
			else
				throw InvalidArgument<glm::ivec2>::pieceNotFoundAtTile(tilePos);
		}

		const Piece<FrameworkProperties>* operator->() const {
			return piece;
		}

	};
	struct ConstPieceCarrier {
	private:
		const Piece<FrameworkProperties>* piece;
		glm::ivec2 tilePos;
	public:
		ConstPieceCarrier(const Piece<FrameworkProperties>* piece, glm::ivec2&& tilePos) :
			piece(piece),
			tilePos(std::move(tilePos))
		{

		}

		bool isValid() {
			return piece != nullptr;
		}

		const Piece<FrameworkProperties>& operator*() {
			if (piece)
				return *piece;
			else
				throw InvalidArgument<glm::ivec2>::pieceNotFoundAtTile(tilePos);
		}

		const Piece<FrameworkProperties>* operator->() {
			return piece;
		}

	};

	struct PieceAliasIterator {
	private:
		ConstPieceIt it;
	public:
		using iterator_category = std::forward_iterator_tag;
		using value_type = PieceAlias;
		using difference_type = std::ptrdiff_t;
		using pointer = const value_type*;
		using reference = const value_type&;

		PieceAliasIterator(ConstPieceIt it)
			: it(it)
		{}

		reference operator*() const {
			return it->second.getId();
		}

		pointer operator->() const {
			return &(it->second.getId());
		}

		PieceAliasIterator& operator++() {
			++it;
			return *this;
		}

		PieceAliasIterator operator++(int) {
			PieceAliasIterator tmp = *this;
			++(*this);
			return tmp;
		}

		bool operator==(const PieceAliasIterator& other) const {
			return it == other.it;
		}

		bool operator!=(const PieceAliasIterator& other) const {
			return !(*this == other);
		}
	};

	struct PiecePropertiesIterator {
	private:
		PieceIt it;
	public:
		using iterator_category = std::forward_iterator_tag;
		using value_type = PieceProperties;
		using difference_type = std::ptrdiff_t;
		using pointer = value_type*;
		using reference = value_type&;

		PiecePropertiesIterator(PieceIt it)
			: it(it)
		{}

		reference operator*() {
			return it->second.getProperties();
		}

		pointer operator->() {
			return &(it->second.getProperties());
		}

		PiecePropertiesIterator& operator++() {
			++it;
			return *this;
		}

		PiecePropertiesIterator operator++(int) {
			PiecePropertiesIterator tmp = *this;
			++(*this);
			return tmp;
		}

		bool operator==(const PiecePropertiesIterator& other) const {
			return it == other.it;
		}

		bool operator!=(const PiecePropertiesIterator& other) const {
			return !(*this == other);
		}
	};

	struct ConstPiecePropertiesIterator {
	private:
		ConstPieceIt it;
	public:
		using iterator_category = std::forward_iterator_tag;
		using value_type = PieceProperties;
		using difference_type = std::ptrdiff_t;
		using pointer = const value_type*;
		using reference = const value_type&;

		ConstPiecePropertiesIterator(ConstPieceIt it)
			: it(it)
		{}

		reference operator*() const {
			return it->second.getProperties();
		}

		pointer operator->() const {
			return &(it->second.getProperties());
		}

		ConstPiecePropertiesIterator& operator++() {
			++it;
			return *this;
		}

		ConstPiecePropertiesIterator  operator++(int) {
			ConstPiecePropertiesIterator  tmp = *this;
			++(*this);
			return tmp;
		}

		bool operator==(const ConstPiecePropertiesIterator& other) const {
			return it == other.it;
		}

		bool operator!=(const ConstPiecePropertiesIterator& other) const {
			return !(*this == other);
		}
	};


public:

	Player(GameGraphicsFacade<FrameworkProperties>& gameGraphicsFacade) : 
		properties(PlayerProperties()),
		myFacade(gameGraphicsFacade) 
	{

	}

	const PlayerProperties& getProperties() const {
		return properties;
	}

	PlayerProperties& getProperties() {
		return properties;
	}

	auto begin() const {
		return PieceAliasIterator(gamePieces.begin());
	}

	auto end() const {
		return PieceAliasIterator(gamePieces.end());
	}

	auto piecePropertiesBegin() {
		return PiecePropertiesIterator(gamePieces.begin());
	}

	auto piecePropertiesEnd() {
		return PiecePropertiesIterator(gamePieces.end());
	}

	auto piecePropertiesBegin() const {
		return ConstPiecePropertiesIterator(gamePieces.cbegin());
	}

	auto piecePropertiesEnd() const {
		return ConstPiecePropertiesIterator(gamePieces.cend());
	}


	Piece<FrameworkProperties>& addPiece(
		const PieceAlias& id, 
		PieceProperties&& properties, 
		glm::vec2 position, 
		glm::vec2 size, 
		const IconAliasRaw& iconName) {
		
		if (gamePieces.find(id) == gamePieces.end()) {

			auto [it, success] = gamePieces.try_emplace(
				id,
				myFacade, std::move(properties), id, position, size, iconName
			);

			// return the newly created piece
			return gamePieces.at(id);
		}
		else {
			throw InvalidArgument<PieceAlias>::elementDuplicateFound(id);
		}

		
	}

	Piece<FrameworkProperties>& addPieceAt(
		const PieceAlias& id,
		PieceProperties&& properties,
		
		const Tile<TileProperties>& tile,
		glm::vec2 size,
		const IconAliasRaw& iconName) {

		if (gamePieces.find(id) == gamePieces.end()) {

			gamePieces.try_emplace(
				id,
				myFacade, std::move(properties), id, tile, size, iconName
			);

			// return the newly created piece
			return gamePieces.at(id);
		}
		else {
			throw InvalidArgument<PieceAlias>::elementDuplicateFound(id);
		}
	}

	

	auto& getPiece(const PieceAlias& id) {
		auto it = gamePieces.find(id);
		if (it != gamePieces.end())
			return it->second;
		else
			throw InvalidArgument<PieceAlias>::pieceNotFound(id);
	}

	const auto& getPiece(const PieceAlias& id) const {
		auto it = gamePieces.find(id);
		if (it != gamePieces.end())
			return it->second;
		else
			throw InvalidArgument<PieceAlias>::pieceNotFound(id);
	}

	PieceCarrier getPieceAt(int x, int y) {
		auto it = std::find_if(gamePieces.begin(), gamePieces.end(),
			[x, y](const auto& pair) {
				return pair.second.isAt(x, y);
			}
		);

		if (it != gamePieces.end()) {
			return PieceCarrier(&(it->second), glm::ivec2(x, y));
		}
		else {
			return PieceCarrier(nullptr, glm::ivec2(x, y));
		}
	}
	
	ConstPieceCarrier getPieceAt(int x, int y) const {
		auto it = std::find_if(gamePieces.begin(), gamePieces.end(),
			[x, y](const auto& pair) {
				return pair.second.isAt(x, y);
			}
		);

		if (it != gamePieces.end()) {
			return ConstPieceCarrier(&(it->second), glm::ivec2( x, y));
		}
		else {
			return ConstPieceCarrier(nullptr, glm::ivec2(x, y));
		}
	}

};