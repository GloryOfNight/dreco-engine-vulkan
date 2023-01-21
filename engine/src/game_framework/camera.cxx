#include "camera.hxx"

#include "core/engine.hxx"
#include "math/casts.hxx"

de::math::mat4 de::gf::camera::getView() const
{
	return _view;
}

de::math::mat4 de::gf::camera::getProjection() const
{
	return _projection;
}

void de::gf::camera::tick(double deltaTime)
{
	node::tick(deltaTime);

	const auto& transform = getTransform();
	_view = de::math::mat4::makeFirstPersonView(transform._translation, de::math::quat_cast(transform._rotation));

	const auto currentExtent = de::renderer::get()->getCurrentExtent();
	_projection = de::math::mat4::makeProjection(0.1f, 1000.f, static_cast<float>(currentExtent.width) / static_cast<float>(currentExtent.height), math::deg_to_rad(75.F));

	de::engine::get()->getRenderer().setCameraData(_view, _projection);
}
