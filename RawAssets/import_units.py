import unreal

editor_asset = unreal.EditorAssetLibrary
asset_tools = unreal.AssetToolsHelpers.get_asset_tools()

RAW_ROOT = "C:/Users/Ghazw/Desktop/Project Greyfield/RawAssets/Units"
CONTENT_ROOT = "/Game/Units"

mapping = {
    # Aircraft: import the Blender-merged single-mesh GLBs (raw Interchange import
    # fragmented each source file into 80+ per-material StaticMesh pieces with no
    # working Python-side pipeline option to combine them; Blender joins them into
    # one mesh with multiple material slots instead).
    "Aircraft/CrescentCoalition": ["kaan_merged.glb"],
    "Aircraft/RedDragon": [
        "h-20_merged.glb",
        "j-10-vigorous-dragon_merged.glb",
        "j-16-hidden-dragon_merged.glb",
        "j-20_merged.glb",
        "j-31-gyrfalcon_merged.glb",
        "j-50_merged.glb",
    ],
    "Aircraft/UnitedAlliance": [
        "b-2-spirit_merged.glb",
        "f-15c-eagle_merged.glb",
        "f-16_merged.glb",
        "f-22-raptor_merged.glb",
        "f-35b-lightning-ii_merged.glb",
    ],
    # Infantry are skinned (skeletal) meshes; Interchange already imports these as a
    # single SkeletalMesh + PhysicsAsset with no fragmentation, so use the originals.
    "Infantry/CrescentCoalition": ["crescent-infantry.glb"],
    "Infantry/GlobalLiberationFront": ["glf-infantry.glb"],
    "Infantry/RedDragon": ["red-dragon-infantry.glb"],
    "Infantry/UnitedAlliance": ["united-infantry.glb"],
}

# Clean up previous import attempts (scaffold folders from the manual GUI attempt,
# and the fragmented-mesh import from the first scripted pass that didn't combine
# static mesh parts).
for stray in ["/Game/Units/Aircraft", "/Game/Units/Infantry"]:
    if editor_asset.does_directory_exist(stray):
        editor_asset.delete_directory(stray)
        print("Deleted stray folder:", stray)

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

        asset_tools.import_asset_tasks([task])
        result = task.get_editor_property("imported_object_paths")
        if result:
            imported_paths.extend(list(result))
            print("OK:", src, "->", list(result))
        else:
            failed.append(src)
            print("FAILED:", src)

editor_asset.save_directory("/Game/Units", False, True)

print("=== SUMMARY ===")
print("Imported:", len(imported_paths))
print("Failed:", len(failed))
for f in failed:
    print(" - FAILED:", f)
