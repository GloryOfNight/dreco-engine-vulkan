#pragma once

#include "gltf/model.hxx"

#include "node.hxx"

#include <string>

namespace de
{
	namespace async
	{
		struct thread_task;
	} // namespace async

	namespace gf
	{
		class DRECO_API gltf_model : public node
		{
		public:
			template <typename Str>
			gltf_model(Str&& modelPath)
				: _modelPath{modelPath}
			{
			}

			virtual void init() override;

		private:
			void onModelLoaded(de::async::thread_task* task);

			std::string _modelPath;

			de::gltf::model _model;
		};
	} // namespace gf
} // namespace de