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
  **Quality-bar decision, 2026-08-24**: mid-poly "sim-style" sourcing (Sketchfab/CGTrader/
  itch.io, filtered CC0/CC-BY) is now the default for units/vehicles/weapons, not the
  original flat-shaded Kenney/Quaternius low-poly bar — Greyfield is a Zero Hour successor
  and needs a grounded-realistic register, not a mobile-game one. Kenney/Quaternius drop to
  a fallback tier (props, UI, low-priority fill). Poly budget still matters against Phase
  2's Mass Entity LOD ceiling — vet each mid-poly find against that before committing it to
  a unit slot. Textures follow the same shift: ambientCG/Poly Haven PBR is now primary,
  Kenney's flat texture sets are fallback-only.
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
- **Blender MCP is now set up and used** (superseding the earlier "no Blender MCP" note) —
  needed because raw glTF/GLB imports of multi-material source files fragment into dozens
  of disconnected per-material StaticMeshes in Unreal (Interchange's Python-side
  `combine_static_meshes_behavior` pipeline option does not actually apply via
  `AssetImportTask.options`, confirmed by testing — don't rely on it). Fix: run each GLB
  through Blender headless first (`blender.exe --background --python <script>`, see
  `RawAssets/merge_meshes.py`) to join same-file mesh parts into one mesh (materials kept as
  slots), *only* for unrigged (no-armature) meshes — skip the join for skinned/rigged source
  files, since merging across skeletons is unsafe. Confirmed working: F-22 82→1 mesh, F-35B
  148→1, etc. `execute_blender_code_for_cli`'s `BLENDER_PATH` env var isn't set in this
  environment — invoke `blender.exe` directly via Bash/PowerShell instead (path:
  `C:\Program Files\Blender Foundation\Blender 5.2\blender.exe`).
- **Save everything before every rebuild, every time** (user's explicit standing
  instruction, 2026-08-24) — not just when convenient. `DataAssetTools.create` only creates
  an asset in memory; nothing persists until `AssetTools.save_assets` is called. Got bitten
  once: a freshly-created `UMassEntityConfigAsset` vanished entirely after an editor-restart
  rebuild because it was never saved.
- **Raw pre-import 3D assets live in `RawAssets/`** (project root, sibling to `Content/`),
  gitignored — not `Content/`, which is reserved for UE's own `.uasset` packages, and not a
  loose `Assets/` folder at root (that was the original drop location, normalized 2026-08-24).
  Organized `RawAssets/Units/<Role>/<Faction>/`, matching the Order of Battle's role
  taxonomy (Infantry, Aircraft, etc.) and the 4-faction roster — e.g.
  `RawAssets/Units/Aircraft/RedDragon/j-20.glb`. Filenames normalized to lowercase-kebab,
  no spaces, no site-cruft suffixes (`_3d_model`, `_-_free`, etc.) — this is a manual step
  each time a new batch of downloads lands, not automated. First batch: 16 GLB files (12
  aircraft — 5 United Alliance real-world F-15/16/22/35/B-2 analogues, 6 Red Dragon
  Chengdu/Shenyang/Xian analogues, 1 Crescent Coalition KAAN — plus 4 infantry, one per
  faction except GLF's currently-unfilled aircraft slot, consistent with GLF having no
  advanced-airframe doctrine per the Order of Battle). **Update 2026-08-25: imported into
  `Content/Units/<Role>/<Faction>/` and visually confirmed in a real PIE run** (F-22 and
  J-20 placed near the HQ, infantry placed too — F-22 later swapped for a replacement
  model the user supplied since the first had misplaced parts). Import is now scripted, not
  manual drag-drop: `unreal-mcp` isn't reachable from a fresh Claude session until the
  editor is already running (project `.mcp.json` server connects at session start only), so
  the pipeline is headless instead — `PythonScriptPlugin` + `EditorScriptingUtilities` are
  now enabled in `ProjectGreyfield.uproject` for this, and
  `UnrealEditor-Cmd.exe <uproject> -ExecutePythonScript="<script>" -unattended -nullrhi
  -stdout -FullStdOutLogOutput` runs an import script (`RawAssets/import_units.py`) without
  ever opening the GUI. `-nullrhi` avoids a real GPU crash dump hit once on a plain headless
  run. Infantry SkeletalMeshes imported clean (no fragmentation) directly from source;
  aircraft need the Blender pre-merge step above first. Still open: none of these are wired
  into the actual faction/unit gameplay classes yet (Order of Battle DataTable rows,
  placeholder-mesh swap-out) — this batch only proves the import+render pipeline works.
  **Also 2026-08-25: user said "stop using computer connecter" (windows-mcp click/drag GUI
  automation)** — it kept misfiring during this session (text typed into the wrong window,
  clicks landing on the wrong actor, a Claude desktop-app window showing this same
  conversation repeatedly stealing focus). Manual in-editor placement/drag-and-drop steps
  are now handed to the user with precise instructions instead of driven by Claude; scripted
  headless work and `windows-mcp Process`/`PowerShell` for the editor process lifecycle are
  still fine. See [[computer-use-preference]] memory for the full detail.
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

## Procedural map generation (started 2026-08-25)

**Design decision, 2026-08-25**: every Greyfield map is generated fresh per match (AoE4-style),
not hand-crafted like the existing `NewMap` level. Four size tiers, each 2x the components of
the last: **Small (2v2, 4 players)**, **Medium (3v3, 6 players)**, **Large (4v4, 8 players)**,
**Gigantic** (8 players placeholder, same as Large — whether Gigantic should mean *more* players
too is an open, undecided question). Small is the only tier actually generated and PIE-verified
so far; Medium/Large/Gigantic share the exact same code path via `GetPresetForSize` and are
believed to work, just unrun.

**What exists** (`Source/ProjectGreyfield/`): `GreyfieldMapGenerationTypes.h` (the
`EGreyfieldMapSize` enum + preset/spawn structs), `GreyfieldMapGenerationSubsystem.h/.cpp` (the
actual generator, a `UWorldSubsystem`), `GreyfieldMapGenTestTrigger.h/.cpp` (DEV/TEST ONLY
placeable actor that calls `GenerateMap` on `BeginPlay`, same role as
`AGreyfieldMassSquadTestTrigger` for Phase 2). `Landscape` added to `ProjectGreyfield.Build.cs`.

Algorithm: layered Perlin (`FMath::PerlinNoise2D`, 5-octave fBm) heightmap, sign-preserving power
curve biasing most of the map toward buildable plains with rolling ~30m hills, buildable pads
flattened under every spawn. Player spawns sit on a ring split into two 180-degree arcs (one per
team) so the two teams are always an exact point-reflection of each other; the heightmap is only
computed for the "top" half and mirrored into the bottom half, so the two teams' terrain is
provably fair by construction, not just statistically similar. Landscape built via
`ALandscapeProxy::Import()` (the same call the editor's own New Landscape panel uses, traced
through `LandscapeEditorDetailCustomization_NewLandscape.cpp` to get the exact call pattern:
`SpawnActor<ALandscape>` → set `LandscapeMaterial` → `SetActorRelativeScale3D` → `Import(...)` →
`GetLandscapeInfo()`), with `MTL_MWAM_AutoMaterial_MASTER` (the MW Landscape Auto Material plugin,
already in `Content/MWLandscapeAutoMaterial/`) assigned as the landscape material — no manual
paint layers needed, it blends purely from slope/height. Resolution presets are Unreal's own
standard values (63 quads/section, 1 section/component): Small=1009 verts/side (16
components, ~1008x1008m at the project's 1uu=1cm convention), Medium=2017, Large=4033,
Gigantic=8065.

**KNOWN GAP — packaging**: `GenerateMap` is `WITH_EDITOR`-gated.
`ALandscapeProxy::Import()` lives entirely inside Landscape's own `WITH_EDITOR` block in the
engine source (confirmed by reading `LandscapeProxy.h`/`LandscapeEdit.cpp` directly) — it's
compiled out of Shipping/packaged builds entirely, not just disabled. Worse than a compile-time
gap: it also asserts `GIsEditor || IsRunningCommandlet()` at runtime, confirmed the hard way — an
attempt to test it via `-game` (standalone) mode from the Editor binary crashed with exactly that
assertion inside `Landscape.dll`'s material-instance setup, even though `-game` still compiles
with `WITH_EDITOR=1`. It works from genuine Editor context only (including real PIE, since
`GIsEditor` stays true for the whole editor process during PIE) — this project currently only
ever ships through PIE, so this isn't blocking today, but packaging will need this ported to a
runtime-safe terrain representation (e.g. a `ProceduralMeshComponent` heightfield) or a
pre-bake-per-seed step. Not started.

**FIXED same day — player-spawn timing / black screen**: the dev-trigger-actor approach above had
a real bug, not just a cosmetic gap. `AGreyfieldGameMode`'s own initial `FindPlayerStart` call ran
*before* the dev trigger's `BeginPlay` had generated any `APlayerStart` actors (confirmed via log:
`FindPlayerStart: PATHS NOT DEFINED or NO PLAYERSTART with positive rating`, before the
`GreyfieldMapGen:` success line) — so the camera pawn fell back to spawning at world origin. The
user hit this for real: pressed Play on `Map_Small2v2` and got a black screen. Root cause,
confirmed by reading `GreyfieldRTSCameraPawn`'s spring arm math: world origin isn't guaranteed to
sit near the generated terrain's height (the noise field there could be anywhere in the -6m..+30m
range), so the camera — ~17m above its pawn's spawn point at a -60° pitch — ended up embedded
inside solid landscape geometry, rendering pure black no matter how long you waited.

