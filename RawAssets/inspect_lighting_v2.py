import unreal

level_subsys = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
actor_subsys = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)

def dump_pp_settings(settings):
    # Exposure
    fields = [
        "auto_exposure_method", "auto_exposure_min_brightness", "auto_exposure_max_brightness",
        "auto_exposure_bias", "auto_exposure_speed_up", "auto_exposure_speed_down",
        "auto_exposure_low_percent", "auto_exposure_high_percent",
        "histogram_log_min", "histogram_log_max",
    ]
    for f in fields:
        override_f = "override_" + f
        try:
            has_override = settings.get_editor_property(override_f)
        except Exception:
            has_override = "n/a"
        try:
            val = settings.get_editor_property(f)
        except Exception:
            val = "n/a"
        print(f"    {f}: override={has_override} value={val}")

def inspect_level(path):
    print(f"\n========== {path} ==========")
    level_subsys.load_level(path)
    actors = actor_subsys.get_all_level_actors()
    found_any_pp = False
    for a in actors:
        cls_name = a.get_class().get_name()
        if any(k in cls_name for k in ["DirectionalLight", "SkyLight", "SkyAtmosphere", "ExponentialHeightFog", "PostProcessVolume"]):
            print("---", cls_name, "-", a.get_actor_label(), "---")
            root = a.get_editor_property("root_component")
            if root:
                print("  root mobility:", root.get_editor_property("mobility"))
            if "DirectionalLight" in cls_name:
                comp = a.get_component_by_class(unreal.DirectionalLightComponent)
                if comp:
                    print("  intensity:", comp.get_editor_property("intensity"))
                    print("  mobility:", comp.get_editor_property("mobility"))
                    print("  atmosphere_sun_light:", comp.get_editor_property("atmosphere_sun_light"))
                    print("  cast_shadows:", comp.get_editor_property("cast_shadows"))
                    print("  visible:", comp.get_editor_property("visible"))
            elif "SkyLight" in cls_name:
                comp = a.get_component_by_class(unreal.SkyLightComponent)
                if comp:
                    print("  intensity:", comp.get_editor_property("intensity"))
                    print("  real_time_capture:", comp.get_editor_property("real_time_capture"))
                    print("  source_type:", comp.get_editor_property("source_type"))
                    print("  visible:", comp.get_editor_property("visible"))
            elif "PostProcessVolume" in cls_name:
                found_any_pp = True
                print("  unbound:", a.get_editor_property("unbound"))
                print("  priority:", a.get_editor_property("priority"))
                print("  blend_weight:", a.get_editor_property("blend_weight"))
                settings = a.get_editor_property("settings")
                dump_pp_settings(settings)
            elif "ExponentialHeightFog" in cls_name:
                comp = a.get_component_by_class(unreal.ExponentialHeightFogComponent)
                if comp:
                    print("  fog_density:", comp.get_editor_property("fog_density"))
                    print("  visible:", comp.get_editor_property("visible"))
    if not found_any_pp:
        print("  !!! NO PostProcessVolume actor found in this level !!!")

inspect_level("/Game/Maps/NewMap")
inspect_level("/Game/Maps/Map_Small2v2")

print("=== END ===")
