#include "transform.hxx"

transform::transform()
	: _translation{}
	, _rotation{}
	, _scale{1.F, 1.F, 1.F}
{
}

transform::transform(const vec3& translation, const rotatorDeg& rotation, const vec3& scale)
	: _translation(translation)
	, _rotation(rotation)
	, _scale(scale)
{
}