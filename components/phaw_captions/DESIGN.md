# phaw_captions — Live Caption enablement + Win8.1 probe (req 1)

Live Caption already exists in Chromium (on-device speech via the **SODA**
component). This component's job is mostly **enablement and Win8.1 safety**: turn
the feature on by default, ensure the SODA binary is installed, and guard against
the case where the SODA binary won't load on the backported 8.1 base.

We reuse Chromium's caption pipeline and bubble UI; we own the enablement policy
and the compatibility probe.

## Public interface

```cpp
// public/caption_service.h
class CaptionService : public KeyedService {
 public:
  enum class Status { kUnavailable, kDownloading, kReady, kUnsupportedOnOS };
  class Observer : public base::CheckedObserver {
   public:
    virtual void OnStatusChanged(Status) {}
  };
  virtual void AddObserver(Observer*) = 0;
  virtual void RemoveObserver(Observer*) = 0;
  virtual Status status() const = 0;
  virtual void SetEnabled(bool) = 0;                 // user toggle (req 1)
  virtual void EnsureModel(const std::string& lang) = 0;  // fetch SODA + pack
};
```

## Class model

| Class | Responsibility |
|-------|----------------|
| `CaptionServiceImpl` | Enablement policy, status, observers |
| `ICaptionBackend` (iface) → `SodaCaptionBackend` | Wraps `speech::SodaInstaller` + `media::LiveCaptionController` |
| `SodaCompatProbe` | Checks the SODA binary actually loads on this OS (Win8.1 guard) |
| `LanguagePackManager` | Requests SODA language packs via the component updater |

```
CaptionServiceImpl
   ├─ SodaCompatProbe.Check() ── fail on Win8.1? ─▶ Status::kUnsupportedOnOS (graceful)
   ├─ ICaptionBackend.Install(lang) ──▶ SodaInstaller (component updater)
   ├─ ICaptionBackend.Start() ──▶ media::LiveCaptionController ──▶ caption bubble UI (reused)
   └─ Observer.OnStatusChanged(...)
```

## Win8.1 compatibility probe (the risk-managed bit)

SODA ships as a Google-built native binary. On the backported 8.1 base it may
depend on APIs 8.1 lacks. `SodaCompatProbe`:

1. After install, loads the SODA library in a **guarded** way.
2. On load failure / missing-import, reports `kUnsupportedOnOS` instead of
   crashing, and the UI shows "captions need Windows 10+."
3. Records the failure so we can prioritize a backport patch if needed.

This keeps req 1 honest: captions work where SODA loads; where it can't, phaw
degrades gracefully rather than breaking. (This is exactly the Phase 5 unknown
called out in PHASES.md.)

## Seams

- Enable the `media::kLiveCaption` feature via `args.gn` +
  `phaw_features.gni` (patches/).
- `SodaCaptionBackend` uses `speech::SodaInstaller` and
  `media::LiveCaptionController` — public Chromium types; no pipeline fork.
- Caption bubble: reuse Chromium's `CaptionBubbleController`.

## Dependencies (DEPS)

Allowed: `//base`, `//components/live_caption`, `//components/soda`,
`//media`, `//components/phaw_common`.
Forbidden: other `//components/phaw_*`.

## Tests

- Unit: `CaptionServiceImpl` state machine (unavailable→downloading→ready /
  →unsupported) with a fake backend.
- Unit: `SodaCompatProbe` maps a load failure to `kUnsupportedOnOS`.
- Manual/VM: real captions on a Win8.1 VM playing a video; verify graceful
  degradation if SODA won't load.
