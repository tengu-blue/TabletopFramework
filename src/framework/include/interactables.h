#pragma once

#include <vector>
#include <algorithm>
#include <optional>

#include <glm/glm.hpp>

template<typename RegionID>
struct ClickableRegion {
	glm::vec2 pos;
	glm::vec2 dim;

	RegionID id;
};


template<typename RegionID>
class ClickableManager {
private:
	std::vector<ClickableRegion<RegionID>> clickables;

public:

	template<typename ClickablesLoader>
	ClickableManager(ClickablesLoader loader) {
		
		DEBUG_ACTION(std::cout << "loading clickable regions for board tiles" << std::endl);
		// load regions
		for (auto it = loader.begin(); it != loader.end(); ++it)
		{
			DEBUG_ACTION(std::cout << "loading clickable region - ");	// maybe more info ?
			clickables.emplace_back(*it);
			DEBUG_ACTION(std::cout << " loaded: " << clickables[clickables.size()-1].pos.x << " " << clickables[clickables.size() - 1].pos.y << std::endl);
		}

		// sort them based on pos.x (might help - TODO: quad trees or something like that)
		std::sort(clickables.begin(), clickables.end(), 
			[](const ClickableRegion<RegionID>& a, const ClickableRegion<RegionID>& b) {
				return a.pos.x < b.pos.x;
			}
		);
	}

	const RegionID* isAt(float x, float y) {

		// NOTE: this place could use some speeding up -> maybe store in some tree 
		
		// go through all and return the first that matches
		auto it = std::find_if(clickables.begin(), clickables.end(),
			[x, y](const ClickableRegion<RegionID>& a) {
				// DEBUG_ACTION(std::cout << a.pos.x<< " "<< a.dim.x << " " << a.pos.y << " " << a.dim.y << std::endl);
				return (a.pos.x <= x && x <= a.pos.x + a.dim.x && a.pos.y <= y && y <= a.pos.y + a.dim.y);
			}
		);

		if (it != clickables.end()) {
			return &(it->id);
		}
		else {
			return nullptr;
		}
	}
};