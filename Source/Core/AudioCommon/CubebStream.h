// Copyright 2017 Dolphin Emulator Project
// Licensed under GPLv2+
// Refer to the license.txt file included.

#pragma once

#include <cstddef>
#include <memory>
#include <vector>
#include <mutex>

#include "AudioCommon/SoundStream.h"

#include <cubeb/cubeb.h>

class CubebStream final : public SoundStream
{
public:
  ~CubebStream() override;
  bool Init() override;
  bool SetRunning(bool running) override;
  void SetVolume(int) override;
  void PushSamples(const short* samples, unsigned int num_samples) override;

private:
  bool m_stereo = false;
  std::shared_ptr<cubeb> m_ctx;
  cubeb_stream* m_stream = nullptr;

  std::vector<short> m_short_buffer;
  std::mutex m_short_buffer_mutex;
  u32 m_max_frames_in_flight = 0;

  static long DataCallback(cubeb_stream* stream, void* user_data, const void* /*input_buffer*/,
                           void* output_buffer, long num_frames);
  static void StateCallback(cubeb_stream* stream, void* user_data, cubeb_state state);
};
