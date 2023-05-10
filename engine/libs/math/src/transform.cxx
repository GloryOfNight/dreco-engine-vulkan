#include "transform.hxx"

de::math::transform::transform()
	: _translation{}
	, _rotation{}
	, _scale{1.F, 1.F, 1.F}
{
}

de::math::transform::transform(const vec3& translation, const euler& rotation, const vec3& scale)
	: _translation(translation)
	, _rotation(rotation)
	, _scale(scale)
{
}