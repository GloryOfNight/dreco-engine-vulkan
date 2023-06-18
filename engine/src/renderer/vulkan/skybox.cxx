#include "skybox.hxx"

#include "constants.hxx"
#include "renderer.hxx"

void de::vulkan::skybox::init()
{
	auto renderer = renderer::get();

	std::array<std::string, 6> cubemaptexs{
		"skyboxes/sea/right.jpg",
		"skyboxes/sea/left.jpg",
		"skyboxes/sea/top.jpg",
		"skyboxes/sea/bottom.jpg",
		"skyboxes/sea/front.jpg",
		"skyboxes/sea/back.jpg"
	};

	_cubemap.create(cubemaptexs);

	const auto skyboxMat = renderer->getMaterial(de::vulkan::constants::materials::skybox);

	_matInst = skyboxMat->makeInstance();

	_matInst->setBufferDependency("cameraData", &renderer->getCameraDataBuffer());
	_matInst->setImageDependecy("cubemap", &_cubemap);
	_matInst->updateDescriptorSets();

	createBox();
}

void de::vulkan::skybox::destroy()
{
	_cubemap.destroy();
	_matInst = nullptr;
	renderer::get()->getVertIndxBufferPool().freeBuffer(_boxMeshId);
}

void de::vulkan::skybox::drawCmd(vk::CommandBuffer commandBuffer)
{
	const auto& cubeBuffer = renderer::get()->getVertIndxBufferPool().getBuffer(_boxMeshId);

	commandBuffer.bindVertexBuffers(0, cubeBuffer.get(), {cubeBuffer.getOffset()});

	_matInst->getMaterial()->bindCmd(commandBuffer);
	_matInst->bindCmd(commandBuffer);

	commandBuffer.setDepthTestEnable(false);
	commandBuffer.draw(_vertSize, 1, 0, 0);
	commandBuffer.setDepthTestEnable(true);
}

void de::vulkan::skybox::createBox()
{
	constexpr std::array<float, 36 * 3> vertices = {
		-1.0f, 1.0f, -1.0f,
		-1.0f, -1.0f, -1.0f,
		1.0f, -1.0f, -1.0f,
		1.0f, -1.0f, -1.0f,
		1.0f, 1.0f, -1.0f,
		-1.0f, 1.0f, -1.0f,
		//
		-1.0f, -1.0f, 1.0f,
		-1.0f, -1.0f, -1.0f,
		-1.0f, 1.0f, -1.0f,
		-1.0f, 1.0f, -1.0f,
		-1.0f, 1.0f, 1.0f,
		-1.0f, -1.0f, 1.0f,
		//
		1.0f, -1.0f, -1.0f,
		1.0f, -1.0f, 1.0f,
		1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, -1.0f,
		1.0f, -1.0f, -1.0f,
		//
		-1.0f, -1.0f, 1.0f,
		-1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 1.0f,
		1.0f, -1.0f, 1.0f,
		-1.0f, -1.0f, 1.0f,
		//
		-1.0f, 1.0f, -1.0f,
		1.0f, 1.0f, -1.0f,
		1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 1.0f,
		-1.0f, 1.0f, 1.0f,
		-1.0f, 1.0f, -1.0f,
		//
		-1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f, 1.0f,
		1.0f, -1.0f, -1.0f,
		1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f, 1.0f,
		1.0f, -1.0f, 1.0f
	};

	_vertSize = sizeof(float) * vertices.size();

	const vk::DeviceSize size = _vertSize;

	auto renderer = renderer::get();
	auto& bpTransfer = renderer->getTransferBufferPool();
	auto& bpVertIndx = renderer->getVertIndxBufferPool();

	const auto transferBufferId = bpTransfer.makeBuffer(size);
	auto region = bpTransfer.map(transferBufferId);
	memcpy(region, vertices.data(), _vertSize);
	//memcpy(reinterpret_cast<uint8_t*>(region) + _indexOffset, indices.data(), _indxSize);
	bpTransfer.unmap(transferBufferId);

	_boxMeshId = bpVertIndx.makeBuffer(size);

	const vk::BufferCopy copyRegion = vk::BufferCopy(0, 0, size);
	de::vulkan::buffer::copyBuffer(bpTransfer.getBuffer(transferBufferId).get(), bpVertIndx.getBuffer(_boxMeshId).get(), {copyRegion});

	bpTransfer.freeBuffer(transferBufferId);
}
