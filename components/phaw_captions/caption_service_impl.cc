// Copyright 2026 The phaw Authors. All rights reserved.
// Use of this source code is governed by the license described in NOTICES.md.

#include "components/phaw_captions/caption_service_impl.h"

#include <utility>

#include "base/functional/bind.h"
#include "components/phaw_captions/caption_backend.h"
#include "components/prefs/pref_registry_simple.h"
#include "components/prefs/pref_service.h"

namespace phaw {
namespace {
constexpr char kEnabledPref[] = "phaw.captions.enabled";
constexpr char kDefaultLanguage[] = "en-US";
}  // namespace

// static
void CaptionServiceImpl::RegisterProfilePrefs(PrefRegistrySimple* registry) {
  registry->RegisterBooleanPref(kEnabledPref, /*default_value=*/false);
}

CaptionServiceImpl::CaptionServiceImpl(PrefService* prefs,
                                       std::unique_ptr<CaptionBackend> backend)
    : prefs_(prefs),
      backend_(std::move(backend)),
      enabled_(prefs->GetBoolean(kEnabledPref)) {
  if (enabled_) {
    EnsureModel(kDefaultLanguage);
  }
}

CaptionServiceImpl::~CaptionServiceImpl() = default;

void CaptionServiceImpl::AddObserver(Observer* observer) {
  observers_.AddObserver(observer);
}

void CaptionServiceImpl::RemoveObserver(Observer* observer) {
  observers_.RemoveObserver(observer);
}

CaptionService::Status CaptionServiceImpl::status() const {
  return status_;
}

void CaptionServiceImpl::SetEnabled(bool enabled) {
  if (enabled_ == enabled) {
    return;
  }
  enabled_ = enabled;
  prefs_->SetBoolean(kEnabledPref, enabled);

  if (!enabled_) {
    backend_->Stop();
    return;
  }
  if (status_ == Status::kReady) {
    backend_->Start();
  } else if (status_ != Status::kUnsupportedOnOS) {
    EnsureModel(kDefaultLanguage);
  }
}

void CaptionServiceImpl::EnsureModel(const std::string& lang) {
  if (status_ == Status::kDownloading) {
    return;
  }
  SetStatus(Status::kDownloading);
  backend_->Install(lang, base::BindOnce(&CaptionServiceImpl::OnBackendReady,
                                         weak_factory_.GetWeakPtr()));
}

void CaptionServiceImpl::OnBackendReady(bool ready) {
  SetStatus(ready ? Status::kReady : Status::kUnsupportedOnOS);
  if (ready && enabled_) {
    backend_->Start();
  }
}

void CaptionServiceImpl::SetStatus(Status status) {
  if (status_ == status) {
    return;
  }
  status_ = status;
  for (Observer& observer : observers_) {
    observer.OnStatusChanged(status_);
  }
}

}  // namespace phaw
