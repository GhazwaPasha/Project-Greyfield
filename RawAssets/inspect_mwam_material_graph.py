import unreal

MASTER_PATH = "/Game/MWLandscapeAutoMaterial/Materials/MASTER/MTL_MWAM_AutoMaterial_MASTER.MTL_MWAM_AutoMaterial_MASTER"
mat = unreal.EditorAssetLibrary.load_asset(MASTER_PATH)
print("Loaded master:", mat)

expr_types = {}
try:
    exprs = unreal.MaterialEditingLibrary.get_material_expressions(mat)
    print("total expressions:", len(exprs))
    for e in exprs:
        cls = e.get_class().get_name()
        expr_types[cls] = expr_types.get(cls, 0) + 1
    layer_related = {k: v for k, v in expr_types.items() if "Landscape" in k}
    print("Landscape-related expression node types and counts:", layer_related)
    normal_related = {k: v for k, v in expr_types.items() if "Normal" in k}
    print("Normal-related expression node types and counts:", normal_related)
except Exception as e:
    print("get_material_expressions failed:", e)

print("=== END ===")
