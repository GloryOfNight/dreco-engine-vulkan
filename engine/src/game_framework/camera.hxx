#pragma once
#include "math/mat4.hxx"

#include "node.hxx"

namespace de::gf
{
	class DRECO_API camera : public node
	{
	public:
		de::math::mat4 getView() const;

		void tick(double deltaTime) override;

	private:
		de::math::mat4 _view;
	};
} // namespace de::gf
