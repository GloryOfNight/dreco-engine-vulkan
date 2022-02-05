#include "world_object.hxx"

void world_object::setTransform(const transform& newTransform)
{
	_transform = newTransform;
}

void world_object::setPosition(const vec3& pos)
{
	_transform._translation = pos;
}

void world_object::setRotation(const rotatorDeg& rot)
{
	_transform._rotation = rot;
}

void world_object::setScale(const vec3& scale)
{
	_transform._scale = scale;
}

void world_object::tick(const double deltaTime)
{
}

const transform& world_object::getTransform() const
{
	return _transform;
}
