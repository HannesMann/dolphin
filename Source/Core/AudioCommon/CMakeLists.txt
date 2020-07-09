add_library(audiocommon
  AudioCommon.cpp
  AudioCommon.h
  AudioStretcher.cpp
  AudioStretcher.h
  CubebStream.cpp
  CubebStream.h
  CubebUtils.cpp
  CubebUtils.h
  Enums.h
  SurroundDecoder.cpp
  SurroundDecoder.h
  NullSoundStream.cpp
  NullSoundStream.h
  WaveFile.cpp
  WaveFile.h
)

if(WIN32)
  target_sources(audiocommon PRIVATE
    WASAPIStream.cpp
    WASAPIStream.h
  )
endif()

target_link_libraries(audiocommon PRIVATE cubeb SoundTouch FreeSurround)
