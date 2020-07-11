// Copyright 2018 Dolphin Emulator Project
// Licensed under GPLv2+
// Refer to the license.txt file included.

#include "AudioCommon/WASAPIStream.h"

#ifdef _WIN32

// clang-format off
#include <Audioclient.h>
#include <comdef.h>
#include <mmdeviceapi.h>
#include <devpkey.h>
#include <functiondiscoverykeys_devpkey.h>
#include <thread>
// clang-format on

#include "Common/Logging/Log.h"
#include "Common/StringUtil.h"
#include "Common/Thread.h"
#include "common/Swap.h"
#include "Core/ConfigManager.h"
#include "VideoCommon/OnScreenDisplay.h"

constexpr u32 ONE_DSP_BUFFER = 160;

WASAPIStream::WASAPIStream()
{
  CoInitialize(nullptr);

  m_format.Format.wFormatTag = WAVE_FORMAT_EXTENSIBLE;
  m_format.Format.nChannels = 2;
  m_format.Format.nSamplesPerSec = 32000;
  m_format.Format.nAvgBytesPerSec = m_format.Format.nSamplesPerSec * 4;
  m_format.Format.nBlockAlign = 4;
  m_format.Format.wBitsPerSample = 16;
  m_format.Format.cbSize = sizeof(WAVEFORMATEXTENSIBLE) - sizeof(WAVEFORMATEX);
  m_format.Samples.wValidBitsPerSample = m_format.Format.wBitsPerSample;
  m_format.dwChannelMask = SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT;
  m_format.SubFormat = KSDATAFORMAT_SUBTYPE_PCM;
}

WASAPIStream::~WASAPIStream()
{
  if (m_enumerator)
    m_enumerator->Release();

  if (m_need_data_event)
    CloseHandle(m_need_data_event);

  if (m_running)
  {
    m_running = false;
    if (m_thread.joinable())
      m_thread.join();
  }

  CoUninitialize();
}

bool WASAPIStream::isValid()
{
  return true;
}

static bool HandleWinAPI(std::string message, HRESULT result)
{
  if (result != S_OK)
  {
    _com_error err(result);
    std::string error = TStrToUTF8(err.ErrorMessage()).c_str();

    switch (result)
    {
    case AUDCLNT_E_DEVICE_IN_USE:
      error = "Audio endpoint already in use!";
      break;
    }

    ERROR_LOG(AUDIO, "WASAPI: %s: %s", message.c_str(), error.c_str());
  }

  return result == S_OK;
}

std::vector<std::string> WASAPIStream::GetAvailableDevices()
{
  CoInitialize(nullptr);

  IMMDeviceEnumerator* enumerator = nullptr;

  HRESULT result =
      CoCreateInstance(__uuidof(MMDeviceEnumerator), nullptr, CLSCTX_INPROC_SERVER,
                       __uuidof(IMMDeviceEnumerator), reinterpret_cast<LPVOID*>(&enumerator));

  if (!HandleWinAPI("Failed to create MMDeviceEnumerator", result))
    return {};

  std::vector<std::string> device_names;
  IMMDeviceCollection* devices;
  result = enumerator->EnumAudioEndpoints(eRender, DEVICE_STATE_ACTIVE, &devices);

  if (!HandleWinAPI("Failed to get available devices", result))
  {
    CoUninitialize();
    return {};
  }

  UINT count;
  devices->GetCount(&count);

  for (u32 i = 0; i < count; i++)
  {
    IMMDevice* device;
    devices->Item(i, &device);
    if (!HandleWinAPI("Failed to get device " + std::to_string(i), result))
      continue;

    LPWSTR device_id;
    device->GetId(&device_id);

    IPropertyStore* device_properties;

    result = device->OpenPropertyStore(STGM_READ, &device_properties);

    if (!HandleWinAPI("Failed to initialize IPropertyStore", result))
      continue;

    PROPVARIANT device_name;
    PropVariantInit(&device_name);

    device_properties->GetValue(PKEY_Device_FriendlyName, &device_name);

    device_names.push_back(TStrToUTF8(device_name.pwszVal));

    PropVariantClear(&device_name);
  }

  devices->Release();
  enumerator->Release();

  CoUninitialize();

  return device_names;
}

