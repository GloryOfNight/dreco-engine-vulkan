#pragma once

#include "math/mat4.hxx"
#include "math/transform.hxx"

class world_object
{
public:
	void setTransform(const transform& newTransform);

	void setPosition(const vec3& pos);

	void setRotation(const rotator& rot);

	void setScale(const vec3& scale);

	const transform& getTransform() const;

private:
	transform _transform;
};