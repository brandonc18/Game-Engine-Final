#pragma once
#include <string>

#include "Vec2.h"

using namespace std;

class Action
{
public:

	Action() = default;

	Action(const string& name, const string& type)
		:name(name), type(type) { }

	Action(const string& name, const string& type, const Vec2i position)
		:name(name), type(type), position(position) {
	}
	
	const string& getName() const { return name; }
	const string& getType() const { return type; }
	const Vec2i& getPosition() const { return position;  }

private:
	string name = "NONE";
	string type = "NONE";
	Vec2i position;
};