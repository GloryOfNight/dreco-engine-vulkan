#pragma once
#include "math/vec3.hxx"

#include "dreco.h"

#include <cstdint>

class vk_renderer;

class DRECO_DECLSPEC engine
{
public:
	engine();
	~engine();
	
	static engine* get();

	vk_renderer* getRenderer() const;
	
	void run();

	void stop();

	vec3 shapeTranslation{0, 0, 0};

private:
	bool startRenderer();

	void stopRenderer();

	void startMainLoop();

	void stopMainLoop();

	void calculateNewDeltaTime(float& NewDeltaTime);

	vk_renderer* _renderer;

	bool isRunning{false};

	uint64_t lastTickTime{0};
};