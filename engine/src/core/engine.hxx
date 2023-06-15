#pragma once
#include "core/managers/event_manager.hxx"
#include "core/managers/input_manager.hxx"
#include "core/misc/fps_counter.hxx"
#include "game_framework/game_instance.hxx"
#include "renderer/render.hxx"
#include "threads/thread_pool.hxx"

#include "dreco.hxx"

#include <cstdint>
#include <utility>

namespace de
{
	class DRECO_API engine
	{
	public:
		engine();
		engine(const engine&) = delete;
		engine(engine&&) = delete;
		~engine();

		static engine* get();

		const de::renderer& getRenderer() const { return _renderer; };
		de::renderer& getRenderer() { return _renderer; };

		const de::async::thread_pool& getThreadPool() const { return _threadPool; };
		de::async::thread_pool& getThreadPool() { return _threadPool; };

		const de::event_manager& getEventManager() const { return _eventManager; };
		de::event_manager& getEventManager() { return _eventManager; };

		const de::input_manager& getInputManager() const { return _inputManager; };
		de::input_manager& getInputManager() { return _inputManager; };

		uint64_t getFrameCount() const { return _frameCounter; };

		SDL_Window* getWindow() const { return _windows[0]; }

		void initialize();

		void run();

		void stop();

		void setCreateGameInstanceFunc(std::function<de::gf::game_instance::unique()> func);

	private:
		static void onSystemSignal(int sig);

		void registerSignals();

		bool startRenderer();

		void startMainLoop();

		void preMainLoop();

		void postMainLoop();

		double calculateNewDeltaTime() noexcept;

		std::array<SDL_Window*, 16> _windows{};

		event_manager _eventManager;

		input_manager _inputManager;

		async::thread_pool _threadPool;

		de::renderer _renderer;

		de::gf::game_instance::unique _gameInstance;

		std::function<de::gf::game_instance::unique()> _createGameInstanceFunc;

		uint64_t _frameCounter{};

		de::misc::fps_counter _fpsCounter;

		bool _isRunning{};
	};
} // namespace de