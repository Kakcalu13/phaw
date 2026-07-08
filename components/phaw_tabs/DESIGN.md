# phaw_tabs — collapsible tabs + hibernation (req 4)

Adds a collapse/expand UX on top of Chromium's tab-groups primitive, and — the
part users actually feel on low-spec Win8.1 machines — **hibernates** a group's
tabs when it collapses so they stop consuming memory/CPU, restoring on expand.

We build the UX and the hibernation policy; we reuse Chromium's tab-group data
model and its renderer-discard machinery. No fork of the tab strip.

## Public interface

```cpp
// public/collapse_controller.h
class CollapseController {
 public:
  class Observer : public base::CheckedObserver {
   public:
    virtual void OnGroupCollapsed(TabGroupId, int tabs_hibernated) {}
    virtual void OnGroupExpanded(TabGroupId) {}
  };
  virtual void AddObserver(Observer*) = 0;
  virtual void RemoveObserver(Observer*) = 0;

  virtual void ToggleCollapse(TabGroupId) = 0;
  virtual bool IsCollapsed(TabGroupId) const = 0;
  virtual void SetHibernateOnCollapse(bool) = 0;   // user setting
};
```

## Class model

| Class | Responsibility |
|-------|----------------|
| `CollapseControllerImpl` | Orchestrates collapse↔hibernate; owns policy + observers |
| `ITabHibernator` (iface) → `RendererDiscardHibernator` | Discards/restores a tab's renderer; wraps `resource_coordinator` LifecycleUnit discard |
| `CollapseStateStore` | Persists collapsed groups across restart (session restore) |
| `TabGroupCollapseListener` | Listens to Chromium's existing group collapse/expand events, forwards to controller |

```
user toggles group
      │
      ▼
CollapseControllerImpl.ToggleCollapse(group)
      ├─ update Chromium group model (collapsed = true)     ← reuse, not fork
      ├─ for each tab in group: ITabHibernator.Hibernate()  ← discard renderer
      ├─ CollapseStateStore.Save(group, collapsed)
      └─ Observer.OnGroupCollapsed(group, n)                ← UI badge "n asleep"
expand → reverse: ITabHibernator.Wake() reloads on activation
```

## Hibernation policy (why it matters on Win8.1)

- On collapse, each background tab in the group is **discarded** (renderer torn
  down, tab kept in strip). This is the same mechanism Chromium uses for memory
  pressure — we just trigger it on the collapse gesture.
- The **active** tab and any tab playing audio / holding a form are exempt
  (`ITabHibernator::CanHibernate` guards this) — no data loss.
- On expand or click, the tab reloads lazily. State is preserved via session
  restore, not kept resident.

## Seams

- Collapse/expand: `TabGroupCollapseListener` observes the tab-strip model — we
  react to Chromium's events, we don't add new tab-strip state.
- Discard: `RendererDiscardHibernator` calls the `resource_coordinator`
  LifecycleUnit discard API (public).
- Persistence: `CollapseStateStore` hooks session restore via `patches/`.

## Dependencies (DEPS)

Allowed: `//base`, `//chrome/browser/ui/tabs` (group model),
`//components/performance_manager` or `//chrome/browser/resource_coordinator`
(discard), `//components/sessions`, `//components/phaw_common`.
Forbidden: other `//components/phaw_*`.

## Tests

- Unit: `RendererDiscardHibernator.CanHibernate` exempts active/audible/form tabs.
- Unit: `CollapseControllerImpl` hibernates exactly the eligible tabs on collapse,
  wakes on expand (fake hibernator).
- Browser test: collapse a 5-tab group → 4 discarded, memory drops; expand →
  tabs restore; restart → collapsed state persists.
