#include "camera.hxx"

#include "core/engine.hxx"
#include "math/casts.hxx"

de::math::mat4 de::gf::camera::getView() const
{
	return _view;
}

void de::gf::camera::tick(double deltaTime)
{
	node::tick(deltaTime);

	const auto& transform = getTransform();
	_view = de::math::mat4::makeFirstPersonView(transform._translation, de::math::quat_cast(transform._rotation));

	de::engine::get()->getRenderer().setCameraView(_view);
}
