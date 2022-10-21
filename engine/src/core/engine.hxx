#pragma once
#include "core/managers/event_manager.hxx"
#include "core/managers/input_manager.hxx"
#include "core/threads/thread_pool.hxx"
#include "game_objects/game_instance.hxx"
#include "renderer/vk_renderer.hxx"

#include "dreco.hxx"

#include <cstdint>
#include <utility>

template <typename base>
struct defaultObject
{
	template <typename T>
	void init()
	{
		decltype(std::declval<T>().makeNew()) val = T().makeNew();
		obj = std::unique_ptr<base>(std::move(val));
	}
	bool isSet() const { return obj != nullptr; }
	std::unique_ptr<base> makeNew() const { return obj->makeNew(); };

private:
	std::unique_ptr<base> obj{nullptr};
};

class DRECO_API engine
{
public:
	enum class init_res
	{
		Ok,
		AlreadyInitialized,
		AlreadyRunning,
		FailedInitSDL,
		FailedFindCWD,
	};

	enum class run_res
	{
		Ok,
		Unitialized,
		InvalidGameInstance,
		FailedMakeNewGameInstance
	};


	engine();
	engine(const engine&) = delete;
	engine(engine&&) = delete;
	~engine();

	engine& operator=(const engine&) = delete;
	engine& operator=(engine&&) = delete;

	static engine* get();

	const vk_renderer& getRenderer() const { return _renderer; };
	vk_renderer& getRenderer() { return _renderer; };

	const thread_pool& getThreadPool() const { return _threadPool; };
	thread_pool& getThreadPool() { return _threadPool; };

	const event_manager& getEventManager() const { return _eventManager; };
	event_manager& getEventManager() { return _eventManager; };

	const input_manager& getInputManager() const { return _inputManager; };
	input_manager& getInputManager() { return _inputManager; };

	uint64_t getFrameCount() const { return _frameCounter; };

	[[nodiscard]] init_res initialize();

	[[nodiscard]] run_res run();

	void stop();

	defaultObject<game_instance> _defaultGameInstance;

private:
	static void onSystemSignal(int sig);

	void registerSignals();

	bool startRenderer();

	void startMainLoop();

	void preMainLoop();

	void postMainLoop();

	double calculateNewDeltaTime() noexcept;

	event_manager _eventManager;

	input_manager _inputManager;

	thread_pool _threadPool;

	vk_renderer _renderer;

	game_instance::unique _gameInstance;

	uint64_t _frameCounter{};

	bool _isRunning{};
};