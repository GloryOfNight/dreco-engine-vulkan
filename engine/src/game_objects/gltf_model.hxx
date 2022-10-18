#pragma once

#include "core/containers/gltf/model.hxx"

#include "node.hxx"

#include <string>

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
	void onModelLoaded(class thread_task* task);

	std::string _modelPath;

	gltf::model _model;
};