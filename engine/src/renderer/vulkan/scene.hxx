#pragma once
#include "core/containers/gltf/model.hxx"
#include "math/transform.hxx"
#include "vulkan/vulkan.h"

#include "buffer.hxx"
#include "material.hxx"

#include <memory>
#include <vector>

namespace de::vulkan
{
	class texture_image;
	class material;
	class mesh;

	class scene final
	{
		class mesh final
		{
		public:
			void init(uint32_t vertexCount, size_t vertexSize, uint32_t vertexOffset, uint32_t indexCount, uint32_t indexOffset);

			void drawCmd(vk::CommandBuffer commandBuffer) const;

			// temporal hold of the mesh matrix (transform)
			de::math::mat4 _mat;

			vk::DeviceSize getVertexSize() const;
			vk::DeviceSize getIndexSize() const;

		private:
			uint32_t _vertexOffset{0};
			vk::DeviceSize _vertexSize{0};
			vk::DeviceSize _vertexCount{0};

			uint32_t _indexOffset{0};
			vk::DeviceSize _indexSize{0};
			vk::DeviceSize _indexCount{0};
		};

	public:
		scene() = default;
		~scene();

		void create(const de::gltf::model& m);

		void bindToCmdBuffer(vk::CommandBuffer commandBuffer);

		bool isEmpty() const;

		void destroy();

		std::vector<std::unique_ptr<texture_image>>& getTextureImages() { return _textureImages; };
		const std::vector<std::unique_ptr<texture_image>>& getTextureImages() const { return _textureImages; }
		const texture_image& getTextureImageFromIndex(uint32_t index) const;

	private:
		struct scene_meshes_info
		{
			uint32_t _totalVertexSize{0};
			std::vector<device_memory::map_memory_region> _vertexMemRegions;

			uint32_t _totalIndexSize{0};
			std::vector<device_memory::map_memory_region> _indexMemRegions;

			uint32_t _totalMaterialsSize{0};
			std::vector<device_memory::map_memory_region> _materialMemRegions;
		};

		void recurseSceneNodes(const de::gltf::model& m, const de::gltf::node& selfNode, const de::math::transform& rootTransform, scene_meshes_info& info);

		void createMeshesBuffer(const scene_meshes_info& info);
		void createMaterialsBuffer(const scene_meshes_info& info);

		std::vector<std::unique_ptr<texture_image>> _textureImages;

		std::vector<material_instance*> _matInstances;

		std::vector<std::vector<std::unique_ptr<mesh>>> _meshes;

		uint32_t _indexOffset;
		buffer::id _meshesVIBufferId;
		buffer::id _materialsBufferId;
	};
} // namespace de::vulkan