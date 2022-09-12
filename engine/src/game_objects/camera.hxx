#pragma once
#include "math/mat4.hxx"

#include "root_node.hxx"

class DRECO_API camera : public root_node
{
public:
	camera(world& w, root_node* owner = nullptr)
		: root_node(w, owner)
	{
	}

	mat4 getView() const;

	mat4 getProjection() const;

	void tick(double deltaTime) override;

private:
	mat4 _view;
	mat4 _projection;
};