import unreal

MAT_PATH = "/Game/MWLandscapeAutoMaterial/Materials/Landscape/MTL_MWAM_Landscape_MountainRangeExample.MTL_MWAM_Landscape_MountainRangeExample"
mat = unreal.EditorAssetLibrary.load_asset(MAT_PATH)
print("Loaded:", mat)
print("class:", mat.get_class().get_name())

# If it's a Material Instance, get its parent
if hasattr(mat, "parent"):
    try:
        parent = mat.get_editor_property("parent")
        print("parent:", parent.get_name() if parent else None)
    except Exception as e:
        print("parent read failed:", e)

# Try to get the base UMaterial and its shading model / properties
base_material = mat.get_base_material() if hasattr(mat, "get_base_material") else None
print("base_material:", base_material.get_name() if base_material else None)
if base_material:
    for prop in ["shading_model", "two_sided", "use_material_attributes", "d3d11_tessellation_mode",
                 "material_domain", "blend_mode"]:
        try:
            print(f"  base.{prop}:", base_material.get_editor_property(prop))
        except Exception as e:
            print(f"  base.{prop}: <not found>")

print("=== END ===")
