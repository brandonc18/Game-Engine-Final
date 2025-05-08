#pragma once

#include "Entity.h"

struct Intersect
{
	bool result = false;
	Vec2f position;
};

namespace Physics
{
	Vec2f GetOverlap(Entity* a, Entity* b)
	{
		//setup variable for player's collision box
		Vec2f entityABox = a->get<CBoundingBox>().size;
		Vec2f entityAPos = a->get<CTransform>().pos;
		//setup variables for the other entities collision box
		Vec2f entityBBox = b->get<CBoundingBox>().size;
		Vec2f entityBPos = b->get<CTransform>().pos;
		//the collision bool will always start as false
		bool collides = false;
		//get the distance between the positions of the entities on the x and y axis
		float distanceX = abs(entityBPos.x - entityAPos.x);
		float distanceY = abs(entityBPos.y - entityAPos.y);
		//overlap varibles
		float xOverlap = 0;
		float yOverlap = 0;
		//x coordinates
		xOverlap = (entityBBox.x / 2) + (entityABox.x / 2) - distanceX;
		//y coordinates
		yOverlap = (entityBBox.y / 2) + (entityABox.y / 2) - distanceY;
		return Vec2f(xOverlap, yOverlap);
	}

	Vec2f GetPreviousOverlap(Entity* a, Entity* b)
	{
		//setup variable for player's collision box
		Vec2f entityABox = a->get<CBoundingBox>().size;
		Vec2f entityAPos = a->get<CTransform>().prevPos;
		//setup variables for the other entities collision box
		Vec2f entityBBox = b->get<CBoundingBox>().size;
		Vec2f entityBPos = b->get<CTransform>().prevPos;
		//the collision bool will always start as false
		bool collides = false;
		//get the distance between the positions of the entities on the x and y axis
		float distanceX = abs(entityBPos.x - entityAPos.x);
		float distanceY = abs(entityBPos.y - entityAPos.y);
		//overlap varibles
		float xOverlap = 0;
		float yOverlap = 0;
		//x coordinates
		xOverlap = (entityBBox.x / 2) + (entityABox.x / 2) - distanceX;
		//y coordinates
		yOverlap = (entityBBox.y / 2) + (entityABox.y / 2) - distanceY;
		return Vec2f(xOverlap, yOverlap);
	}

	bool IsInside(const Vec2f& pos, Entity* e)
	{
		//literally just see if a point is inside an entity
		//get distance between the point and entity center
		Vec2f dist = pos - e->get<CTransform>().pos;
		dist.x = abs(dist.x);
		dist.y = abs(dist.y);
		if (dist.x < (e->get<CAnimation>().animation.getSize().x / 2) && dist.y < (e->get<CAnimation>().animation.getSize().y / 2))
		{
			return true;
		}
		return false;
	}

	float CrossProduct(const Vec2f& a, const Vec2f& b)
	{
		float crossProduct = (a.x * b.y) - (a.y * b.x);
		return crossProduct;
	}

	Intersect LineIntersection(const Vec2f& a, const Vec2f& b, const Vec2f& c, const Vec2f& d)
	{
		//returns an intersect that has a bool for if it intersects as well as where it intersected if there was one
		Intersect intersection;
		//hypotenuse of each line segment
		Vec2f r = b - a;
		Vec2f s = d - c;
		//scalars we are solving for
		float t = -1;
		float u = -1;
		//check to see if there is an intersection, there is one if both u and t are between 0-1
		u = CrossProduct((c - a), r) / Physics::CrossProduct(r,s);
		t = CrossProduct((c - a), s) / Physics::CrossProduct(r,s);
		//if there is an intersection, calculate the position of the intersection
		if (u <= 1 && u >= 0 && t <= 1 && t >= 0) 
		{
			intersection.result = true;
			intersection.position = a + r * t;
		}
		else
		{
			intersection.result = false;
			intersection.position = { 0,0 };
		}

		return intersection;
	}

	Intersect EntityIntersect(const Vec2f& a, const Vec2f& b, Entity* e)
	{
		//backup intersect if no intersect is found
		Intersect noIntersect;
		noIntersect.position = Vec2f(0, 0);
		noIntersect.result = false;
		//get all 4 points of the entity's bounding box
		Vec2f topLeft = Vec2f (e->get<CTransform>().pos.x - (e->get<CAnimation>().animation.getSize().x / 2), e->get<CTransform>().pos.y - (e->get<CAnimation>().animation.getSize().y / 2));
		Vec2f topRight = Vec2f(e->get<CTransform>().pos.x + (e->get<CAnimation>().animation.getSize().x / 2), e->get<CTransform>().pos.y - (e->get<CAnimation>().animation.getSize().y / 2));;
		Vec2f bottomLeft = Vec2f(e->get<CTransform>().pos.x - (e->get<CAnimation>().animation.getSize().x / 2), e->get<CTransform>().pos.y + (e->get<CAnimation>().animation.getSize().y / 2));;
		Vec2f bottomRight = Vec2f(e->get<CTransform>().pos.x + (e->get<CAnimation>().animation.getSize().x / 2), e->get<CTransform>().pos.y + (e->get<CAnimation>().animation.getSize().y / 2));;

		//check if a line given through 2 points intersects with an entity
		//An entity is made up of 4 line segments, so call LineIntersection 4 times
		if (LineIntersection(a, b, topLeft, topRight).result == true)
		{ return LineIntersection(a, b, topLeft, topRight); }
		else if (LineIntersection(a, b, topLeft, bottomLeft).result == true)
		{ return LineIntersection(a, b, topLeft, bottomLeft); }
		else if (LineIntersection(a, b, topRight, bottomRight).result == true)
		{ return LineIntersection(a, b, topRight, bottomRight); }
		else if (LineIntersection(a, b, bottomLeft, bottomRight).result == true)
		{ return LineIntersection(a, b, topRight, bottomRight); }
		//if no intersect, return false
		return noIntersect;
	}
}
