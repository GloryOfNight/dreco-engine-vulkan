#pragma once
#include "core/managers/event_manager.hxx"
#include "core/managers/input_manager.hxx"
#include "core/objects/debug_camera.hxx"
#include "core/threads/thread_pool.hxx"
#include "renderer/vulkan/vk_renderer.hxx"

#include "dreco.hxx"

#include <cstdint>

class DRECO_DECLSPEC engine
{
public:
	engine();
	engine(const engine&) = delete;
	engine(engine&&) = delete;
	~engine();

	engine& operator=(const engine&) = delete;
	engine& operator=(engine&&) = delete;

	static engine* get();

	const camera* getCamera() const;

	const vk_renderer& getRenderer() const { return _renderer; };
	vk_renderer& getRenderer() { return _renderer; };

	const thread_pool& getThreadPool() const { return _threadPool; };
	thread_pool& getThreadPool() { return _threadPool; };

	const event_manager& getEventManager() const { return _eventManager; };
	event_manager& getEventManager() { return _eventManager; };

	const input_manager& getInputManager() const { return _inputManager; };
	input_manager& getInputManager() { return _inputManager; };

	[[nodiscard]] bool init();

	void run();

	void stop();

private:
	bool startRenderer();

	void startMainLoop();

	void preMainLoop();

	void postMainLoop();

	double calculateNewDeltaTime();

	event_manager _eventManager;

	input_manager _inputManager;

	thread_pool _threadPool;

	vk_renderer _renderer;

	debug_camera _camera;

	bool _isRunning;
};