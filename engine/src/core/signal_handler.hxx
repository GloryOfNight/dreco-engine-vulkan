#pragma once

struct signal_handler
{
	static void registerSignalsHandle();

private:
	static void onSignal(int sig);

	static void stopEngine();
};