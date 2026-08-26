"""
Imports a curated subset of the "Pistol and Rifle Locomotion Animations 1700"
(Kingboars, Fab) pack into Content/Animations/GaspFix.

Source pack ships the UEFN Mannequin skeleton (Characters/UEFN_Mannequin/Meshes/
SKM_UEFN_Mannequin.FBX) plus ~1700 per-clip FBX files, each with the full
character mesh baked in alongside the animation (that's why the pack is 267MB
zipped for what should be tiny mocap curves). We only want the animation data,
bound to one shared skeleton -- so every clip import uses
FbxImportUI.import_mesh = False + mesh_type_to_import = FBXIT_ANIMATION,
which discards the baked-in mesh and keeps only the keyframes.

Run via:
  UnrealEditor-Cmd.exe "<uproject>" -ExecutePythonScript="RawAssets/import_gasp_locomotion.py"
with the editor closed first (standard project convention -- avoid two
processes touching the same project simultaneously).
"""

import os
import unreal

SOURCE_ROOT = r"C:\ProgramData\Epic\EpicGamesLauncher\VaultCache\FabLibrary\Pistol_and_Rifle_Locomotion_Animations_1700-509f8c0a\fbx\gaspfix_extracted\GaspFix"
SKELETON_MESH_SRC = os.path.join(SOURCE_ROOT, "Characters", "UEFN_Mannequin", "Meshes", "SKM_UEFN_Mannequin.FBX")
RIFLE_ROOT = os.path.join(SOURCE_ROOT, "_FixedRifle")

DEST_ROOT = "/Game/Animations/GaspFix"
MESH_DEST = f"{DEST_ROOT}/UEFN_Mannequin"

# Curated starter set: full 8-directional walk/run loops + idle/turn/sprint.
# Skips the hundreds of arc/box/transition combinatorial variants -- those
# exist in the pack if we ever want them, this is enough for a real 8-way
# blend space without bloating the project.
CLIPS = {
    "Idle": [
        "M_Neutral_Stand_Idle_Loop_Rifle",
        "M_Neutral_Crouch_Idle_Loop_Rifle",
        "M_Neutral_Idle_turn_left_Rifle",
        "M_Neutral_Idle_turn_right_Rifle",
    ],
    "Walk": [
        "M_Neutral_Walk_Loop_F_Rifle",
        "M_Neutral_Walk_Loop_B_Rifle",
        "M_Neutral_Walk_Loop_FL_Rifle",
        "M_Neutral_Walk_Loop_FR_Rifle",
        "M_Neutral_Walk_Loop_BL_Rifle",
        "M_Neutral_Walk_Loop_BR_Rifle",
        "M_Neutral_Walk_Loop_LL_Rifle",
        "M_Neutral_Walk_Loop_RR_Rifle",
    ],
    "Run": [
        "M_Neutral_Run_Loop_F_Rifle",
        "M_Neutral_Run_Loop_B_Rifle",
        "M_Neutral_Run_Loop_FL_Rifle",
        "M_Neutral_Run_Loop_FR_Rifle",
        "M_Neutral_Run_Loop_BL_Rifle",
        "M_Neutral_Run_Loop_BR_Rifle",
        "M_Neutral_Run_Loop_LL_Rifle",
        "M_Neutral_Run_Loop_RR_Rifle",
    ],
    "Sprint": [
        "M_Neutral_Sprint_Loop_F_Rifle",
    ],
}


def log(msg):
    unreal.log(msg)
    print(msg)


def import_skeleton_mesh():
    asset_tools = unreal.AssetToolsHelpers.get_asset_tools()

    task = unreal.AssetImportTask()
    task.filename = SKELETON_MESH_SRC
    task.destination_path = MESH_DEST
    task.destination_name = "SKM_UEFN_Mannequin"
    task.replace_existing = True
    task.automated = True
    task.save = True

    options = unreal.FbxImportUI()
    options.import_mesh = True
    options.import_animations = False
    options.import_materials = False
    options.import_textures = False
    options.import_as_skeletal = True
    options.create_physics_asset = False
    options.mesh_type_to_import = unreal.FBXImportType.FBXIT_SKELETAL_MESH

    task.options = options
    asset_tools.import_asset_tasks([task])

    mesh_path = f"{MESH_DEST}/SKM_UEFN_Mannequin"
    if not unreal.EditorAssetLibrary.does_asset_exist(mesh_path):
        raise RuntimeError(f"Skeletal mesh import failed, not found at {mesh_path}")

    # Standard UE naming: importing a mesh with no existing skeleton creates
    # a companion Skeleton asset named "<mesh>_Skeleton" alongside it.
    skeleton_path = f"{MESH_DEST}/SKM_UEFN_Mannequin_Skeleton"
    if unreal.EditorAssetLibrary.does_asset_exist(skeleton_path):
        return skeleton_path

    # Fallback: scan the destination folder for whatever Skeleton asset got created.
    for path in unreal.EditorAssetLibrary.list_assets(MESH_DEST, recursive=False):
        data = unreal.EditorAssetLibrary.find_asset_data(path)
        if data.get_class().get_name() == "Skeleton":
            return path.split(".")[0] if "." in path else path

    raise RuntimeError("Could not locate companion Skeleton asset after mesh import")


def import_animation_clips(skeleton_path):
    skeleton = unreal.load_asset(skeleton_path)
    if skeleton is None:
        raise RuntimeError(f"Could not load skeleton at {skeleton_path}")

    asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
    results = []

    for category, names in CLIPS.items():
        dest = f"{DEST_ROOT}/{category}"
        for name in names:
            src = os.path.join(RIFLE_ROOT, category, name + ".FBX")
            if not os.path.isfile(src):
                results.append(f"MISSING SOURCE: {src}")
                continue

            task = unreal.AssetImportTask()
            task.filename = src
            task.destination_path = dest
            task.destination_name = name
            task.replace_existing = True
            task.automated = True
            task.save = True

            options = unreal.FbxImportUI()
            options.import_mesh = False
            options.import_animations = True
            options.import_materials = False
            options.import_textures = False
            options.skeleton = skeleton
            options.mesh_type_to_import = unreal.FBXImportType.FBXIT_ANIMATION

            task.options = options
            asset_tools.import_asset_tasks([task])

            anim_path = f"{dest}/{name}"
            ok = unreal.EditorAssetLibrary.does_asset_exist(anim_path)
            results.append(f"{'OK' if ok else 'FAILED'}: {anim_path}")

    return results


def run():
    log("=== GaspFix locomotion import: starting ===")
    skeleton_path = import_skeleton_mesh()
    log(f"Skeleton ready at {skeleton_path}")

    results = import_animation_clips(skeleton_path)
    for line in results:
        log(line)

    ok_count = sum(1 for r in results if r.startswith("OK"))
    fail_count = len(results) - ok_count
    log(f"=== GaspFix locomotion import: done. {ok_count} ok, {fail_count} failed/missing ===")


run()
