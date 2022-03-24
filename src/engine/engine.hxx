#pragma once
#include "core/managers/event_manager.hxx"
#include "core/managers/input_manager.hxx"
#include "core/objects/game_instance.hxx"
#include "core/threads/thread_pool.hxx"
#include "renderer/vulkan/vk_renderer.hxx"

#include "dreco.hxx"

#include <cstdint>

class DRECO_API engine
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

	template<class T>
	void runGI();

	void stop();

private:
	void run();

	bool startRenderer();

	void startMainLoop();

	void preMainLoop();

	void postMainLoop();

	double calculateNewDeltaTime();

	event_manager _eventManager;

	input_manager _inputManager;

	thread_pool _threadPool;

	vk_renderer _renderer;

	bool _isRunning;

	std::unique_ptr<game_instance> _gameInstance;
};

template <class T>
inline void engine::runGI()
{
	static_assert(std::is_base_of<game_instance, T>(), "T should be direved from game instance");
	if (this)
	{
		_gameInstance.reset(new T(*this));
		run();
	}
}
