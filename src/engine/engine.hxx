#pragma once
#include "core/camera.hxx"
#include "math/vec3.hxx"

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

	camera* getCamera() const;

	void run();

	void stop();

private:
	bool startRenderer();

	void stopRenderer();

	void startMainLoop();

	void stopMainLoop();

	void calculateNewDeltaTime(double& NewDeltaTime);

	vk_renderer* _renderer;

	camera* _camera;

	bool _isRunning;
};