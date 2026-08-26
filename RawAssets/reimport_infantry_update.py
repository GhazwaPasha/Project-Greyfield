import unreal

editor_asset = unreal.EditorAssetLibrary
asset_tools = unreal.AssetToolsHelpers.get_asset_tools()

RAW_ROOT = "C:/Users/Ghazw/Desktop/Project Greyfield/RawAssets/Units"
CONTENT_ROOT = "/Game/Units"

# Targeted replacement, 2026-08-25: new infantry models supplied for United
# Alliance, Crescent Coalition, and Red Dragon (GLF unchanged, no new model
# supplied). Deliberately NOT touching Aircraft and NOT deleting destination
# folders first (unlike the original import_units.py) -- reimporting to the
# same destination_path with replace_existing=True updates the existing
# SkeletalMesh/PhysicsAsset in place so anything already referencing it
# (placed actors, Mass configs, Blueprints) keeps pointing at a valid asset
# instead of being orphaned by a delete+recreate.
mapping = {
    "Infantry/CrescentCoalition": ["crescent-infantry.glb"],
    "Infantry/RedDragon": ["red-dragon-infantry.glb"],
    "Infantry/UnitedAlliance": ["united-infantry.glb"],
}

imported_paths = []
failed = []

for rel_dir, files in mapping.items():
    dest_path = "{}/{}".format(CONTENT_ROOT, rel_dir)
    for fname in files:
        src = "{}/{}/{}".format(RAW_ROOT, rel_dir, fname)
        task = unreal.AssetImportTask()
        task.filename = src
        task.destination_path = dest_path
        task.automated = True
        task.save = True
        task.replace_existing = True
        task.replace_existing_settings = True

        asset_tools.import_asset_tasks([task])
        result = task.get_editor_property("imported_object_paths")
        if result:
            imported_paths.extend(list(result))
            print("OK:", src, "->", list(result))
        else:
            failed.append(src)
            print("FAILED:", src)

editor_asset.save_directory("/Game/Units/Infantry", False, True)

print("=== SUMMARY ===")
print("Imported:", len(imported_paths))
print("Failed:", len(failed))
for f in failed:
    print(" - FAILED:", f)
