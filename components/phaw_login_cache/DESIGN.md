# phaw_login_cache — login-only cache / storage policy (req 5)

> "Block all cache that accesses my info **except** cache used to log in."

Interpreted precisely and safely: **first-party** cookies/cache/storage (what
keeps *you* logged into the site you're visiting) are allowed; **third-party**
cookies/cache/storage (what trackers use to follow you) are denied or
partitioned. Login keeps working because login state is first-party.

## Public interface

```cpp
// public/storage_policy_service.h
class StoragePolicyService : public KeyedService {
 public:
  enum class Decision { kAllow, kBlock, kPartitionEphemeral };
  class Observer : public base::CheckedObserver {
   public:
    virtual void OnThirdPartyBlocked(const url::Origin& tracker,
                                     const url::Origin& top_frame) {}
  };
  virtual void AddObserver(Observer*) = 0;
  virtual void RemoveObserver(Observer*) = 0;

  // Central rule. Pure function of (requestor, top-frame, kind).
  virtual Decision Decide(const url::Origin& requestor,
                          const url::Origin& top_frame,
                          StorageKind kind) const = 0;

  virtual void AllowLoginException(const url::Origin& site) = 0;  // opt-in override
};
```

## Class model

| Class | Responsibility |
|-------|----------------|
| `StoragePolicyServiceImpl` | Holds config + exception list; implements `Decide` |
| `LoginAwareRule` | The pure decision rule (unit-tested in isolation) |
| `AuthExceptionStore` | Persisted per-site overrides (`AllowLoginException`) |
| `PhawCookieAccessDelegate : net::CookieAccessDelegate` | Enforces `Decide` for cookies |
| `StorageAccessThrottle : blink::URLLoaderThrottle` | Applies `Decide` to cache/storage requests |

```
StorageAccessThrottle ─┐
PhawCookieAccessDelegate ┼─▶ StoragePolicyServiceImpl.Decide(requestor, top, kind)
                         │        └─▶ LoginAwareRule + AuthExceptionStore
                         └─▶ Observer.OnThirdPartyBlocked (UI counter)
```

## The rule (LoginAwareRule) — small, total, testable

```
Decide(requestor, top_frame, kind):
    if requestor is same-site as top_frame:    return kAllow               # first-party = your login
    if AuthExceptionStore.contains(requestor): return kAllow               # explicit opt-in
    if kind == cookie:                         return kBlock               # 3p cookies = the cross-site identifier
    else (cache / localStorage / idb):         return kPartitionEphemeral  # NIK-partitioned, session-lifetime
```

- **Allow first-party** → login/session cookies *and your own site's cache*
  survive → you stay signed in.
- **Block third-party cookies** → closes the primary cross-site tracking
  identifier (the classic "follow you everywhere" vector).
- **Partition + ephemeral** third-party cache & storage → **Chromium already
  partitions the HTTP cache by Network Isolation Key**, so a third party cannot
  read it across sites; we keep that on and force third-party DOM storage to
  session-lifetime. We deliberately do **not** hard-block third-party cache:
  that would only re-download shared CDN subresources (fonts/JS) with no privacy
  gain over partitioning. Partition, don't block.

## Seams

- Cookies: `PhawCookieAccessDelegate` set on the `CookieMonster` via a
  `patches/` hook; builds on Chromium's `CookieSettings`.
- Cache/storage: `StorageAccessThrottle`; leverages Chromium's existing
  third-party **storage partitioning** rather than reinventing it.

## Dependencies (DEPS)

Allowed: `//base`, `//net`, `//services/network/public`,
`//components/content_settings/core`, `//components/phaw_common`.
Forbidden: other `//components/phaw_*`.

## Tests

- Unit: `LoginAwareRule` truth table across (same-site / cross-site) × kind ×
  exception — the whole policy provable without a browser.
- Browser test: log into a first-party site, restart → **still logged in**;
  a third-party tracker on that page → cookies **blocked**; its cache & storage
  **partitioned** (unreadable across sites) and cleared at session end.
