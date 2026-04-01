#pragma once

#include <glm/glm.hpp>

#include <filesystem>
#include <iostream>
#include <unordered_map>

#include "configuration_file.h"
#include "game_loading_utils.h"
#include "framework_contexts.h"
#include "playing_board_elements.h"
#include "framework_exceptions.h"

/// <summary>
/// Storage for tiles. Tiles are loaded once and then never change. Hold the graph of the game plan.
/// Knows where edges between tiles are and their weights. Will be able to utilize pathfinding algorithms on this graph.
/// </summary>
/// <typeparam name="BoardProperties"></typeparam>
template <typename BoardProperties>
class PlayingBoard {
	
	using BoardLoader = raw_type<typename BoardProperties::BoardLoader>;
	using TileProperties = raw_type<typename BoardProperties::TileProperties>;
	using EdgeValidator = raw_type<typename EdgeValidator<BoardProperties>::EdgeValidator_type>;
private:

	EdgeValidator edgeValidator;

	const GameObject me;

	// hold all known tiles
	std::unordered_map<std::size_t, Tile<TileProperties>> tiles;
	// edges between the tiles (key = from tile; Edge has all info)
	std::unordered_map<std::size_t, std::vector<Edge>> edges;

	std::vector<Edge>::const_iterator constEmptyIt;

	// the grid position is used as keys for the tiles
	std::size_t tilePositionToName(int x, int y) const {
		return (static_cast<size_t>(x) << 32) | static_cast<size_t>(y);
	}
	
	using TileEntryIterator = decltype(tiles.begin());
	using TileEntryConstIterator = decltype(tiles.cbegin());

	inline void loadTiles(BoardLoader& loader) {

		DEBUG_ACTION(std::cout << "loading tiles " << std::endl);

		for (auto&& it = loader.tilesBegin(); it != loader.tilesEnd(); ++it) {
			auto&& tileEntry = *it;
			
			auto key = tilePositionToName(tileEntry.pos.x , tileEntry.pos.y);

			DEBUG_ACTION(std::cout << "tile key: " << key);

			if (tiles.find(key) != tiles.end())
				throw InvalidArgument<glm::ivec2>::elementDuplicateFound(tileEntry.pos);

			tiles.emplace(key, 
				Tile<TileProperties>(
					glm::vec2((tileEntry.offset.x / UNIT_SCALING - width)/2, (tileEntry.offset.y / UNIT_SCALING - height)/2),
					tileEntry.dim / UNIT_SCALING,
					tileEntry.pos,
					std::move(tileEntry.properties)));

			DEBUG_ACTION(std::cout << " - loaded " << std::endl);
		}
	}

	inline void loadEdges(BoardLoader& loader) {

		DEBUG_ACTION(std::cout << "loading edges " << std::endl);

		for (auto&& it = loader.edgesBegin(); it != loader.edgesEnd(); ++it) {
			auto&& edgeEntry = *it;
			
			auto key = tilePositionToName(edgeEntry.from.x, edgeEntry.from.y);

			DEBUG_ACTION(std::cout << "edge from " << key << " to " << tilePositionToName(edgeEntry.to.x, edgeEntry.to.y));

			// create the vector (first edge from this tile)
			if (edges.find(key) == edges.end()) {
				edges.emplace(key, std::vector<Edge>());
			}
			
			edges.at(key).emplace_back(edgeEntry.cost, edgeEntry.from, edgeEntry.to);
			
			DEBUG_ACTION(std::cout << " - loaded " << std::endl);

		}
	}
	

	// to iterator over tile properties -> for public use
	struct TilePropertyIterator {
	private:
		TileEntryIterator it;
	public:
		using iterator_category = std::forward_iterator_tag;
		using value_type = TileProperties;
		using difference_type = std::ptrdiff_t;
		using pointer = value_type*;
		using reference = value_type&;

		TilePropertyIterator(TileEntryIterator it) 
		: it(it) {

		}

		reference operator*() {
			return it->second.getProperties();
		}

		pointer operator->() {
			return &it->second.getProperties();
		}

		TilePropertyIterator& operator++() {
			++it;
			return *this;
		}

		TilePropertyIterator operator++(int) {
			TilePropertyIterator tmp = *this;
			++(*this);
			return tmp;
		}

		bool operator==(const TilePropertyIterator& other) const {
			return it == other.it;
		}

		bool operator!=(const TilePropertyIterator& other) const {
			return !(*this == other);
		}

	};

	struct ConstTilePropertyIterator {
	private:
		TileEntryConstIterator it;
	public:
		using iterator_category = std::forward_iterator_tag;
		using value_type = TileProperties;
		using difference_type = std::ptrdiff_t;
		using pointer = const value_type*;
		using reference = const value_type&;

		ConstTilePropertyIterator(TileEntryConstIterator it)
			: it(it) {

		}

		reference operator*() {
			return it->second.getProperties();
		}

		pointer operator->() {
			return &it->second.getProperties();
		}

		ConstTilePropertyIterator& operator++() {
			++it;
			return *this;
		}

		ConstTilePropertyIterator operator++(int) {
			ConstTilePropertyIterator tmp = *this;
			++(*this);
			return tmp;
		}

		bool operator==(const TilePropertyIterator& other) const {
			return it == other.it;
		}

		bool operator!=(const TilePropertyIterator& other) const {
			return !(*this == other);
		}

	};


