import unreal

# Phase 2 (Mass Entity player integration, 2026-08-29) - verifies the representation LOD hybrid
# actually spawns a real, selectable AGreyfieldMassUnitVisual actor for a near/High-LOD Mass
# entity. AGreyfieldMassUnitVisual::BeginPlay logs its own entity-handle resolution, so this
# script just needs to spawn a squad, request PIE, and stay alive long enough for that log line
# to appear (the caller greps the log file directly - see PROJECT_STATUS.md's headless-testing
# note on why set_keep_python_script_alive is required for this).

MAP_PATH = "/Game/Maps/NewMap"
LEADER_CONFIG_PATH = "/Game/Data/Mass/MEC_GreyfieldUnit_Leader.MEC_GreyfieldUnit_Leader"

level_subsys = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
level_subsys.load_level(MAP_PATH)
print("Loaded level:", MAP_PATH)

actor_subsys = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
# Spawned right on top of NewMap's PlayerStart (-200, 0, 92) rather than far off at (500,500,100)
# - the earlier attempt at that farther point never got a HighResTemplateActor spawn after 20+
# real seconds, and MassLODSubsystem's near/mid/far distance thresholds for this trait haven't
# been inspected/tuned, so ruling distance-from-viewer out empirically is cheaper than reading
# more engine source blind.
trigger = actor_subsys.spawn_actor_from_class(
    unreal.GreyfieldMassSquadTestTrigger,
    unreal.Vector(-100.0, 200.0, 100.0),
    unreal.Rotator(0, 0, 0),
)
config = unreal.load_asset(LEADER_CONFIG_PATH)
trigger.set_editor_property("entity_config", config)
trigger.set_editor_property("squad_count", 9)
trigger.set_editor_property("spawn_origin_offset", unreal.Vector(0.0, 0.0, 0.0))
trigger.set_editor_property("destination_offset", unreal.Vector(0.0, 400.0, 0.0))
print("Spawned GreyfieldMassSquadTestTrigger squad_count=9 near PlayerStart.")

unreal.EditorPythonScripting.set_keep_python_script_alive(True)
level_subsys.editor_request_begin_play()
print("PIE begin-play requested - watch for 'GreyfieldMassUnitVisual' log lines.")