IMMDevice* WASAPIStream::GetDeviceByName(std::string name)
{
  CoInitialize(nullptr);

  IMMDeviceEnumerator* enumerator = nullptr;

  HRESULT result =
      CoCreateInstance(__uuidof(MMDeviceEnumerator), nullptr, CLSCTX_INPROC_SERVER,
                       __uuidof(IMMDeviceEnumerator), reinterpret_cast<LPVOID*>(&enumerator));

  if (!HandleWinAPI("Failed to create MMDeviceEnumerator", result))
    return nullptr;

  IMMDeviceCollection* devices;
  result = enumerator->EnumAudioEndpoints(eRender, DEVICE_STATE_ACTIVE, &devices);

  if (!HandleWinAPI("Failed to get available devices", result))
  {
    return {};
  }

  UINT count;
  devices->GetCount(&count);

  for (u32 i = 0; i < count; i++)
  {
    IMMDevice* device;
    devices->Item(i, &device);
    if (!HandleWinAPI("Failed to get device " + std::to_string(i), result))
      continue;

    LPWSTR device_id;
    device->GetId(&device_id);

    IPropertyStore* device_properties;

    result = device->OpenPropertyStore(STGM_READ, &device_properties);

    if (!HandleWinAPI("Failed to initialize IPropertyStore", result))
      continue;

    PROPVARIANT device_name;
    PropVariantInit(&device_name);

    device_properties->GetValue(PKEY_Device_FriendlyName, &device_name);

    if (TStrToUTF8(device_name.pwszVal) == name)
    {
      devices->Release();
      enumerator->Release();
      CoUninitialize();
      return device;
    }

    PropVariantClear(&device_name);
  }

  devices->Release();
  enumerator->Release();
  CoUninitialize();

  return nullptr;
}

bool WASAPIStream::Init()
{
  HRESULT result =
      CoCreateInstance(__uuidof(MMDeviceEnumerator), nullptr, CLSCTX_INPROC_SERVER,
                       __uuidof(IMMDeviceEnumerator), reinterpret_cast<LPVOID*>(&m_enumerator));

  if (!HandleWinAPI("Failed to create MMDeviceEnumerator", result))
    return false;

  return true;
}

