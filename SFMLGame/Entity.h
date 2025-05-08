#pragma once

#include "Components.h"
#include <string>
#include <tuple>

using namespace std;

class EntityManager;

using ComponentTuple = tuple<
	CAnimation,
	CBoundingBox,
	CGravity,
	CTransform, 
	CShape, 
	CCollision,
	CInput,
	CLifespan,
	CScore,
	CState,
	CChasePlayer,
	CPatrol,
	CHealth,
	CInvincibility,
	CDamage,
	CDraggable
>;


class Entity
{
public:
	bool isActive() const { return active; }
	void destroy() { active = false; }
	size_t id() const { return _id; }
	const string& tag() const { return _tag; }

	template<typename T>
	bool has() const
	{
		return get<T>().exists;
	}

	template<typename T, typename... TArgs>
	T& add(TArgs&&... mArgs)
	{
		T& component = get<T>();
		component = T(forward<TArgs>(mArgs)...);
		component.exists = true;
		return component;
	}

	template<typename T>
	T& get()
	{
		return std::get<T>(components);
	}

	template<typename T>
	const T& get() const
	{
		return std::get<T>(components);
	}

	template<typename T>
	void remove()
	{
		get<T>() = T();
	}

private:
	friend class EntityManager;

    Entity(const size_t id, const string& tag)
        : _id(id), _tag(tag) {}

	ComponentTuple components;
	bool active = true;
	string _tag = "default";
	size_t _id = 0;
};

