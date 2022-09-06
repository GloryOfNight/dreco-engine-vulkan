#pragma once
#include "math/mat4.hxx"

#include "node_base.hxx"

class camera : public node_base
{
public:
	camera(world& w, node_base* owner = nullptr)
		: node_base(w, owner)
	{
	}

	mat4 getView() const;

	mat4 getProjection() const;

	void tick(double deltaTime) override;

private:
	mat4 _view;
	mat4 _projection;
};