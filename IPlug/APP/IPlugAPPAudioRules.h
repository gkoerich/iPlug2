#pragma once

/**
 * @file Pure audio-settings rules for IPlugAPPHost.
 *
 * STL only: no RtAudio, no WDL, no platform header. IPlugAPPHost holds the device state and calls
 * these; a test can exercise them without an audio API present. Keeping them here rather than as
 * private statics in IPlugAPP_host.cpp is what lets an app-layer caller reach the same rule the
 * host applies internally, instead of reimplementing it and drifting.
 */

#include "IPlugPlatform.h"

#include <algorithm>
#include <cstdint>
#include <string>
#include <vector>

BEGIN_IPLUG_NAMESPACE

/** The rate to open a device pair at.
 * @param desired The rate currently stored in the app's preferences
 * @param supported The rates both devices of the pair offer, as an intersection
 * @return `desired` when the pair offers it, otherwise 48000, otherwise 44100, otherwise the first
 * rate offered. A stream opened at an unsupported rate simply fails, so a device change snaps to
 * this. An empty `supported` means nothing was probed, and `desired` passes through untouched --
 * there is no better answer available, and substituting one would override a valid stored choice. */
inline uint32_t PickSupportedSampleRate(uint32_t desired, const std::vector<uint32_t>& supported)
{
  auto offers = [&supported](uint32_t rate) {
    return std::find(supported.begin(), supported.end(), rate) != supported.end();
  };

  if (supported.empty() || offers(desired))
    return desired;

  return offers(48000) ? 48000 : (offers(44100) ? 44100 : supported.front());
}

/** The device the input stream actually opens on.
 * @param driverIsAsio Whether the selected driver is Windows ASIO
 * @param inputDeviceName The raw mAudioInDev entry
 * @param outputDeviceName The raw mAudioOutDev entry
 * @return An ASIO driver serves both directions from one device, which the host keeps in
 * mAudioOutDev alone -- mAudioInDev keeps whatever the last non-ASIO driver left there, since the
 * preferences dialog disables its combo. Reading the raw entry under ASIO therefore names a device
 * that is not the one recording. This returns the output device verbatim in that case, empty
 * string included: falling back to the stale input name would report a device change that never
 * happened, and the standalone's watcher answers that by clearing the stored calibration. */
inline std::string EffectiveInputDeviceName(bool driverIsAsio, const char* inputDeviceName,
                                            const char* outputDeviceName)
{
  if (driverIsAsio)
    return outputDeviceName ? outputDeviceName : "";

  return inputDeviceName ? inputDeviceName : "";
}

END_IPLUG_NAMESPACE
