// wormholegl Copyright (C) 2024 kernaltrap8
// This program comes with ABSOLUTELY NO WARRANTY
// This is free software, and you are welcome to redistribute it
// under certain conditions

/*
    wormholegl.hpp
*/
#ifndef WORMHOLEGL_HPP
#define WORMHOLEGL_HPP
#include <cstdlib>
#include <iostream>
#include <pulse/error.h>
#include <pulse/simple.h>
#include <signal.h>
#include <string>
#include <unordered_map>
#include <vector>
#define VERSION_BANNER "wormholegl v0.1"

volatile sig_atomic_t keep_running = 1;

void handle_signal(int) { keep_running = 0; }

namespace Wormhole {
std::unordered_map<std::string, void (*)(char *arg)> CommandLineArgs;
auto CheckCommandLineArgs(int argc, char *argv[]);
// This will be called within Wormhole::CheckCommandLineArgs
auto InitCommandLine();
namespace Pulse {
pa_simple *init_ulse_audio(int &error);
auto process_audio(pa_simple *paStream);
auto cleanup_pulse_audio(pa_simple &paStream);
auto Init(void);
} // namespace Pulse
} // namespace Wormhole
#endif // WORMHOLE_HPP