bool WASAPIStream::SetRunning(bool running)
{
  if (running)
  {
    IMMDevice* device = nullptr;

    HRESULT result;

    if (SConfig::GetInstance().sWASAPIDevice == "default")
    {
      result = m_enumerator->GetDefaultAudioEndpoint(eRender, eConsole, &device);
    }
    else
    {
      result = S_OK;
      device = GetDeviceByName(SConfig::GetInstance().sWASAPIDevice);

      if (!device)
      {
        ERROR_LOG(AUDIO, "Can't find device '%s', falling back to default",
                  SConfig::GetInstance().sWASAPIDevice.c_str());
        result = m_enumerator->GetDefaultAudioEndpoint(eRender, eConsole, &device);
      }
    }

    if (!HandleWinAPI("Failed to obtain default endpoint", result))
      return false;

    LPWSTR device_id;
    device->GetId(&device_id);

    // Show a friendly name in the log
    IPropertyStore* device_properties;

    result = device->OpenPropertyStore(STGM_READ, &device_properties);

    if (!HandleWinAPI("Failed to initialize IPropertyStore", result))
      return false;

    PROPVARIANT device_name;
    PropVariantInit(&device_name);

    device_properties->GetValue(PKEY_Device_FriendlyName, &device_name);

    INFO_LOG(AUDIO, "Using audio endpoint '%s'", TStrToUTF8(device_name.pwszVal).c_str());

    PropVariantClear(&device_name);

    // Get IAudioDevice
    result = device->Activate(__uuidof(IAudioClient), CLSCTX_INPROC_SERVER, nullptr,
                              reinterpret_cast<LPVOID*>(&m_audio_client));

    if (!HandleWinAPI("Failed to activate IAudioClient", result))
    {
      device->Release();
      return false;
    }

    REFERENCE_TIME device_period = 0;

    result = m_audio_client->GetDevicePeriod(nullptr, &device_period);

    device_period += std::max(0, SConfig::GetInstance().iLatency - 5) * (10000 / m_format.Format.nChannels);
    INFO_LOG(AUDIO, "Audio period set to %d", device_period);

    if (!HandleWinAPI("Failed to obtain device period", result))
    {
      device->Release();
      m_audio_client->Release();
      m_audio_client = nullptr;
      return false;
    }

    result = m_audio_client->Initialize(
        AUDCLNT_SHAREMODE_EXCLUSIVE,
        AUDCLNT_STREAMFLAGS_EVENTCALLBACK | AUDCLNT_STREAMFLAGS_NOPERSIST, device_period,
        device_period, reinterpret_cast<WAVEFORMATEX*>(&m_format), nullptr);

    if (result == AUDCLNT_E_UNSUPPORTED_FORMAT)
    {
      OSD::AddMessage("Your current audio device doesn't support 16-bit 32000 hz PCM audio. WASAPI "
                      "exclusive mode won't work.",
                      6000U);
      return false;
    }

    if (result == AUDCLNT_E_BUFFER_SIZE_NOT_ALIGNED)
    {
      result = m_audio_client->GetBufferSize(&m_requested_frames);
      m_audio_client->Release();

      if (!HandleWinAPI("Failed to get aligned buffer size", result))
      {
        device->Release();
        m_audio_client->Release();
        m_audio_client = nullptr;
        return false;
      }

      // Get IAudioDevice
      result = device->Activate(__uuidof(IAudioClient), CLSCTX_INPROC_SERVER, nullptr,
                                reinterpret_cast<LPVOID*>(&m_audio_client));

      if (!HandleWinAPI("Failed to reactivate IAudioClient", result))
      {
        device->Release();
        return false;
      }

      device_period =
          static_cast<REFERENCE_TIME>(
              10000.0 * 1000 * m_requested_frames / m_format.Format.nSamplesPerSec + 0.5) +
          SConfig::GetInstance().iLatency * 10000;

      result = m_audio_client->Initialize(
          AUDCLNT_SHAREMODE_EXCLUSIVE,
          AUDCLNT_STREAMFLAGS_EVENTCALLBACK | AUDCLNT_STREAMFLAGS_NOPERSIST, device_period,
          device_period, reinterpret_cast<WAVEFORMATEX*>(&m_format), nullptr);
    }

    if (!HandleWinAPI("Failed to initialize IAudioClient", result))
    {
      device->Release();
      m_audio_client->Release();
      m_audio_client = nullptr;
      return false;
    }

    result = m_audio_client->GetBufferSize(&m_requested_frames);

    if (!HandleWinAPI("Failed to get buffer size from IAudioClient", result))
    {
      device->Release();
      m_audio_client->Release();
      m_audio_client = nullptr;
      return false;
    }

    result = m_audio_client->GetService(__uuidof(IAudioRenderClient),
                                        reinterpret_cast<LPVOID*>(&m_audio_renderer));

    if (!HandleWinAPI("Failed to get IAudioRenderClient from IAudioClient", result))
    {
      device->Release();
      m_audio_client->Release();
      m_audio_client = nullptr;
      return false;
    }

    m_need_data_event = CreateEvent(nullptr, FALSE, FALSE, nullptr);
    m_audio_client->SetEventHandle(m_need_data_event);

    result = m_audio_client->Start();

    if (!HandleWinAPI("Failed to get IAudioRenderClient from IAudioClient", result))
    {
      device->Release();
      m_audio_renderer->Release();
      m_audio_renderer = nullptr;
      m_audio_client->Release();
      m_audio_client = nullptr;
      CloseHandle(m_need_data_event);
      return false;
    }

    device->Release();

    m_max_frames_in_flight = std::max(m_requested_frames, ONE_DSP_BUFFER * 2);
    m_short_buffer = std::vector<short>(m_max_frames_in_flight, 0);
    m_short_buffer.reserve(m_max_frames_in_flight * 2);

    INFO_LOG(AUDIO, "WASAPI: Successfully initialized!");

    m_running = true;
    m_thread = std::thread([this] { SoundLoop(); });
    m_thread.detach();
  }
  else
  {
    m_running = false;

    if (m_thread.joinable())
      m_thread.join();

    while (!m_stopped)
    {
    }

    if (m_audio_client)
    {
      m_audio_renderer->Release();
      m_audio_renderer = nullptr;

      m_audio_client->Release();
      m_audio_client = nullptr;
    }
  }

  return true;
}

