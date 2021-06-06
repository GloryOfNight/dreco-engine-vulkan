#pragma once
#include "world_object.hxx"

class camera : public world_object
{
public:
	mat4 getView() const;

	mat4 getProjection() const;
};