#pragma once
#include "math/mat4.hxx"

#include "entity.hxx"

class camera : public entity
{
public:
	camera(world& w, entity* owner = nullptr)
		: entity(w, owner)
	{
	}

	mat4 getView() const;

	mat4 getProjection() const;

	void tick(double deltaTime) override;

private:
	mat4 _view;
	mat4 _projection;
};