void WASAPIStream::SoundLoop()
{
  Common::SetCurrentThreadName("WASAPI Handler");
  BYTE* data;

  if (m_audio_renderer)
  {
    m_audio_renderer->GetBuffer(m_requested_frames, &data);
    m_audio_renderer->ReleaseBuffer(m_requested_frames, AUDCLNT_BUFFERFLAGS_SILENT);
  }

  m_stopped = false;

  while (m_running)
  {
    if (!m_audio_renderer)
      continue;

    WaitForSingleObject(m_need_data_event, 1000);

    m_audio_renderer->GetBuffer(m_requested_frames, &data);

    float volume = SConfig::GetInstance().m_IsMuted ? 0 : SConfig::GetInstance().m_Volume / 100.0;

    {
      std::lock_guard<std::mutex> guard(m_short_buffer_mutex);

      if (m_short_buffer.size() < 2)
      {
        m_short_buffer.push_back(0);  // need at least one frame/two samples
        m_short_buffer.push_back(0);
      }

      for (std::size_t sample = 0; sample < m_requested_frames * 2; sample += 2)
      {
        reinterpret_cast<short*>(data)[sample] = static_cast<s16>(Common::swap16(
            sample >= m_short_buffer.size() ? m_short_buffer[m_short_buffer.size() - 2] :
                                              m_short_buffer[sample]) * volume);
        reinterpret_cast<short*>(data)[sample + 1] = static_cast<s16>(Common::swap16(
            sample + 1 >= m_short_buffer.size() ? m_short_buffer[m_short_buffer.size() - 1] :
                                                m_short_buffer[sample + 1]) * volume);
      }

      if (m_short_buffer.size() < m_requested_frames * 2)
      {
        // NOTICE_LOG(AUDIO, "Underflow from our side by %lu frames", num_frames -
        // (m_short_buffer.size() / 2));
        m_short_buffer.erase(m_short_buffer.begin(), m_short_buffer.end());
      }
      else
        m_short_buffer.erase(m_short_buffer.begin(),
                             m_short_buffer.begin() + m_requested_frames * 2);
    }

    m_audio_renderer->ReleaseBuffer(m_requested_frames, 0);
  }

  m_stopped = true;
}

void WASAPIStream::PushSamples(const short* samples, unsigned int num_samples)
{
  if (samples)
  {
    std::lock_guard<std::mutex> guard(m_short_buffer_mutex);
    m_short_buffer.insert(m_short_buffer.end(), &samples[0],
                          &samples[num_samples * 2] /* last is L+R */);

    if (m_short_buffer.size() > m_max_frames_in_flight * 2)
      m_short_buffer.erase(m_short_buffer.begin(),
                           m_short_buffer.end() - m_max_frames_in_flight * 2);
  }
}

#endif  // _WIN32
