import unreal

editor_asset = unreal.EditorAssetLibrary
asset_tools = unreal.AssetToolsHelpers.get_asset_tools()

# Remove the old (broken) f-22-raptor content
old_path = "/Game/Units/Aircraft/UnitedAlliance/f-22-raptor_merged"
if editor_asset.does_directory_exist(old_path):
    editor_asset.delete_directory(old_path)
    print("Deleted old folder:", old_path)

src = "C:/Users/Ghazw/Desktop/Project Greyfield/RawAssets/Units/Aircraft/UnitedAlliance/f22_raptor_new_merged.glb"
dest_path = "/Game/Units/Aircraft/UnitedAlliance/f22_raptor_new_merged"

task = unreal.AssetImportTask()
task.filename = src
task.destination_path = dest_path
task.automated = True
task.save = True
task.replace_existing = True

asset_tools.import_asset_tasks([task])
result = task.get_editor_property("imported_object_paths")
if result:
    print("OK:", src, "->", list(result))
else:
    print("FAILED:", src)

editor_asset.save_directory("/Game/Units", False, True)
