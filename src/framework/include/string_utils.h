#pragma once
#include <string>
#include <cctype>
#include <algorithm>

// adapted from : https://stackoverflow.com/questions/216823/how-to-trim-a-stdstring
inline void trim(std::string& s) {
	s.erase(s.begin(), std::find_if(s.begin(), s.end(),
			[](unsigned char ch) { return !std::isspace(ch); } // find first non whitespace char
		)
	);
		
	s.erase(std::find_if(s.rbegin(), s.rend(), 
			[](unsigned char ch) { return !std::isspace(ch); }	// find first non whitespace from the end
		).base(), s.end()
	);
}

inline bool containsWhitespace(const std::string& str) {
	return std::any_of(str.begin(), str.end(), 
		[](unsigned char ch) { return std::isspace(ch); }
	);
}