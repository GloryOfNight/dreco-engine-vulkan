#pragma once
#include "core/async/thread_pool.hxx"
#include "core/managers/event_manager.hxx"
#include "core/managers/input_manager.hxx"
#include "core/misc/fps_counter.hxx"
#include "game_framework/game_instance.hxx"
#include "renderer/render.hxx"

#include "dreco.hxx"

#include <cstdint>
#include <utility>

namespace de
{
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
			ok,
			already_initialized,
			failed_already_running,
			failed_init_sdl,
			failed_find_cwd,
		};

		enum class run_res
		{
			ok,
			failed_unitialized,
			failed_invalid_game_instance,
			failed_make_new_game_instance
		};

		engine();
		engine(const engine&) = delete;
		engine(engine&&) = delete;
		~engine();

		engine& operator=(const engine&) = delete;
		engine& operator=(engine&&) = delete;

		static engine* get();

		const de::renderer& getRenderer() const { return _renderer; };
		de::renderer& getRenderer() { return _renderer; };

		const de::async::thread_pool& getThreadPool() const { return _threadPool; };
		de::async::thread_pool& getThreadPool() { return _threadPool; };

		const event_manager& getEventManager() const { return _eventManager; };
		event_manager& getEventManager() { return _eventManager; };

		const input_manager& getInputManager() const { return _inputManager; };
		input_manager& getInputManager() { return _inputManager; };

		uint64_t getFrameCount() const { return _frameCounter; };

		[[nodiscard]] init_res initialize();

		[[nodiscard]] run_res run();

		void stop();

		defaultObject<de::gf::game_instance> _defaultGameInstance;

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

		async::thread_pool _threadPool;

		de::renderer _renderer;

		gf::game_instance::unique _gameInstance;

		uint64_t _frameCounter{};

		de::misc::fps_counter _fpsCounter;

		bool _isRunning{};
	};
} // namespace de