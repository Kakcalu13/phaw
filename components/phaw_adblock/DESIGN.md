# phaw_adblock — ad & anti-adblock (reqs 2, 3)

Drives the `adblock-rust` engine to block ad/tracker **requests** (network) and
hide ad **elements** (cosmetic), plus anti-adblock scriptlets. This component
owns the integration glue; the matching algorithm lives in the Rust library.

## Public interface (the only header other layers include)

```cpp
// public/adblock_service.h
class AdblockService : public KeyedService {
 public:
  class Observer : public base::CheckedObserver {
   public:
    virtual void OnResourceBlocked(const GURL& url, int tab_id) {}
    virtual void OnCountersChanged(int session_total) {}
  };
  virtual void AddObserver(Observer*) = 0;
  virtual void RemoveObserver(Observer*) = 0;
  virtual bool IsEnabled() const = 0;
  virtual void SetEnabled(bool) = 0;                 // global toggle (req 2)
  virtual void AllowSite(const url::Origin&) = 0;    // per-site allow
};
```

Callers (UI, throttle factory) hold `AdblockService*`. The impl and every
collaborator below are private to this component.

## Class model (single responsibility each; service only coordinates)

| Class | Responsibility | Owns / thread |
|-------|----------------|---------------|
| `AdblockServiceImpl` | Coordinates; holds config, observer list, hands the engine to throttles | UI thread; owns engine + store |
| `IAdblockEngine` (iface) → `RustAdblockEngine` | `ShouldBlock(url, ctx)` + cosmetic selectors; wraps `adblock-rust-ffi` | **engine sequence** (off UI) |
| `AdblockThrottle : blink::URLLoaderThrottle` | Per-request; asks engine, cancels on block, reports to service | IO/worker per request |
| `CosmeticInjector : content::RenderFrameObserver` | Injects element-hiding CSS + anti-adblock scriptlets at document-start (req 3) | renderer |
| `FilterListStore` | Fetches/updates EasyList/EasyPrivacy/anti-adblock lists; rebuilds engine on change | UI thread + net |
| `ScriptletLibrary` | Named scriptlets that neutralize common detectors (req 3) | data, loaded once |

```
AdblockServiceImpl ──owns──▶ IAdblockEngine (RustAdblockEngine ─FFI─▶ adblock-rust)
        │  ▲                         ▲
        │  └── observers (UI) ◀──────┘ block events
        ├──creates──▶ AdblockThrottle (network seam) ──queries──▶ engine
        └──owns──▶ FilterListStore ──rebuild──▶ engine
CosmeticInjector (renderer) ◀──mojo── selectors + scriptlets from service
```

## Seams (per architecture.md §2)

- **Network:** `AdblockThrottle` via a `URLLoaderThrottleProvider` hook in `patches/`.
- **Cosmetic:** `CosmeticInjector` (a `RenderFrameObserver`) gets selectors +
  scriptlets over a **Mojo** interface (`mojom::AdblockRenderer`).
- No engine internals are edited; we only implement Chromium interfaces.

## Threading

FFI/`ShouldBlock` runs on a dedicated **engine sequence**, never the UI thread.
Throttles query via a thread-safe snapshot handle; results marshalled with
`PostTask`. `RustAdblockEngine` documents this affinity; violating it is a bug.

## Anti-adblock (req 3) — how detection is defeated

1. Cosmetic hiding runs at **document-start**, before page detectors execute.
2. `ScriptletLibrary` injects small shims that neutralize common
   blocker-detectors (bait-element checks, timing probes).
3. Anti-adblock filter lists feed the same engine.
   *This is an arms race → `FilterListStore` auto-updates; never "done."*

## Dependencies (DEPS — enforces D1/D2)

Allowed: `//base`, `//content/public/*`, `//services/network/public`,
`//components/phaw_common`, `//third_party/adblock_rust_ffi`.
**Forbidden:** any other `//components/phaw_*` (no feature-to-feature deps).

## BUILD.gn targets

`:public` (interface only) · `:impl` · `:cosmetic_mojom` · `:unit_tests`.

## Tests

- Unit: `RustAdblockEngine.ShouldBlock` matches/□ against a fixture list.
- Unit: `AdblockThrottle` cancels on block, forwards otherwise (fake engine).
- Browser test: ad host blocked, page host allowed; cosmetic hide applied;
  anti-adblock test page loads clean.
