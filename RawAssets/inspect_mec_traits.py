import unreal

PATHS = [
    "/Game/Data/Mass/MEC_GreyfieldUnit_Leader.MEC_GreyfieldUnit_Leader",
    "/Game/Data/Mass/MEC_GreyfieldUnit_Follower.MEC_GreyfieldUnit_Follower",
]

for path in PATHS:
    config = unreal.load_asset(path)
    if not config:
        print(f"FAILED to load {path}")
        continue
    print(f"=== {path} ({config.get_class().get_name()}) ===")
    entity_config = config.get_editor_property("config")
    traits = entity_config.get_editor_property("traits")
    for i, trait in enumerate(traits):
        print(f"  [{i}] class={trait.get_class().get_name()}")
        if "Visualization" in trait.get_class().get_name():
            try:
                high = trait.get_editor_property("high_res_template_actor")
                low = trait.get_editor_property("low_res_template_actor")
                print(f"      high_res_template_actor={high} low_res_template_actor={low}")
            except Exception as e:
                print(f"      (couldn't read template actor props: {e})")
