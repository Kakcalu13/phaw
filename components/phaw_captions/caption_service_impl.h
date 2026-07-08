// Copyright 2026 The phaw Authors. All rights reserved.
// Use of this source code is governed by the license described in NOTICES.md.

#ifndef COMPONENTS_PHAW_CAPTIONS_CAPTION_SERVICE_IMPL_H_
#define COMPONENTS_PHAW_CAPTIONS_CAPTION_SERVICE_IMPL_H_

#include <memory>
#include <string>

#include "base/memory/raw_ptr.h"
#include "base/memory/weak_ptr.h"
#include "base/observer_list.h"
#include "components/phaw_captions/public/caption_service.h"

class PrefRegistrySimple;
class PrefService;

namespace phaw {

class CaptionBackend;

// Concrete CaptionService (req 1). A small state machine over the backend:
// unavailable -> downloading -> {ready | unsupported-on-os}. Coordinates only;
// SODA specifics live in the backend, the Win8.1 load check in SodaCompatProbe.
class CaptionServiceImpl : public CaptionService {
 public:
  CaptionServiceImpl(PrefService* prefs,
                     std::unique_ptr<CaptionBackend> backend);
  ~CaptionServiceImpl() override;

  static void RegisterProfilePrefs(PrefRegistrySimple* registry);

  // CaptionService:
  void AddObserver(Observer* observer) override;
  void RemoveObserver(Observer* observer) override;
  Status status() const override;
  void SetEnabled(bool enabled) override;
  void EnsureModel(const std::string& lang) override;

 private:
  void SetStatus(Status status);
  void OnBackendReady(bool ready);

  const raw_ptr<PrefService> prefs_;
  const std::unique_ptr<CaptionBackend> backend_;
  Status status_ = Status::kUnavailable;
  bool enabled_ = false;
  base::ObserverList<Observer> observers_;

  base::WeakPtrFactory<CaptionServiceImpl> weak_factory_{this};
};

}  // namespace phaw

#endif  // COMPONENTS_PHAW_CAPTIONS_CAPTION_SERVICE_IMPL_H_