**Fix**: moved map generation out of the dev trigger's `BeginPlay` entirely and into
`AGreyfieldGameMode::InitGame()` (gated behind a new `bGenerateProceduralMap` bool, default
`false`), which runs before any player login/spawn — structurally impossible to race now. Added
`AGreyfieldGameMode_Procedural` (trivial subclass, just flips that bool on) as `Map_Small2v2`'s
World Settings GameMode Override, so `NewMap` (still on the base `AGreyfieldGameMode`) is
untouched. `AGreyfieldMapGenTestTrigger` is no longer placed in the level (the class itself is
still there, harmless, for future ad hoc tests) — `RawAssets/create_map_small2v2.py` was updated
and rerun to rebuild the level without it. Also dropped the level's `NavMeshBoundsVolume`: it had
no real geometry to build against at edit time (the landscape doesn't exist until runtime), so it
only produced an editor "needs rebuild" nag with zero benefit — real runtime nav mesh rebuilding
for generated terrain is real future work (rebuild after `GenerateMap` completes, once
units/pathfinding are actually being tested on generated maps), not done here.

Re-verified the same way (headless PIE via `LevelEditorSubsystem.editor_request_begin_play()`):
log now shows `Game class is 'GreyfieldGameMode_Procedural'` → `Greyfield MapGen: generated...`
→ `Match State Changed... InProgress` → `PIE: Server logged in`, with **no**
`FindPlayerStart: PATHS NOT DEFINED` warning anywhere in the log — the ordering fix holds.
Not independently re-confirmed with real rendering (verification is still `-nullrhi`, no visual
check) — next actual Play in the editor by the user is the real confirmation.

**Separate, expected, NOT a bug**: the very first PIE run against `Map_Small2v2` in a session can
show a black/incomplete screen for several seconds while the MW Auto Material's large 4K textures
and landscape shaders compile live (`FLandscapePhysicalMaterialPS`, `FLumenCardCS`, etc. — logged
compile times up to ~10s total the first time). This is normal first-compile stall, not the bug
above, and gets faster on repeat plays once local DDC is warm (already true after this session's
verification runs).

**FIXED — root cause #2, light mobility**: the user still saw solid black (both in PIE and the
bare edit-time viewport, before any landscape even existed), with everything gameplay-side working
(HUD, resource counter, mouse-click building placement, minimap — strong evidence world geometry
and collision were genuinely fine). Inspected `NewMap`'s own lighting actors for comparison
(headless Python, `LevelEditorSubsystem`) and found the mismatch: this project runs
`r.AllowStaticLighting=False` (fully dynamic lighting, confirmed in `DefaultEngine.ini`), but
`spawn_actor_from_class` defaults `DirectionalLight`/`SkyLight`/`SkyAtmosphere` to **Stationary**
mobility — which can't render at all with static lighting disabled project-wide. `NewMap`'s own
lights are Movable. **Fix applied**: `RawAssets/create_map_small2v2.py` now explicitly sets
`mobility = MOVABLE` on every lighting/sky actor (Sun, SkyLight, SkyAtmosphere, and an added
`ExponentialHeightFog` — `NewMap` has one, the level didn't), and matches `NewMap`'s exact light
values (`DirectionalLight` intensity 6.0, `SkyLight` intensity 1.0 + real-time capture). Rebuilt
and re-tested — **this did not fully fix it**: Lit mode was still black afterward. Confirmed
applied and ruled out as the *sole* cause, not confirmed as a contributing one either — genuinely
unresolved whether it mattered at all.

**FIXED — root cause #3, wrong material asset**: with mobility/geometry/collision all seemingly
fine and Lit still black, asked the user to switch the viewport to **Unlit** to isolate
lighting from geometry/material — it showed the terrain in full 3D detail, but every inch of it
tiled with the literal text **"ADD COLOR TEXTURE TO MATERIAL"**. That's
`MTL_MWAM_AutoMaterial_MASTER`'s own built-in missing-texture placeholder: the master material is
a *template* meant to be turned into a Material Instance with real textures plugged into its
parameters, not applied directly to a landscape — which is exactly what `GenerateMap` was doing.
**Fix**: point `LandscapeMaterial` at one of the plugin's three ready-made Material Instances
instead of the bare master — `MTL_MWAM_Landscape_MountainRangeExample`, picked as the best fit for
this generator's rolling-hills output. Rebuilt; re-checked Unlit and it now shows real snow/rock
texture detail, no more placeholder. **Not yet done**: a dedicated Greyfield-specific Material
Instance with its own tuned parameters (not tied to the plugin's own demo map).

**⚠ STILL OPEN, not fixed — root cause #4, unknown**: with mobility corrected AND the real
textured material confirmed working in Unlit, **Lit mode is still solid black**. This is the
actual live blocker, not yet root-caused. Everything gameplay/geometry/collision/material-side is
now independently confirmed fine — this is isolated specifically to the lighting/post-process
render pass.

**2026-08-26 session — two of the leading theories ruled out, one positive signal found, one new
open question:**
- **PostProcessVolume theory ruled out.** Headless inspection (`RawAssets/inspect_lighting_v2.py`)
  of both levels' actors found **neither `NewMap` nor `Map_Small2v2` has a `PostProcessVolume`
  actor at all** — so its absence can't be what's different between a level that renders fine and
  one that doesn't. (Both do have Movable Sun/SkyLight/SkyAtmosphere/ExponentialHeightFog with
  matching values, confirming last session's mobility fix is still correctly in place on both.)
- **World-Partition-streaming-proxy theory ruled out before it was even tested**: research turned
  up a real, confirmed Epic forums bug where `ALandscapeProxy::Import()` doesn't create streaming
  proxies in a **World Partition** level unless you also call
  `ULandscapeSubsystem::ChangeGridSize()` — but re-reading `RawAssets/create_map_small2v2.py`
  confirmed `Map_Small2v2` is deliberately built as a **plain, non-World-Partition level**
  ("doesn't need it" — see that script's own comment), unlike `NewMap`. So this bug doesn't apply
  here; it was a dead end specific to WP levels. (`NewMap` itself does use WP — headless actor
  count confirmed a real `Landscape` + ~64 `LandscapeStreamingProxy` actors there, all
  editor-authored, not runtime-imported — a structurally different landscape-creation path from
  `Map_Small2v2`'s, which is the one remaining real difference between the two levels' landscapes.)
- **Positive signal for the Lumen/ray-tracing-registration theory**: ran a real (GPU-backed, not
  `-nullrhi`) headless PIE session for the first time (`UnrealEditor-Cmd.exe` with
  `-ExecutePythonScript=RawAssets/test_pie_mapgen.py`, no `-nullrhi`) and captured the actual
  D3D12/ray-tracing log output. Confirmed: `LogRenderer: Recreating Persistent SBTs ... 
  NumGeometrySegments changed: current: 0 - new: 512` fires right after PIE reaches
  `InProgress` — i.e. the runtime-created landscape **does** get picked up and rebuilds the ray
  tracing acceleration structure with real geometry segments, so it's not being silently skipped
  by the RT scene at a structural level. Also confirmed generation itself still works cleanly in
  a real-RHI run: `Greyfield MapGen: generated EGreyfieldMapSize::Small2v2 map, seed=19368,
  1009x1009 verts, 4 player starts`.
- **New, unexplained finding — not yet root-caused, may be a red herring**: this same real-RHI
  headless run auto-issued a `QUIT_EDITOR` console command and fully exited **~350ms after PIE
  hit `InProgress`**, on its own, with no crash/error/assert logged — `LogStaticMesh: Abandoning
  remaining async distance field tasks for shutdown` / `...card representation tasks for
  shutdown` confirm the Lumen card-representation build for the landscape was still async/in-flight
  at that moment, before the process closed. Cause of the auto-quit itself is unknown — happened
  with no keypress or script command sent, didn't reproduce when the earlier `-nullrhi` runs were
  used (those don't tick real rendering, so this may be specific to real-RHI + `-unattended` +
  `-ExecutePythonScript` together). **Practical effect**: this caps any headless real-RHI diagnostic
  run to about 1 second of actual rendering — not enough to get past the already-documented normal
  first-few-seconds shader/DDC-warmup stall, so this session's headless attempt is genuinely
  inconclusive on whether Lit mode is still black after that stall passes. Didn't chase the
  auto-quit further; flagging it here in case it recurs or turns out to matter.
- **Genuine blocker for finishing this headlessly**: `CaptureViewport` doesn't reflect PIE (existing
  documented limitation) and nullrhi runs render nothing, so there is currently no way for me to
  visually confirm whether Lit mode is still black past the warmup stall without either (a) the
  user pressing Play and looking, or (b) screenshotting a live GUI-editor PIE session myself
  (allowed per [[computer-use-preference]]'s narrower "screenshot to verify, don't click/type"
  carve-out — not yet tried this session, worth doing next before asking the user).
- **Fix attempt applied same session, not yet visually confirmed**: since neither map had a
  `PostProcessVolume` and the project therefore has no per-level exposure override at all,
  `RawAssets/create_map_small2v2.py` now also spawns one — unbound, auto-exposure method
  `Histogram`, min brightness 0.03 / max 8.0 / bias 0.0 (wide enough to never itself clip the scene
  toward black). Script rerun to rebuild the level; headless inspection confirms it saved correctly
  (`unbound: True`, all three overrides present with those exact values). **Deliberately left
  `NewMap` untouched** — it already renders correctly with zero `PostProcessVolume`, so this is
  additive insurance on `Map_Small2v2` only, not a change applied project-wide. This may or may not
  be the actual root cause (see the ruled-out theories above — it's the one remaining
  lighting-side difference between the two levels I could find and fix without live rendering), so
  **still needs a real Play-in-editor check** — genuinely can't go further without eyes on the
  screen: `CaptureViewport` doesn't reflect PIE, `-nullrhi` renders nothing, and a same-session
  attempt at a real (GPU-backed) headless PIE run to get render-pass log diagnostics hit a new,
  unexplained problem (below) that capped it to ~1 second of actual rendering — not enough to get
  past the already-documented normal first-few-seconds shader/DDC warmup stall.
- **Positive signal for the Lumen/ray-tracing-registration theory** (from that ~1-second real-RHI
  run, before the PostProcessVolume fix above was applied): confirmed via log that the runtime-
  created landscape does get picked up by the ray tracing acceleration structure rebuild right as
  PIE reaches `InProgress` (`LogRenderer: Recreating Persistent SBTs ... NumGeometrySegments
  changed: current: 0 - new: 512`) — so it's not being silently skipped by the RT scene at a
  structural level, at least not immediately. Map generation itself also logged clean in this
  real-RHI run: `Greyfield MapGen: generated EGreyfieldMapSize::Small2v2 map, seed=19368,
  1009x1009 verts, 4 player starts`.
- **New, unexplained finding from that same run — not root-caused, may be a red herring**: the
  real-RHI headless run (`UnrealEditor-Cmd.exe`, no `-nullrhi`, `-unattended
  -ExecutePythonScript=RawAssets/test_pie_mapgen.py`) auto-issued a `QUIT_EDITOR` console command
  and fully exited **~350ms after PIE hit `InProgress`**, on its own — no crash, no error, no
  assert logged, and nothing sent from my side that would trigger it. `LogStaticMesh: Abandoning
  remaining async distance field tasks for shutdown` / `...card representation tasks for shutdown`
  confirm the landscape's Lumen card-representation build was still in-flight (async, normal) at
  that moment. Didn't reproduce with earlier `-nullrhi` runs (those don't tick real rendering at
  all, so may be specific to real-RHI + `-unattended` + `-ExecutePythonScript` together — untested
  whether it's `-unattended` specifically). Not chased further this session; flagging in case it
  recurs or turns out to matter for future headless real-rendering verification attempts.
- **PostProcessVolume fix confirmed NOT the (sole) cause — user tested it same session**: Lit mode
  still solid black after the PPV fix above. Crucially, the user also reported **"Lighting Only"
  view mode is black too**, not just Lit. This is a much sharper signal than Lit-vs-Unlit alone:
  Lighting Only shows the raw light contribution with material/albedo stripped out and, like
  Unlit, bypasses tonemapping/exposure compensation — if it reads as black, the surface is
  receiving essentially no *perceptible* light, full stop, independent of any
  exposure/tonemap/PostProcessVolume question (which this observation makes moot as a cause,
  though the PPV added above is still worth keeping as standing insurance).
- **Root cause #4 found (high confidence), fix applied, not yet re-tested**: `create_map_small2v2.py`
  set the Sun's `DirectionalLightComponent.intensity` to `6.0`, copied from `NewMap`'s own working
  value on the assumption that matching a known-good number would carry over safely. It doesn't —
  UE5 `DirectionalLight` intensity is physical **Lux**: real daylight runs roughly 1,000–10,000 lux
  (overcast) up to ~100,000 lux (direct sun), so 6.0 is deep-twilight level, functionally
  imperceptible without heavy exposure compensation. That exactly explains the symptom pattern:
  Unlit ignores light entirely so it looks fine; Lit's auto-exposure tries to compensate but
  apparently can't pull a signal that dim up to visible; Lighting Only has zero exposure
  compensation and shows the true (near-zero) light level directly, i.e. black. `NewMap`'s simple
  placeholder-box scene apparently reads as acceptable at the same 6.0 lux (smaller/closer geometry,
  different overall scene response) even though the value itself was always this dim — this was
  never actually a landscape-specific or Lumen-specific bug, just an unphysical light value that
  happened to be tolerable for one scene and not the other. **Fix applied**: bumped intensity to
  `80000.0` (physically-plausible direct-daylight range, matching the level Epic's own
  SkyAtmosphere-paired sample levels use), left `SkyLight` untouched (real-time-capture, scales
  with the actual scene automatically). Rebuilt and headless-verified the script ran clean
  (`save_map returned: True`). **Not yet visually re-confirmed** — needs the same real
  Play-in-editor check as before.
- **Next concrete step, in order**: (1) user presses Play on `Map_Small2v2` again and reports
  whether Lit/Lighting-Only are still black — this is the most likely actual fix of the four things
  tried across this bug (mobility, material, PostProcessVolume, now light intensity). (2) If still
  black, check the viewport's **Lumen visualization view mode** (View Mode → Lumen → "Surface
  Cache" or "Final Gather") — this directly shows whether the landscape has a valid Lumen scene
  representation, and would confirm or rule out the runtime-Import()-registration theory despite
  the positive RT-scene signal noted above (the ray tracing scene and the Lumen surface cache are
  registered separately, so one being fine doesn't guarantee the other is). (3)
  `r.DynamicGlobalIlluminationMethod=0` (Lumen off) as a diagnostic if (2) is inconclusive.

**Verified 2026-08-25**: `unreal-mcp` was not reachable this session (editor wasn't running yet
when the session started — matches the documented "connects at session start only" gotcha), so
verification went through the same headless-Python pattern as asset import, extended one step
further: `UnrealEditor-Cmd.exe <uproject> -ExecutePythonScript=<script> -unattended -nullrhi`
where the script itself calls `unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)`,
`.load_level(...)`, then `.editor_request_begin_play()` — the exact API the editor's own Play
button calls, so `GIsEditor` stays true and the run is a genuine PIE session, all headless with no
GUI click needed. Real PIE log confirms clean success: *"Greyfield MapGen: generated
EGreyfieldMapSize::Small2v2 map, seed=6107, 1009x1009 verts, 1008x1008m, 4 player starts"* — no
crash, no material-load warning (MW Auto Material loaded fine). A new level,
`/Game/Maps/Map_Small2v2`, was created headlessly for this (`RawAssets/create_map_small2v2.py`)
with lighting, a nav mesh bounds volume, `AGreyfieldGameMode` set as World Settings' GameMode
Override, and one `AGreyfieldMapGenTestTrigger` (Small2v2, seed 0 = random every run) placed —
`NewMap` (the real hand-built vertical-slice level) was left untouched. Not yet done: an actual
visual look at the generated terrain/material blend — `-nullrhi` means nothing rendered this run,
and PIE-only content doesn't show up in `CaptureViewport` per the existing documented limitation.
The user can see it for real by opening the editor on `Map_Small2v2` and pressing Play (a fresh
terrain generates every time, by design).

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
