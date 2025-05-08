#pragma once

#include "Entity.h"

using namespace std;

using EntityVec = vector<Entity*>;
using EntityMap = map<string, EntityVec>;

class EntityManager
{
public:
	EntityManager() = default;
	~EntityManager()
	{
		entityMap.clear();
		for (auto& e : entities)
		{
			delete e;
		}
		entities.clear();
	}

	void update()
	{
		for (auto e : entitiesToAdd)
		{
			entities.push_back(e);
			entityMap[e->tag()].push_back(e);
		}
		entitiesToAdd.clear();

		removeDeadEntities(entities);

		for (auto& pairing : entityMap)
		{
			removeDeadEntities(pairing.second);
		}
	}

	Entity* addEntity(const string& tag)
	{
		Entity* entity = new Entity(totalEntities++, tag);

		entitiesToAdd.push_back(entity);

		if (entityMap.find(tag) == entityMap.end()){ entityMap[tag] = EntityVec(); }

		return entity;
	}

	EntityVec& getEntities()
	{
		return entities;
	}

	EntityVec& getEntities(const string& tag)
	{
		return entityMap[tag];
	}
private:
	EntityVec entities;
	EntityVec entitiesToAdd;
	EntityMap entityMap;
	size_t totalEntities = 0;

	void removeDeadEntities(EntityVec& vec)
	{
		for (auto it = vec.begin(); it != vec.end();)
		{
			if (!(*it)->active)
				it = vec.erase(it);
			else
				++it;
		}
	}
};

