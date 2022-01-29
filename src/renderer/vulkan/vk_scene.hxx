#pragma once
#include "core/containers/gltf/model.hxx"
#include "vulkan/vulkan.h"

#include <vector>

class vk_texture_image;
class vk_graphics_pipeline;
class vk_mesh;

class vk_scene
{
public:
	vk_scene() = default;
	~vk_scene();

	void create(const model& m);

    void update();

	void recreatePipelines();

	void bindToCmdBuffer(VkCommandBuffer commandBuffer);

	bool isEmpty() const;

	void destroy();

	std::vector<vk_texture_image*>& getTextureImages() { return _textureImages; };
	const std::vector<vk_texture_image*>& getTextureImages() const { return _textureImages; }

	std::vector<vk_graphics_pipeline*>& getGraphicPipelines() { return _graphicsPipelines; };
	const std::vector<vk_graphics_pipeline*>& getGraphicPipelines() const { return _graphicsPipelines; }

    std::vector<vk_mesh*>& getMeshes() { return _meshes; };
	const std::vector<vk_mesh*>& getMeshes() const { return _meshes; }

private:
	void recurseSceneNodes(const model& m, const node& selfNode, const mat4& rootMat);

	std::vector<vk_texture_image*> _textureImages;
	std::vector<vk_graphics_pipeline*> _graphicsPipelines;
	std::vector<vk_mesh*> _meshes;
};
