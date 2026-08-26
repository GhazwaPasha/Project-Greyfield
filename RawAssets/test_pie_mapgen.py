import unreal

MAP_PATH = "/Game/Maps/Map_Small2v2"

level_subsys = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
level_subsys.load_level(MAP_PATH)
print("Loaded level:", MAP_PATH)

level_subsys.editor_request_begin_play()
print("PIE begin-play requested - GreyfieldMapGenTestTrigger.BeginPlay should fire on the next tick(s).")
