# phaw_common — shared interface layer

The **only** place a feature may reach for something another feature also needs.
Its purpose is to keep the "features never depend on each other" rule (D2) true.

## What belongs here

- Small, stable **interfaces** and value types shared by ≥2 features.
- Cross-cutting utilities with no feature logic (e.g. a `PhawObserverList`
  helper, common `mojom` value structs, the `phaw_features.gni` flag declarations).

## What must NOT go here

- Any feature's business logic. If it decides *ad-blocking* or *storage* policy,
  it lives in that feature, not here.
- Back-references to UI or to a specific feature impl.

## Dependency rule

`phaw_common` depends **only on the Engine** (`//base`, `//content/public`,
mojo). It never depends on `//components/phaw_adblock`, `_tabs`, etc. — those
depend on *it*. This one-way arrow is what prevents a dependency cycle (tangle).

```
phaw_adblock ─┐
phaw_tabs     ─┼─▶ phaw_common ─▶ Engine
phaw_login_cache ─┘
phaw_captions ─┘        (no arrows back up, no arrows sideways)
```

Keep this layer **thin**. A fat "common" module is how shared-utility tangle
starts; when in doubt, leave it in the feature.
