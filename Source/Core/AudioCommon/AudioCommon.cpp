// Copyright 2009 Dolphin Emulator Project
// Licensed under GPLv2+
// Refer to the license.txt file included.

#include "AudioCommon/AudioCommon.h"
#include "AudioCommon/CubebStream.h"
#include "AudioCommon/NullSoundStream.h"
#include "AudioCommon/WASAPIStream.h"
#include "Common/Common.h"
#include "Common/FileUtil.h"
#include "Common/Logging/Log.h"
#include "Core/ConfigManager.h"

// This shouldn't be a global, at least not here.
std::unique_ptr<SoundStream> g_sound_stream;

namespace AudioCommon
{
static bool s_audio_dump_start = false;
static bool s_sound_stream_running = false;

constexpr int AUDIO_VOLUME_MIN = 0;
constexpr int AUDIO_VOLUME_MAX = 100;

static std::unique_ptr<SoundStream> CreateSoundStreamForBackend(std::string_view backend)
{
  if (backend == BACKEND_CUBEB)
    return std::make_unique<CubebStream>();
  else if (backend == BACKEND_NULLSOUND)
    return std::make_unique<NullSound>();
  else if (backend == BACKEND_WASAPI && WASAPIStream::isValid())
    return std::make_unique<WASAPIStream>();
  return {};
}

void InitSoundStream()
{
  std::string backend = SConfig::GetInstance().sBackend;
  g_sound_stream = CreateSoundStreamForBackend(backend);

  if (!g_sound_stream)
  {
    WARN_LOG(AUDIO, "Unknown backend %s, using %s instead.", backend.c_str(),
             GetDefaultSoundBackend().c_str());
    backend = GetDefaultSoundBackend();
    g_sound_stream = CreateSoundStreamForBackend(GetDefaultSoundBackend());
  }

  if (!g_sound_stream || !g_sound_stream->Init())
  {
    WARN_LOG(AUDIO, "Could not initialize backend %s, using %s instead.", backend.c_str(),
             BACKEND_NULLSOUND);
    g_sound_stream = std::make_unique<NullSound>();
    g_sound_stream->Init();
  }

  UpdateSoundStream();
  SetSoundStreamRunning(true);

  if (SConfig::GetInstance().m_DumpAudio && !s_audio_dump_start)
    StartAudioDump();
}

void ShutdownSoundStream()
{
  INFO_LOG(AUDIO, "Shutting down sound stream");

  if (SConfig::GetInstance().m_DumpAudio && s_audio_dump_start)
    StopAudioDump();

  SetSoundStreamRunning(false);
  g_sound_stream.reset();

  INFO_LOG(AUDIO, "Done shutting down sound stream");
}

std::string GetDefaultSoundBackend()
{
  std::string backend = BACKEND_CUBEB;
  return backend;
}

DPL2Quality GetDefaultDPL2Quality()
{
  return DPL2Quality::High;
}

std::vector<std::string> GetSoundBackends()
{
  std::vector<std::string> backends;

  backends.emplace_back(BACKEND_NULLSOUND);
  backends.emplace_back(BACKEND_CUBEB);
  if (WASAPIStream::isValid())
    backends.emplace_back(BACKEND_WASAPI);

  return backends;
}

bool SupportsDPL2Decoder(std::string_view backend)
{
  return false;
}

bool SupportsLatencyControl(std::string_view backend)
{
  return backend == BACKEND_CUBEB || backend == BACKEND_WASAPI;
}

bool SupportsVolumeChanges(std::string_view backend)
{
  // FIXME: this one should ask the backend whether it supports it.
  //       but getting the backend from string etc. is probably
  //       too much just to enable/disable a stupid slider...
  return backend == BACKEND_CUBEB || backend == BACKEND_WASAPI;
}

void UpdateSoundStream()
{
  if (g_sound_stream)
  {
    int volume = SConfig::GetInstance().m_IsMuted ? 0 : SConfig::GetInstance().m_Volume;
    g_sound_stream->SetVolume(volume);
  }
}

void SetSoundStreamRunning(bool running)
{
  if (!g_sound_stream)
    return;

  if (s_sound_stream_running == running)
    return;
  s_sound_stream_running = running;

  if (g_sound_stream->SetRunning(running))
    return;
  if (running)
    ERROR_LOG(AUDIO, "Error starting stream.");
  else
    ERROR_LOG(AUDIO, "Error stopping stream.");
}

void SendAIBuffer(const short* samples, unsigned int num_samples)
{
  if (!g_sound_stream)
    return;

  if (SConfig::GetInstance().m_DumpAudio && !s_audio_dump_start)
    StartAudioDump();
  else if (!SConfig::GetInstance().m_DumpAudio && s_audio_dump_start)
    StopAudioDump();

  g_sound_stream->PushSamples(samples, num_samples);
}

void StartAudioDump()
{
  std::string audio_file_name_dtk = File::GetUserPath(D_DUMPAUDIO_IDX) + "dtkdump.wav";
  std::string audio_file_name_dsp = File::GetUserPath(D_DUMPAUDIO_IDX) + "dspdump.wav";
  File::CreateFullPath(audio_file_name_dtk);
  File::CreateFullPath(audio_file_name_dsp);
  s_audio_dump_start = true;
}

void StopAudioDump()
{
  if (!g_sound_stream)
    return;
  s_audio_dump_start = false;
}

void IncreaseVolume(unsigned short offset)
{
  SConfig::GetInstance().m_IsMuted = false;
  int& currentVolume = SConfig::GetInstance().m_Volume;
  currentVolume += offset;
  if (currentVolume > AUDIO_VOLUME_MAX)
    currentVolume = AUDIO_VOLUME_MAX;
  UpdateSoundStream();
}

void DecreaseVolume(unsigned short offset)
{
  SConfig::GetInstance().m_IsMuted = false;
  int& currentVolume = SConfig::GetInstance().m_Volume;
  currentVolume -= offset;
  if (currentVolume < AUDIO_VOLUME_MIN)
    currentVolume = AUDIO_VOLUME_MIN;
  UpdateSoundStream();
}

void ToggleMuteVolume()
{
  bool& isMuted = SConfig::GetInstance().m_IsMuted;
  isMuted = !isMuted;
  UpdateSoundStream();
}
}  // namespace AudioCommon
