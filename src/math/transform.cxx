#include "transform.hxx"

transform::transform(const vec3& translation, const vec3& rotation, const vec3& scale)
	: _translation(translation)
	, _rotation(rotation)
	, _scale(scale)
{
}