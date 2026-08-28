import unreal

# Phase 2 (Mass Entity) PIE verification, 2026-08-29 - proves actual squad movement/positions
# in a live PIE session via the throttled log added to UGreyfieldSquadFormationProcessor.
# Runs against NewMap (the stable hand-built vertical slice), deliberately NOT Map_Small2v2 -
# the procedural map's Lit-mode-black bug is a separate, deprioritized issue and shouldn't be
# entangled with this test.

MAP_PATH = "/Game/Maps/NewMap"
LEADER_CONFIG_PATH = "/Game/Data/Mass/MEC_GreyfieldUnit_Leader.MEC_GreyfieldUnit_Leader"

level_subsys = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
level_subsys.load_level(MAP_PATH)
print("Loaded level:", MAP_PATH)

actor_subsys = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)

trigger = actor_subsys.spawn_actor_from_class(
    unreal.GreyfieldMassSquadTestTrigger,
    unreal.Vector(500.0, 500.0, 100.0),
    unreal.Rotator(0, 0, 0),
)
config = unreal.load_asset(LEADER_CONFIG_PATH)
if not config:
    raise RuntimeError(f"Failed to load {LEADER_CONFIG_PATH}")

trigger.set_editor_property("entity_config", config)
trigger.set_editor_property("squad_count", 9)
trigger.set_editor_property("spawn_origin_offset", unreal.Vector(0.0, 0.0, 0.0))
trigger.set_editor_property("destination_offset", unreal.Vector(0.0, 1200.0, 0.0))
print("Spawned GreyfieldMassSquadTestTrigger at (500,500,100), squad_count=9, destination offset (0,1200,0).")

# Without this, EditorPythonExecuter force-quits the editor one tick after this script returns
# (UEditorPythonScriptingLibrary::GetKeepPythonScriptAlive() defaults false) - confirmed via
# engine source (EditorPythonExecuter.cpp) after -ExecutePythonScript headless PIE runs kept
# capturing only a single tick's worth of log output no matter the in-processor logging budget.
# Setting this true keeps the process alive after the script finishes so PIE keeps ticking for
# real; the launcher (PowerShell) kills the process explicitly once it's done collecting log data.
unreal.EditorPythonScripting.set_keep_python_script_alive(True)

level_subsys.editor_request_begin_play()
print("PIE begin-play requested - watch for 'Squad move proof' log lines over the next several seconds.")
