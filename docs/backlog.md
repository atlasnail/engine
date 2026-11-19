# Backlog and non-goals

Ideas that are deliberately **not** in the 1.0 plan. Putting something here is not rejection; it is protection against changing the current milestone's scope. To promote an item, write an ADR, then add it to `docs/roadmap.md` with a milestone and exit criteria.

## Non-goals (do not build without an ADR that reverses this list)

| Item | Why not |
| --- | --- |
| 3D rendering, 3D physics | The engine is explicitly 2D; 3D would reshape the renderer, math and asset pipeline. |
| Mobile, console, web/WASM targets | Requires platform, input, packaging and certification work far beyond two desktop targets. |
| Full visual editor | Months of UI/tooling work before a proven runtime; command-line tools and hot reload cover development needs first. |
| General rigid-body physics | Tile/AABB platformers and arcade games do not need it; integrating one later is a dependency choice, not a core rewrite. |
| Built-in networking / multiplayer | A separate domain; games can add a library. |
| Native hot reload of game code | Complex per platform, conflicts with deterministic testing; Lua covers iteration. |
| Built-in storefront/achievements/cloud saves | Platform integrations with ongoing maintenance cost; not engine concerns. |
| Scripting languages beyond Lua for 1.0 | Each binding multiplies testing; the C ABI is the extension path. |

## Storefront and multiplayer integration planning (not implemented)

Steam achievements, Steam multiplayer and GOG Galaxy achievements are future
**optional game-side adapters**, not engine-core requirements. The existing 1.0
non-goals remain unchanged; promotion to a supported engine feature requires an
ADR and roadmap decision. No SDK, dependency, credentials or service is added now.

- Games own neutral achievement identifiers and their mapping to each store's
  identifiers. Adapters translate platform callbacks and errors without exposing
  SDK types to engine core or stable C/C++ interfaces.
- Offline or unsupported clients must still run; adapters should report
  unavailable/pending/failure explicitly and reconcile pending unlocks when
  online. Do not promise cross-store synchronization without a game-owned policy.
- Steam multiplayer involves lobby discovery, invites, transport, identity,
  authentication, disconnect/reconnect and abuse handling; it is separate from
  achievements and cannot be inferred from a common storefront abstraction.
  Games own authoritative rules and security, and choose a transport deliberately.
- At implementation time evaluate each vendor's current SDK terms, distribution
  rules, supported Windows/Linux binaries, callback threading, lifecycle,
  achievement synchronization, network privacy/security and entitlement testing.
  Pin SDK versions only after an ADR. Avoid sending player identifiers or game
  data to third parties without an explicit game-level privacy policy.
- Plan fakes/offline/error tests and real platform integration tests with test
  accounts and permitted app IDs; document differences across stores and OSes.
  No portable provider API or performance promise is committed yet.

## Deferred, plausible later (P2+)

| Idea | Trigger to consider | Notes |
| --- | --- | --- |
| Parallel ECS systems / jobified simulation | Update time exceeds F4 budget with 4+ cores idle | Must preserve determinism or be opt-in and documented. |
| Vulkan or D3D12 backend | GL 3.3 cannot deliver a required feature or measured problem on supported hardware | Behind the existing renderer interface. |
| C# / Python / Zig first-party bindings | C ABI proven and community demand is concrete | C ABI (M8) is the prerequisite. |
| Tilemap editor and visual scene editor | Games are being authored and data files become the bottleneck | Start with an external-format import, never a required tool. |
| Audio effects (reverb, filters, spatialization) | A shipped game needs it | Must fit the audio CPU budget. |
| Shader effects library (outline, palette swap, CRT) | Common requests across games | Keep as data, not engine policy. |
| Asset archive encryption/obfuscation | Shipping requirement appears | Be honest about its limits. |
| Crash reporting upload service | External users hit crashes | Local dumps first; never automatic upload without consent. |
| Steam/itch.io packaging helpers | First release target chosen | Keep in tools, not the engine core. |
| Controller rumble and advanced haptics | Action game demands it | SDL supports it; budget the API design. |
| Localization tooling/export (PO/XLIFF) | Translators join | Runtime tables are already planned in M6. |
| Deterministic replay recording/branching | Debugging complex gameplay bugs | Builds on the fixed-step determinism suite. |
| Multi-window | Tooling needs it | Platform layer must stay single-window-first until then. |
| Software renderer fallback | A target lacks GL 3.3 | Unlikely on the stated platforms. |
| Engine-level mod support / sandboxed content | A game needs it | Asset and script security becomes a real design problem. |

## Deferred until after 1.0 by default

- Additional platforms, language bindings, editors, networking, physics, 3D.
- Performance work beyond the published 1.0 targets, unless a measured regression appears.
- Cosmetic refactors of stable public API.
