#pragma once

#include <string>
#include <filesystem>
#include "game_loading.h"
#include "playing_board_elements.h"
#include "pathfinding.h"

template<typename FrameworkProperties>
class GameContext;


class DefaultEdgeValidator {
public:
	
	template<typename FrameworkProperties>
	DefaultEdgeValidator(GameContext<FrameworkProperties> context) {}
	
	inline bool isEdgeValid(const Edge& edge) const {
		return true;
	}
};

