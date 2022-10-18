#pragma once
#include "math/mat4.hxx"

#include "node.hxx"

class DRECO_API camera : public node
{
public:
	mat4 getView() const;

	mat4 getProjection() const;

	void tick(double deltaTime) override;

private:
	mat4 _view;
	mat4 _projection;
};