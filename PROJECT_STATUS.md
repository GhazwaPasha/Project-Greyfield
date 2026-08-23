# Operation Greyfield — Project Status

*Last updated: 2026-08-24. Read this first in any new session before doing anything else.*

## What this is

An original RTS in Unreal Engine 5.8, built via the `unreal-mcp` MCP server (Unreal's own
MCP plugin — driven directly, not through Blueprint editing by hand). Same base-building /
resource / command mechanics as classic 2000s RTS games, same structural formula as
C&C Generals Zero Hour (verified: 3 sub-factions per faction sharing a base roster, not
full separate rosters) — but every asset, name, unit, and mechanic is original. No EA IP,
no reskins.

Project root: `C:\Users\Ghazw\Desktop\Project Greyfield\` (flat single folder — do not
recreate the nested `ProjectGreyfield\ProjectGreyfield\` structure it used to have).

## Where we are

**Playable right now, end-to-end, in PIE**: pan/select/move units, harvest resources,
place buildings, queue a unit from a selected building, fight with team-based combat
(GAS-backed health/damage), lose to an AI opponent that builds and attacks on its own
timer without cheating. All placeholder primitive geometry (cubes/cylinders) — no real
art yet.

| Phase | Status |
|---|---|
| 0 — Scaffolding | ✅ Done |
| 1 — Camera & Selection | ✅ Done |
| 2 — Squads at Scale (Mass Entity) | 🔶 In progress — C++ backbone lands 2026-08-24 (see below), content/PIE proof still blocked |
| 3 — Base & Economy | ✅ Done (real gaps tracked, see below) |
| 4 — Combat & GAS | ✅ Done (real gaps tracked, see below) |
| 5 — AI Opponent | ✅ Done — **confirmed with a real unattended PIE loss**, not just "the code runs" |
| 6 — Fog, UI, Art Pass | 🔶 Code-side done (fog of war, real HUD, minimap, win/loss, command card v0); remainder is asset-blocked, not code-blocked |

### Known code gaps (honest, not hidden)
- `DT_Buildings` DataTable exists but is never read — building placement uses one flat
  hardcoded cost, not real per-building-type data
- Secondary resource ("power") is tracked on `AGreyfieldPlayerState` but nothing produces
  or consumes it — no gating exists yet
- No real `GameplayEffect` assets — damage sets the AttributeSet directly, bypassing GAS's
  actual effect pipeline; zero `GameplayAbility` classes exist yet either
- AI director spawns units for free on a timer — no economy of its own yet
- Only one map exists, no save/load, no main menu (boots straight into the test level)

## Key documents (all Artifacts, all still live)

- **[Build Log](https://claude.ai/code/artifact/bd2f7934-8a2f-4ed9-b2c9-c63c88d85009)** —
  phase-by-phase implementation history + a prioritized backlog. The backlog was drawn up
  but the user asked to stick to the original phase order for now rather than reprioritize
  by it — still worth reading for the honest gap list per phase.
- **[Supply Manifest](https://claude.ai/code/artifact/ce37d842-6fbc-41bb-8233-fef749b66dd7)**
  — free asset sourcing plan (models/textures/music/SFX/voice/UI), all CC0-or-compatible.
  **User is sourcing assets themselves** — this is reference only, not a task queue for me.
- **[Faction Roster](https://claude.ai/code/artifact/bcabf1c0-538d-46b4-a07d-818c54125b3b)**
  — military/economy asset shopping list per faction (superseded in priority by "user
  sources it themselves," but the unit list itself is still the design reference).
- **[Order of Battle](https://claude.ai/code/artifact/4e9ec445-9098-4df3-afdb-d1a32708e1b5)**
  — the actual game design: universal counter matrix, the 4-faction/12-sub-faction
  structure, unit-to-unit parity rules, rank-gated general powers (R1/R3/R5/R7). **This is
  design only — none of the 12 sub-factions exist in code yet**, only the single shared
  roster currently implemented.

## Faction design (locked in, not yet built in code)

Four factions, three sub-factions each, all "conventional modern military" register (real
hardware, no invented sci-fi tech):

1. **United Alliance** (western) — precision doctrine. Subs: Special Operations Command,
   Strike Command, Systems Command.
2. **Red Dragon** (Chinese-style) — mass doctrine. Subs: People's Armor Corps, People's
   Infantry Corps, Strategic Rocket Corps.
3. **Crescent Coalition** (Turkey+Pakistan+Middle East pact) — combined-arms doctrine,
   genuinely new, no Zero Hour equivalent. Subs: Logistics Command, Armored Cavalry, Air
   Defense Command.
4. **Global Liberation Front** — irregular doctrine, real-world hardware not sci-fi. Subs:
   Ambush Cells, Sapper Corps, Insurgent Logistics.

Sub-factions unlock as mid-match branching paths (not preselected), per the user's own
design intent — that branching mechanic doesn't exist in code yet, it's still just the
single shared roster.

**⚠ Open flag, not resolved**: "Global Liberation Front" reads very close to "Global
Liberation Army" (GLA) — same acronym shape, same role as Zero Hour's third faction. Raised
with the user once; they haven't decided on a rename. Worth re-raising if it comes up again,
not blocking anything.

## Infrastructure — how to actually work in this project

- **`.mcp.json` lives at the project root** — `unreal-mcp` HTTP server on
  `127.0.0.1:8000/mcp`, only responds while the Unreal Editor is open on this project.
- **I compile and restart the editor myself now** — the user does not want to be asked to
  touch the editor for routine work. Full self-serve loop:
  1. `mcp__windows-mcp__Process` (mode `list`, name `UnrealEditor`) to check if it's running;
     kill it (mode `kill`, `force: true`) if so.
  2. Clean rebuild via `PowerShell`:
     ```powershell
     $targetArg = '-Target="ProjectGreyfieldEditor Win64 Development -Project=""C:/Users/Ghazw/Desktop/Project Greyfield/ProjectGreyfield.uproject"""'
     & "C:\Program Files\Epic Games\UE_5.8\Engine\Build\BatchFiles\Build.bat" $targetArg -WaitMutex
     ```
     A real `Link [x64] UnrealEditor-ProjectGreyfield.dll` line in the output confirms it's
     a proper baseline build, not a Live-Coding-only patch.
  3. Relaunch: `mcp__windows-mcp__PowerShell` → `Start-Process -FilePath "<path>\ProjectGreyfield.uproject"`.
     First launch after certain changes may show a "Select an app to open this .uproject
     file" picker — screenshot, click "Unreal Engine," click "Just once."
  4. Wait ~40-60s, then poll `netstat -ano | grep "127.0.0.1:8000"` until it's listening.
  5. `SceneTools.get_current_level` will likely show a scratch `/Temp/Untitled_*` level —
     always `load_level` back to `/Game/Maps/NewMap` before trusting `find_actors` results.
- **Live Coding (`Ctrl+Alt+F11`) is unreliable in this project** — hit two real editor
  crashes (`Cast of ... Default__Object to Actor failed` during PIE's World Partition
  streaming generation) from spawning instances of classes that only existed via a Live
  Coding patch, never a clean baseline build. **Default to the full clean-rebuild cycle
  above for anything nontrivial** rather than Live Coding, even though it costs an editor
  restart each time.
- **UMGToolSet uses camelCase params** (`folderPath`, not `folder_path`) — inconsistent
  with the snake_case most other toolsets use. Check the error's schema dump if a call
  fails with "required param missing."
- **`CaptureViewport` returns an image unrelated to the actual PIE session** — don't trust
  it for visual verification; use `GetLogEntries` + `find_actors`/`get_properties` on live
  PIE actors instead.
- No Blender/3D-authoring MCP is set up — user said to forget that direction, they're
  sourcing assets manually.
- **Save everything before every rebuild, every time** (user's explicit standing
  instruction, 2026-08-24) — not just when convenient. `DataAssetTools.create` only creates
  an asset in memory; nothing persists until `AssetTools.save_assets` is called. Got bitten
  once: a freshly-created `UMassEntityConfigAsset` vanished entirely after an editor-restart
  rebuild because it was never saved.
- **This level uses World Partition** — placed actors live in their own external-actor
  packages, not inside the `.umap` itself, so `AssetTools.is_dirty("/Game/Maps/NewMap")` can
  read `false` right after placing a brand-new actor (the actor's own package is what's
  dirty, not the map). `SceneTools.save_actor` is meant for this but errored on a
  never-before-saved actor ("Asset does not exist" for its not-yet-created external package)
  — use `AssetTools.save_assets` with an **empty list** (saves everything dirty project-wide)
  instead; that worked reliably where the specific-path attempts didn't.
- **`CaptureViewport` still doesn't reflect the live PIE world** (matches the older
  documented gotcha, confirmed again 2026-08-24 with the new annotated version of the tool):
  it renders the editor's own level viewport, so entities that only exist in a running PIE
  simulation (e.g. spawned Mass entities) never appear in the capture, even though existing
  level actors (HQ, resource nodes, etc.) do. Don't trust it for verifying anything that only
  exists at PIE runtime — use `GetLogEntries` instead.

## What's next

Following the **original phase order** (user explicitly said to stick to this, not the
backlog's value-ranked reordering):

**Phase 2 — Mass Entity migration.** Its own deferral condition ("ship the vertical slice
on plain Actors, migrate once the game is proven fun") is now satisfied — Phase 5's real
win/loss confirms it. This is flagged as the steepest phase in the whole plan: a genuine
ECS migration for unit movement/steering, hand-written in C++ since no MCP toolset touches
Mass Entity. Currently tested to ~11 units on plain `Character`+NavMesh; target is 2000+
without a framerate cliff.

The architecture for hitting 2000+ is already locked in (Build Log, Phase 2 section, added
2026-08-24) — don't re-derive it: tiered representation LOD (full skeletal mesh near/selected
→ ISM further out, animation tick reduced/cut by distance), tiered simulation LOD (full
AI tick near/in-combat, reduced or coarse elsewhere), squad-leader-only NavMesh queries
(followers use formation offsets, not their own pathfinding), and a spatial hash grid for
targeting/avoidance/fog-of-war instead of O(n²) per-unit scans. Same pattern Epic's own
Mass Entity samples use at similar scale.

**C++ backbone landed 2026-08-24** (compiles clean, editor boot verified with 0 map-check
errors):
- `MassGameplay` + `MassAI` plugins enabled in the `.uproject`; `MassEntity`, `MassCore`,
  `MassCommon`, `MassSpawner`, `MassSimulation`, `MassMovement`, `MassRepresentation`,
  `MassLOD`, `MassActors`, `MassNavigation`, `MassNavMeshNavigation` added to
  `ProjectGreyfield.Build.cs`
- `GreyfieldMassFragments.h` — `FGreyfieldMassAgentFragment` (mutable team/health/attack
  cooldown), `FGreyfieldMassAgentParams` (const-shared per-unit-type stats), and
  `FGreyfieldSquadFragment` (squad id/leader flag/formation offset/leader handle)
- `GreyfieldMassAgentTrait` / `GreyfieldMassSquadTrait` — compose those fragments into a
  Mass entity config alongside Epic's stock Movement/Steering/Avoidance/NavMesh-Navigation
  traits
- `GreyfieldSquadFormationProcessor` — the squad-level-pathfinding piece: only the squad
  leader entity ever queries the NavMesh; every follower's move target is written each
  tick from the leader's resolved position + a formation offset
- `GreyfieldMassSubsystem` — `FormSquad()` / `IssueSquadMoveOrder()`, the gameplay-facing
  entry point for ordering a Mass-simulated squad around
- `GreyfieldMassCombatProcessor` (added same day, second slice) — lets entities with **no
  actor at all** still fight: builds two spatial hash grids per scan (Engine's own
  `THierarchicalHashGrid2D`, the same tool Mass's own avoidance system uses internally,
  same sizing as `FNavigationObstacleHashGrid2D`), one per team, so each entity only ever
  queries the *opposing* team's grid for nearest-target-in-range. Throttled to a 0.25s scan
  interval rather than every frame (attack cooldown still accumulates every scan, so fire
  rate isn't distorted). Damage applied directly to the health fragment, matching the
  existing Actor-side GAS simplification rather than introducing a second damage model.
  Compiled clean, editor re-verified booting healthy (0 map-check errors) after this landed.

**`unreal-mcp` reconnected 2026-08-24 — content authoring + first real PIE proof landed:**
- Two `UMassEntityConfigAsset`s built via the reconnected toolset:
  `/Game/Data/Mass/MEC_GreyfieldUnit_Leader` (Movement/Steering/Avoidance/NavMesh-Navigation/
  AssortedFragments[Transform+Actor]/DistanceLODCollector/MovableVisualization/GreyfieldAgent/
  GreyfieldSquad traits) and `MEC_GreyfieldUnit_Follower` (same minus NavMesh Navigation —
  the actual leader/follower archetype split the Phase 2 design calls for).
- `AGreyfieldMassSquadTestTrigger` (dev-only, `Source/ProjectGreyfield/`) + a new
  `UGreyfieldMassSubsystem::SpawnAndTestSquad()` spawn entities directly via
  `UMassSpawnerSubsystem::SpawnEntities`, then call the real `FormSquad`/`IssueSquadMoveOrder`
  path — proves the actual gameplay-facing API, not a separate test-only shortcut.
- **Real PIE run, zero Mass Entity Template validation errors**, log-confirmed: *"spawned 9
  entities from 'MEC_GreyfieldUnit_Leader'... squad formed, move order issued."* Getting to
  zero errors took three real, fixed bugs (see Build Log for detail): missing
  `FTransformFragment`/`FMassActorFragment` (fixed via `MassAssortedFragmentsTrait`), the
  wrong LOD-collector trait (`MassLODCollectorTrait`'s processor doesn't auto-register;
  swapped to `MassDistanceLODCollectorTrait`, which does), and the test actor itself having
  no root component (`GetActorLocation()` silently returned zero).
- **Not yet proven**: actual squad movement/positions in PIE. `CaptureViewport` reflects the
  editor's own level viewport, not the live PIE world — Mass entities that only exist in the
  running simulation don't show up in it (same limitation already noted below, not a new
  bug). A timed debug log of leader/follower positions inside the processors would close
  this out cleanly; not done yet.
- Squad leader death/succession and formation types are still just noted gaps (see above),
  unchanged by this pass.
- The *existing* selection/order UI still targets `AGreyfieldUnit` actors only — nothing
  yet calls `UGreyfieldMassSubsystem` from the player's actual move/attack-move order path.
  Deliberately not touched: there's currently no way for the player to select/own a
  Mass-simulated squad in the first place, so wiring this blind (untestable without content)
  risks writing wrong integration code. This needs a real design decision once content
  authoring is unblocked, not a guess now.
- Fog-of-war (per-unit-per-tick today) will need to move to per-grid-cell before unit
  counts climb anywhere near 2000 — not started, same reason (untestable without content).
- **Squad leader death isn't handled** (caught 2026-08-24 while authoring content): if the
  leader entity dies, followers' lookup for it just fails silently — no crash, but they
  freeze in place with no re-pathing and no promotion. Needs succession logic. Also, leader
  selection today is arbitrary (`FormSquad` just picks `SquadEntities[0]`, whatever's first
  in the array). Proposed fix for both, not yet implemented: promote/select whoever is
  closest to the squad's centroid (cheap, and a central unit's new path looks nearly
  identical to the old leader's so followers don't visibly lurch) — rejected "front-most
  toward destination" since that unit takes fire first and would make succession trigger
  constantly. Implementing this properly means giving every entity the NavMesh Navigation
  trait's fragments (not just a separate "leader config"), since Mass fragment composition
  is fixed at spawn — can't flip a follower into a leader without it already carrying those
  fragments. **User said to note this and move on rather than fix now** — the
  Leader/Follower config split below still reflects the old (unfixed) design.
- **No real formation system** (asked about 2026-08-24: "like AoE4?" — no). What exists is
  just an anti-stacking grid: `FGreyfieldSquadFragment.FormationOffset` assigns each
  follower a fixed row/column slot behind the leader, nothing more. AoE4-style formations
  (selectable line/box/staggered types, facing-aware spacing that reorients as the group
  turns, ranged-behind-melee role awareness, holding shape through combat) are **not
  built**. Would layer on top of the existing squad/leader-follower structure reasonably
  well (the `FormationOffset` field already exists, would just need real per-type offset
  patterns instead of one fixed grid, plus a player-facing formation-select UI) — noted as
  backlog, not started, per user's explicit "note it and move on."

Separately, whenever it comes up again: building the 12-sub-faction system in code (new
unit classes/DataTable rows per the Order of Battle doc) is a large, distinct task from
Mass Entity — worth its own dedicated pass, not squeezed in alongside it.

## Session-behavior notes (how the user wants me to work)

- **No small stops.** Don't pause for permission on routine implementation steps. Surface
  only at real blockers, finished milestones, or genuine decisions only the user can make
  (like the GLF naming flag above).
- **Handle the whole build/restart process myself**, including running Build.bat directly —
  established explicitly, see Infrastructure section.
- **Report honestly, including gaps.** The user pushed back once on phase-status framing
  that implied more completeness than was real ("done" hid real gaps) — the Build Log and
  this file both now carry explicit known-gaps lists rather than clean checkmarks.
