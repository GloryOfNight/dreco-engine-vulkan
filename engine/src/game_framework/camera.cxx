#include "camera.hxx"

#include "core/engine.hxx"
#include "math/math.hxx"

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

	{ // update view
		de::math::transform viewTransform = getTransform();
		viewTransform._translation._x = -viewTransform._translation._x;
		viewTransform._translation._y = -viewTransform._translation._y;
		viewTransform._translation._z = -viewTransform._translation._z;
		viewTransform._rotation._pitch = -viewTransform._rotation._pitch;
		viewTransform._scale._x = -viewTransform._scale._x;

		_view = de::math::mat4::makeTransform(viewTransform);
	}

	{ // update projection
		const auto currentExtent = de::renderer::get()->getCurrentExtent();
		_projection = de::math::mat4::makeProjection(1.F, std::numeric_limits<float>::max(), static_cast<float>(currentExtent.width) / static_cast<float>(currentExtent.height), math::degreesToRadians(75.F));
	}
	de::engine::get()->getRenderer().setCameraData(_view, _projection);
}
