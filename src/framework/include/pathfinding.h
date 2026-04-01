#pragma once

#include <glm/glm.hpp>
#include <vector>

// forward declaration
template<typename FrameworkProperties>
class PathfinderContext;

// path representation 
class PiecePath {
private:
	std::vector<glm::ivec2> path;

public:
	PiecePath() {}

	PiecePath(std::vector<glm::ivec2>&& path) :
	path(std::move(path))
	{}

	inline size_t getLength() {
		return path.size();
	}

	inline auto begin() {
		return path.begin();
	}

	inline auto end() {
		return path.end();
	}

	inline auto last() {
		return end() - 1;	// only valid for len != 0
	}
};

// NOTE: in the future maybe implement bfs 

// only looks if there's an edge between the two specified tiles
class DefaultPathFinder {
public:

	template<typename FrameworkProperties>
	inline PiecePath getPathBetween(const glm::ivec2& from, const glm::ivec2& to, PathfinderContext<FrameworkProperties> context) {
		
		if (context.getCost() > 0 && context.isValidEdgeBetween(from, to)) {
			if (context.getEdgeCost(from, to) <= context.getCost()) {
				return PiecePath({ to });
			}
		}

		return PiecePath();
	}


};