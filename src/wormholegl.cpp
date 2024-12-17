// wormholegl Copyright (C) 2024 kernaltrap8
// This program comes with ABSOLUTELY NO WARRANTY
// This is free software, and you are welcome to redistribute it
// under certain conditions

/*
    wormholegl.cpp
*/

#include "wormholegl.hpp"
#include <cmath>

namespace Wormhole {
auto InitCommandLine() {
  CommandLineArgs["-v"] = []() {
    std::cout << VERSION_BANNER << std::endl;
    exit(0);
  };
  CommandLineArgs["--version"] = CommandLineArgs["-v"];
}
auto CheckCommandLineArgs(int argc, char *argv[]) {
  InitCommandLine(); // Initialize the map with arguments
  for (int i = 1; i < argc; ++i) {
    std::string command = argv[i];

    // Check if the command exists in the map
    if (CommandLineArgs.find(command) != CommandLineArgs.end()) {
      CommandLineArgs[command](); // Call the associated function
    } else {
      std::cerr << "wormholegl: invalid argument: " << command << std::endl;
    }
  }
} // CheckCommandLineArgs
namespace Pulse {
pa_simple *init_pulse_audio(int &error) {
  // Traditional initialization instead of designated initializer
  static const pa_sample_spec sampleSpec = {
      PA_SAMPLE_S16LE, // format
      44100,           // rate
      1                // channels
  };

  pa_simple *paStream =
      pa_simple_new(nullptr,          // Use default server
                    "AudioCapture",   // Application name
                    PA_STREAM_RECORD, // Record stream
                    nullptr,          // Use default device
                    "Recording",      // Stream description
                    &sampleSpec,      // Sample format
                    nullptr,          // Use default channel map
                    nullptr,          // Use default buffering attributes
                    &error);

  if (!paStream) {
    std::cerr << "PulseAudio connection failed: " << pa_strerror(error)
              << std::endl;
    return nullptr;
  }

  return paStream;
} // init_pulse_audio
auto process_audio(pa_simple *paStream) {
  const size_t bufferSize = 1024;              // Number of bytes per read
  std::vector<int16_t> buffer(bufferSize / 2); // 16-bit samples (2 bytes each)
  int error;

  std::cout << "Listening to audio input (Ctrl+C to stop)...\n";

  while (keep_running) {
    // Read audio data into buffer
    if (pa_simple_read(paStream, buffer.data(), bufferSize, &error) < 0) {
      std::cerr << "Failed to read data: " << pa_strerror(error) << std::endl;
      break;
    }

    // Process and print the samples
    for (size_t i = 0; i < buffer.size(); i++) {
      // Normalize sample to [-1.0, 1.0]
      float value = buffer[i] / 32768.0f; // 16-bit signed range
      std::cout << value << " ";
      if (i > 20)
        break; // Print only the first 20 samples
    }
    std::cout << std::endl;
  }
} // process_audio
auto cleanup_pulse_audio(pa_simple *paStream) {
  if (paStream) {
    pa_simple_free(paStream);
  }
} // cleanup_pulse_audio
auto Init(void) {
  // Set up signal handler to gracefully exit on Ctrl+C
  signal(SIGINT, handle_signal);

  int error;
  pa_simple *paStream = init_pulse_audio(error);

  if (paStream) {
    process_audio(paStream);       // Process audio while capturing
    cleanup_pulse_audio(paStream); // Clean up the stream when done
  }
} // Init
} // namespace Pulse
} // namespace Wormhole

auto main(int argc, char *argv[]) -> int {
  Wormhole::CheckCommandLineArgs(argc, argv);
  Wormhole::Pulse::Init();
  return 0;
}
