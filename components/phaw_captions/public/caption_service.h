// Copyright 2026 The phaw Authors. All rights reserved.
// Use of this source code is governed by the license described in NOTICES.md.

#ifndef COMPONENTS_PHAW_CAPTIONS_PUBLIC_CAPTION_SERVICE_H_
#define COMPONENTS_PHAW_CAPTIONS_PUBLIC_CAPTION_SERVICE_H_

#include <string>

#include "base/observer_list_types.h"
#include "components/keyed_service/core/keyed_service.h"

namespace phaw {

// Enables Live Caption (Chromium's on-device SODA speech) and guards against
// the case where the SODA binary will not load on the backported Win8.1 base,
// degrading gracefully instead of crashing (req 1).
class CaptionService : public KeyedService {
 public:
  enum class Status {
    kUnavailable,      // not set up / disabled
    kDownloading,      // fetching the SODA binary + language pack
    kReady,            // captions available
    kUnsupportedOnOS,  // SODA cannot load here (e.g. Win8.1) — degrade gracefully
  };

  class Observer : public base::CheckedObserver {
   public:
    virtual void OnStatusChanged(Status status) {}

   protected:
    ~Observer() override = default;
  };

  CaptionService(const CaptionService&) = delete;
  CaptionService& operator=(const CaptionService&) = delete;
  ~CaptionService() override = default;

  virtual void AddObserver(Observer* observer) = 0;
  virtual void RemoveObserver(Observer* observer) = 0;

  virtual Status status() const = 0;

  // User toggle (req 1).
  virtual void SetEnabled(bool enabled) = 0;

  // Ensure the SODA model and |lang| language pack are installed, downloading if
  // needed. Runs the Win8.1 compatibility probe; may finish in kUnsupportedOnOS.
  virtual void EnsureModel(const std::string& lang) = 0;

 protected:
  CaptionService() = default;
};

}  // namespace phaw

#endif  // COMPONENTS_PHAW_CAPTIONS_PUBLIC_CAPTION_SERVICE_H_
