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

		void setViewId(uint32_t viewId);

		uint32_t getViewId() const { return _viewId; }

	private:
		de::math::mat4 _view;

		uint32_t _viewId = 0;
	};
} // namespace de::gf
