// Copyright 2026 The phaw Authors. All rights reserved.
// Use of this source code is governed by the license described in NOTICES.md.

#ifndef COMPONENTS_PHAW_CAPTIONS_SODA_COMPAT_PROBE_H_
#define COMPONENTS_PHAW_CAPTIONS_SODA_COMPAT_PROBE_H_

#include "components/phaw_captions/public/caption_service.h"

namespace base {
class FilePath;
}  // namespace base

namespace phaw {

// The Win8.1 safety valve for req 1. SODA is a Google-built native binary that
// may depend on APIs the backported Win8.1 base lacks, so rather than assume, we
// empirically load it and report whether it works — degrading gracefully instead
// of crashing.
class SodaCompatProbe {
 public:
  // Attempts to load |soda_library| and resolve its entry point. Returns true
  // iff it loads — i.e. captions can run on this OS.
  bool ProbeLoad(const base::FilePath& soda_library) const;

  // Maps a probe result to a service status. Pure and unit-testable.
  static CaptionService::Status ClassifyLoadResult(bool loaded);
};

}  // namespace phaw

#endif  // COMPONENTS_PHAW_CAPTIONS_SODA_COMPAT_PROBE_H_
