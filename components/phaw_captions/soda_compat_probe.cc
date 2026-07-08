// Copyright 2026 The phaw Authors. All rights reserved.
// Use of this source code is governed by the license described in NOTICES.md.

#include "components/phaw_captions/soda_compat_probe.h"

#include "base/files/file_path.h"
#include "base/native_library.h"

namespace phaw {

bool SodaCompatProbe::ProbeLoad(const base::FilePath& soda_library) const {
  // Load in a guarded way: a missing-import failure on Win8.1 surfaces as a load
  // error here rather than a crash later.
  base::NativeLibraryLoadError error;
  base::NativeLibrary handle = base::LoadNativeLibrary(soda_library, &error);
  if (!handle) {
    return false;
  }
  // confirm-in-tree: resolve a known SODA export (e.g. "CreateSodaAsync") to
  // confirm the ABI is usable, not just that the DLL mapped.
  base::UnloadNativeLibrary(handle);
  return true;
}

// static
CaptionService::Status SodaCompatProbe::ClassifyLoadResult(bool loaded) {
  return loaded ? CaptionService::Status::kReady
                : CaptionService::Status::kUnsupportedOnOS;
}

}  // namespace phaw
