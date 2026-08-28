import unreal

# Phase 2 (Mass Entity player integration, 2026-08-29) - points each unit config's
# MassMovableVisualizationTrait at the new lightweight AGreyfieldMassUnitVisual actor class for
# its High-LOD (near/selected) representation. The trait's default LODRepresentation array is
# already [HighResSpawnedActor, LowResSpawnedActor, StaticMeshInstance, None] - this script only
# fills in *which* actor class to spawn for that top tier, previously left None.

PATHS = [
    "/Game/Data/Mass/MEC_GreyfieldUnit_Leader.MEC_GreyfieldUnit_Leader",
    "/Game/Data/Mass/MEC_GreyfieldUnit_Follower.MEC_GreyfieldUnit_Follower",
]

visual_class = unreal.GreyfieldMassUnitVisual

for path in PATHS:
    config = unreal.load_asset(path)
    if not config:
        print(f"FAILED to load {path}")
        continue
    entity_config = config.get_editor_property("config")
    traits = entity_config.get_editor_property("traits")
    found = False
    for trait in traits:
        if trait.get_class().get_name() == "MassMovableVisualizationTrait":
            trait.set_editor_property("high_res_template_actor", visual_class)
            found = True
            print(f"{path}: set high_res_template_actor = {visual_class}")
    if not found:
        print(f"{path}: no MassMovableVisualizationTrait found!")
    unreal.EditorAssetLibrary.save_loaded_asset(config)
    print(f"{path}: saved")
