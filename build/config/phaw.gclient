# gclient solution for the phaw base checkout.
# Consumed by build/fetch-base.ps1:  gclient sync --gclientfile phaw.gclient
#
# Supermium is a FULL Chromium source fork with its own in-tree DEPS (not a patch
# set over upstream), so `src` points directly at Supermium's repo; `gclient sync`
# then reads that DEPS to pull the third-party dependencies — same flow as
# checking out chromium/src. Pin `url` to a specific tag/commit (NOT a moving
# branch) so builds are reproducible; Phase 0 records the chosen tag.
# Confirm exact bootstrap against Supermium's docs/ before first sync.
#
#   PIN (fill in during Phase 0 Spike A):
#     SUPERMIUM_TAG = "vX.Y.ZZZZ.NNN"   # e.g. the ~M144 release you validated
#
solutions = [
  {
    "name": "src",
    # Supermium tracks Chromium's layout; check out its src at the pinned tag.
    "url": "https://github.com/win32ss/supermium.git@REPLACE_WITH_SUPERMIUM_TAG",
    "managed": False,
    "custom_deps": {},
    "custom_vars": {
      # Windows target: pull the Win toolchain deps.
      "checkout_win": True,
    },
  },
]
target_os = ["win"]
target_cpu = ["x86", "x64"]
