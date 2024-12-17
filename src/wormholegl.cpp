// wormholegl Copyright (C) 2024 kernaltrap8
// This program comes with ABSOLUTELY NO WARRANTY
// This is free software, and you are welcome to redistribute it
// under certain conditions

/*
    wormholegl.cpp
*/

#include "wormholegl.hpp"
#include <iomanip>
#include <chrono>
#include <cmath>

static char *pa_device = NULL;

namespace Wormhole {
auto InitCommandLine() {
  CommandLineArgs["-v"] = [](char *) {
    std::cout << VERSION_BANNER << std::endl;
    exit(0);
  };
  CommandLineArgs["-d"] = [](char *arg) {
    pa_device = arg;
  };
  CommandLineArgs["--version"] = CommandLineArgs["-v"];
  CommandLineArgs["--device"] = CommandLineArgs["-d"];
}
auto CheckCommandLineArgs(int argc, char *argv[]) {
  InitCommandLine(); // Initialize the map with arguments
  for (int i = 1; i < argc; ++i) {
    std::string command = argv[i];

    // Check if the command exists in the map
    if (CommandLineArgs.find(command) != CommandLineArgs.end()) {
      CommandLineArgs[command](argv[i + 1]); // Call the associated function
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
    48000,           // rate
    2                // channels
  };
  static const struct pa_buffer_attr pa_atrrs = {
    (uint32_t) -1,   // maxlength - Maximum length of the buffer in bytes.
    (uint32_t) -1,   // tlength - Playback only: target length of the buffer.
    (uint32_t) -1,   // prebuf - Playback only: pre-buffering.
    (uint32_t) -1,   // minreq - Playback only: minimum request.
    (uint32_t) 64               // fragsize - Recording only: fragment size.
  };

  pa_simple *paStream =
      pa_simple_new(nullptr,          // Use default server
                    "AudioCapture",   // Application name
                    PA_STREAM_RECORD, // Record stream
                    pa_device,          // Use default device
                    "Recording",      // Stream description
                    &sampleSpec,      // Sample format
                    nullptr,          // Use default channel map
                    &pa_atrrs,  // Use default buffering attributes
                    &error);

  if (!paStream) {
    std::cerr << "PulseAudio connection failed: " << pa_strerror(error)
              << std::endl;
    return nullptr;
  }

  return paStream;
} // init_pulse_audio
auto process_audio(pa_simple *paStream) {
  const size_t bufferSize = 64;              // Number of bytes per read
  std::vector<uint8_t> buffer(bufferSize); // 16-bit samples (2 bytes each)
  int error;

  std::cout << "Listening to audio input (Ctrl+C to stop)...\n";
  while (keep_running) {
    auto start = std::chrono::high_resolution_clock::now();
    // Read audio data into buffer
    if (pa_simple_read(paStream, buffer.data(), bufferSize, &error) < 0) {
      std::cerr << "Failed to read data: " << pa_strerror(error) << std::endl;
      break;
    }

    // Process and print the samples
    for (size_t i = 0; i < buffer.size(); i++) {
      // Normalize sample to [-1.0, 1.0]
      auto value = buffer[i] / 255.0f; // 16-bit signed range
      std::cout << std::fixed << std::setprecision(7) << value << " ";
      if (i == 20)
        break; // Print only the first 20 samples
    }
    std::cout << std::endl;
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    if ((1000000 / 48000) * (64 / 8) > duration.count())
      usleep((1000000 / 48000) * (64 / 8) - duration.count());
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
