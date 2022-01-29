#pragma once
#include "world_object.hxx"

class camera : public world_object
{
public:
	mat4 getView() const;

	mat4 getProjection() const;

	void tick(double deltaTime) override;

protected:
	float farZ = 10000.F;

private:
	mat4 _view;
	mat4 _projection;
};