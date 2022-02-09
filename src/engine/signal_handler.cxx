#include "signal_handler.hxx"

#include "core/utils/log.hxx"

#include "engine.hxx"

#include <csignal>

void signal_handler::registerSignalsHandle()
{
	static bool wasRegistered = false;
	if (wasRegistered)
		return;
	wasRegistered = true;
	std::signal(SIGINT, signal_handler::onSignal);
	std::signal(SIGILL, signal_handler::onSignal);
	std::signal(SIGFPE, signal_handler::onSignal);
	std::signal(SIGSEGV, signal_handler::onSignal);
	std::signal(SIGTERM, signal_handler::onSignal);
	std::signal(SIGBREAK, signal_handler::onSignal);
	std::signal(SIGABRT, signal_handler::onSignal);
}

void signal_handler::onSignal(int sig)
{
	if (sig == SIGBREAK)
		DE_LOG(Info, "Engine recieved break. . . Stopping engine.");
	else if (sig == SIGFPE)
		DE_LOG(Info, "Engine recieved floating point excetion. . . Stopping engine.");
	else if (sig == SIGSEGV)
		DE_LOG(Info, "Engine recieved segment violation. . . Stopping engine.");
	else if (sig == SIGABRT || sig == SIGTERM || sig == SIGINT)
		DE_LOG(Info, "Engine stop signal. . . Stopping engine.");
	stopEngine();
}

void signal_handler::stopEngine()
{
	auto* eng = engine::get();
	if (eng)
		eng->stop();
}