	// to iterate over tile values (not keys and values) -> for inside framework use
	struct TileIterator {
	private:
		TileEntryConstIterator it;
	public:
		using iterator_category = std::forward_iterator_tag;
		using value_type = Tile<TileProperties>;
		using difference_type = std::ptrdiff_t;
		using pointer = const value_type*;
		using reference = const value_type&;

		TileIterator(TileEntryConstIterator it)
			: it(it) {

		}

		reference operator*() const {
			return it->second;
		}

		pointer operator->() const {
			return &(it->second);
		}

		TileIterator& operator++() {
			++it;
			return *this;
		}

		TileIterator operator++(int) {
			TileIterator tmp = *this;
			++(*this);
			return tmp;
		}

		bool operator==(const TileIterator& other) const {
			return it == other.it;
		}

		bool operator!=(const TileIterator& other) const {
			return !(*this == other);
		}


	};

	Tile<TileProperties>& getTileM(int x, int y) {
		auto it = tiles.find(tilePositionToName(x, y));
		if (it != tiles.end())
			return it->second;
		else
			throw InvalidArgument<glm::ivec2>::tileNotFound(x, y);
	}

	const Tile<TileProperties>& getTileM(int x, int y) const {
		auto it = tiles.find(tilePositionToName(x, y));
		if (it != tiles.end())
			return it->second;
		else
			throw InvalidArgument<glm::ivec2>::tileNotFound(x, y);
	}


public:

	static constexpr float width = BoardProperties::WIDTH / 2 / UNIT_SCALING;
	static constexpr float height = BoardProperties::HEIGHT / 2 / UNIT_SCALING;

	using IteratorType = TileIterator;

	template<typename FrameworkProperties>
	PlayingBoard(
		GameGraphicsFacade<FrameworkProperties>& facade,
		EdgeValidator&& edgeValidator) :
		edgeValidator(std::move(edgeValidator)),
		me(facade.
		addObject(
			glm::vec3(), 
			glm::vec2(width, height), 
			BoardProperties().boardIconName())	
	)
	{
		BoardLoader bl;
		loadTiles(bl);
		loadEdges(bl);
	}

	auto tilesBegin() const {
		return TileIterator(tiles.cbegin());	
	}

	auto tilesEnd() const {
		return TileIterator(tiles.cend());
	}

	auto tilePropertiesBegin() {
		return TilePropertyIterator(tiles.begin());
	}

	auto tilePropertiesEnd() {
		return TilePropertyIterator(tiles.end());
	}

	auto tilePropertiesBegin() const {
		return ConstTilePropertyIterator(tiles.begin());
	}

	auto tilePropertiesEnd() const {
		return ConstTilePropertyIterator(tiles.end());
	}

	auto edgesFromBegin(const glm::ivec2 from) const {
		auto it = edges.find(tilePositionToName(from.x, from.y));
		if (it != edges.end())
			return it->second.cbegin();
		else
			return constEmptyIt;
	}

	auto edgesFromEnd(const glm::ivec2 from) const {
		
		auto it = edges.find(tilePositionToName(from.x, from.y));
		if (it != edges.end())
			return it->second.cend();
		else
			return constEmptyIt;
	}

	EdgeValidator& getEdgeValidator() {
		return edgeValidator;
	}

	const EdgeValidator& getEdgeValidator() const {
		return edgeValidator;
	}

	// ------------------------------------------------------------

	const Tile<TileProperties>& getTile(int x, int y) const {
		auto it = tiles.find(tilePositionToName(x, y));
		if (it != tiles.end())
			return it->second;
		else
			throw InvalidArgument<glm::ivec2>::tileNotFound(x, y);
	}

	const Tile<TileProperties>& getTile(const glm::ivec2& tilePos) const {
		return getTile(tilePos.x, tilePos.y);
	}

	TileProperties& getTileProperties(int x, int y) {
		return getTileM(x, y).getProperties();
	}

	TileProperties& getTileProperties(const glm::ivec2& tilePos) {
		return getTileProperties(tilePos.x, tilePos.y);
	}

	const TileProperties& getTileProperties(int x, int y) const {
		return getTileM(x, y).getProperties();
	}

	const TileProperties& getTileProperties(const glm::ivec2& tilePos) const {
		return getTileProperties(tilePos.x, tilePos.y);
	}

	
	bool isEdgeBetween(const glm::ivec2& from, const glm::ivec2& to) const {
		auto key = tilePositionToName(from.x, from.y);
		
		auto edgesIt = edges.find(key);
		if (edgesIt == edges.end())
			return false;

		auto&& fromEdges = (edgesIt)->second;

		auto edge = std::find_if(fromEdges.begin(), fromEdges.end(),
			[&](const Edge& e) {
				return e.to == to;
			}
		);
		if (edge == fromEdges.end()) {
			return false;
		}
		else {
			return edgeValidator.isEdgeValid(*edge);
		}

	}

	const Edge& getEdge(const glm::ivec2& from, const glm::ivec2& to) const
	{
		auto key = tilePositionToName(from.x, from.y);

		auto edgesIt = edges.find(key);
		if (edgesIt == edges.end())
			throw InvalidArgument<glm::ivec2>::tileNotFound(from);

		auto&& fromEdges = (edgesIt)->second;

		auto edge = std::find_if(fromEdges.begin(), fromEdges.end(),
			[&](const Edge& e) {
				return e.to == to;
			}
		);

		if (edge != fromEdges.end()) {
			return *edge;
		}
		else {
			throw InvalidArgument<glm::ivec2>::tileNotFound(to);
		}
	}
	
};