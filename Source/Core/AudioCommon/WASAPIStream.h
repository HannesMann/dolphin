// Copyright 2018 Dolphin Emulator Project
// Licensed under GPLv2+
// Refer to the license.txt file included.

#pragma once

#ifdef _WIN32
// clang-format off
#include <Windows.h>
#include <mmreg.h>
// clang-format on
#endif

#include <atomic>
#include <string>
#include <thread>
#include <vector>
#include <mutex>

#include "AudioCommon/SoundStream.h"

struct IAudioClient;
struct IAudioRenderClient;
struct IMMDevice;
struct IMMDeviceEnumerator;

class WASAPIStream final : public SoundStream
{
#ifdef _WIN32
public:
  explicit WASAPIStream();
  ~WASAPIStream();
  bool Init() override;
  bool SetRunning(bool running) override;
  void SoundLoop() override;
  void PushSamples(const short* samples, unsigned int num_samples) override;

  static bool isValid();
  static std::vector<std::string> GetAvailableDevices();
  static IMMDevice* GetDeviceByName(std::string name);

private:
  u32 m_requested_frames = 0;
  std::atomic<bool> m_running = false;
  std::atomic<bool> m_stopped = false;
  std::thread m_thread;

  std::vector<short> m_short_buffer;
  std::mutex m_short_buffer_mutex;
  u32 m_max_frames_in_flight = 0;

  IAudioClient* m_audio_client = nullptr;
  IAudioRenderClient* m_audio_renderer = nullptr;
  IMMDeviceEnumerator* m_enumerator = nullptr;
  HANDLE m_need_data_event = nullptr;
  WAVEFORMATEXTENSIBLE m_format;
#endif  // _WIN32
};
