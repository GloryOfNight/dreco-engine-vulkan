#pragma once
#include "core/objects/camera.hxx"
#include "math/vec3.hxx"
#include "core/threads/thread_pool.hxx"

#include "dreco.h"

#include <cstdint>

class vk_renderer;

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

	vk_renderer* getRenderer() const;

	const camera* getCamera() const;

	void run();

	void stop();

private:
	bool startRenderer();

	void stopRenderer();

	void startMainLoop();

	void preMainLoop();

	void stopMainLoop();

	void postMainLoop();

	double calculateNewDeltaTime();

	thread_pool* _thread_pool;

	vk_renderer* _renderer;

	camera _camera;

	bool _isRunning;
};