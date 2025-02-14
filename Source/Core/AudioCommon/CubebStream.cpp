// Copyright 2017 Dolphin Emulator Project
// Licensed under GPLv2+
// Refer to the license.txt file included.

#include <cubeb/cubeb.h>
#include <algorithm>
#include <cmath>

#include "AudioCommon/CubebStream.h"
#include "AudioCommon/CubebUtils.h"
#include "Common/CommonTypes.h"
#include "Common/Logging/Log.h"
#include "Common/Thread.h"
#include "Core/ConfigManager.h"
#include "Core/Core.h"
#include "Common/Swap.h"

constexpr u32 ONE_DSP_BUFFER = 160;

long CubebStream::DataCallback(cubeb_stream* stream, void* user_data, const void* /*input_buffer*/,
                               void* output_buffer, long num_frames)
{
  auto* self = static_cast<CubebStream*>(user_data);

  if (self->m_stereo)
  {
    //NOTICE_LOG(AUDIO, "Cubeb wants %lu frames", num_frames);
    std::lock_guard<std::mutex> guard(self->m_short_buffer_mutex);

    if(self->m_short_buffer.size() < 2)
    {
      self->m_short_buffer.push_back(0); // need at least one frame/two samples
      self->m_short_buffer.push_back(0);
    }

    for(std::size_t sample = 0; sample < num_frames * 2; sample += 2)
    {
      // TODO: maybe not a good idea to reuse the last sample
      static_cast<short*>(output_buffer)[sample] =
        Common::swap16(sample + 1 >= self->m_short_buffer.size() ? self->m_short_buffer[self->m_short_buffer.size() - 1] : self->m_short_buffer[sample + 1]);
      static_cast<short*>(output_buffer)[sample + 1] =
        Common::swap16(sample >= self->m_short_buffer.size() ? self->m_short_buffer[self->m_short_buffer.size() - 2] : self->m_short_buffer[sample]);
    }

    if (self->m_short_buffer.size() < num_frames * 2)
    {
      //NOTICE_LOG(AUDIO, "Underflow from our side by %lu frames", num_frames - (self->m_short_buffer.size() / 2));
      self->m_short_buffer.erase(self->m_short_buffer.begin(), self->m_short_buffer.end());
    }
    else
      self->m_short_buffer.erase(self->m_short_buffer.begin(), self->m_short_buffer.begin() + num_frames * 2);
  }
  else { /* TODO */ }

  return num_frames;
}

void CubebStream::StateCallback(cubeb_stream* stream, void* user_data, cubeb_state state)
{
}

bool CubebStream::Init()
{
  m_ctx = CubebUtils::GetContext();
  if (!m_ctx)
    return false;

  m_stereo = !SConfig::GetInstance().ShouldUseDPL2Decoder();

  cubeb_stream_params params;
  params.rate = 32000;
  if (m_stereo)
  {
    params.channels = 2;
    params.format = CUBEB_SAMPLE_S16NE;
    params.layout = CUBEB_LAYOUT_STEREO;
  }
  else
  {
    params.channels = 6;
    params.format = CUBEB_SAMPLE_FLOAT32NE;
    params.layout = CUBEB_LAYOUT_3F2_LFE;
  }

  u32 minimum_latency = 0;
  if (cubeb_get_min_latency(m_ctx.get(), &params, &minimum_latency) != CUBEB_OK)
    ERROR_LOG(AUDIO, "Error getting minimum latency");
  INFO_LOG(AUDIO, "Minimum latency: %i frames", minimum_latency);

  // SSBM gives us buffers every 5 ms, we keep at least one extra buffer to prevent underruns = minimum 10 ms latency.
  // Latency is added in 1 ms increments, 32 frames = 1 ms.
  m_max_frames_in_flight = ONE_DSP_BUFFER + 32 * std::max(0, SConfig::GetInstance().iLatency - 5);
  m_short_buffer = std::vector<short>(m_max_frames_in_flight, 0);
  m_short_buffer.reserve(m_max_frames_in_flight * 2);

  //NOTICE_LOG(AUDIO, "Max frames in flight: %u", m_max_frames_in_flight);
  //NOTICE_LOG(AUDIO, "Minimum latency from Cubeb in frames: %u", minimum_latency);

  return cubeb_stream_init(m_ctx.get(), &m_stream, "Dolphin Audio Output", nullptr, nullptr,
                           nullptr, &params, m_max_frames_in_flight,
                           DataCallback, StateCallback, this) == CUBEB_OK;
}

bool CubebStream::SetRunning(bool running)
{
  if (running)
    return cubeb_stream_start(m_stream) == CUBEB_OK;
  else
    return cubeb_stream_stop(m_stream) == CUBEB_OK;
}

CubebStream::~CubebStream()
{
  SetRunning(false);
  cubeb_stream_destroy(m_stream);
  m_ctx.reset();
}

void CubebStream::SetVolume(int volume)
{
  cubeb_stream_set_volume(m_stream, volume / 100.0f);
}

void CubebStream::PushSamples(const short* samples, unsigned int num_samples)
{
  if(samples)
  {
    std::lock_guard<std::mutex> guard(m_short_buffer_mutex);
    m_short_buffer.insert(m_short_buffer.end(), &samples[0], &samples[num_samples * 2] /* last is L+R */);

    if(m_short_buffer.size() > m_max_frames_in_flight * 2)
      m_short_buffer.erase(m_short_buffer.begin(), m_short_buffer.end() - m_max_frames_in_flight * 2);
  }
}
