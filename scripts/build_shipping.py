import os
import shutil
import sys
import subprocess


def copy_folder(src, dest):
    """Copy an entire folder to *dest* overwriting existing content."""
    if os.path.exists(dest):
        shutil.rmtree(dest)
    shutil.copytree(src, dest)
    print(f"Copied {src} -> {dest}")


def build_project(msbuild_path, project_path):
    """Build the project using MSBuild."""
    build_command = [msbuild_path, project_path, "/p:Configuration=Release"]
    print(f"Executing: {' '.join(build_command)}")
    try:
        subprocess.check_call(build_command)
        print("Build successful!")
    except subprocess.CalledProcessError:
        print("Build failed.")
        sys.exit(1)


def main():
    if len(sys.argv) < 4:
        print("Error: folder_to_game, output_folder, and solution_dir paths not provided.")
        sys.exit(1)

    folder_to_game = sys.argv[1]
    output_folder = sys.argv[2]
    solution_dir = sys.argv[3]

    msbuild_path = r"C:\Program Files\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin\MSBuild.exe"
    project_path = os.path.join(solution_dir, "Game", "Game.vcxproj")

    # Step 1: Build the project
    build_project(msbuild_path, project_path)

    # Step 2: Copy the asset folder into output/data
    os.makedirs(output_folder, exist_ok=True)
    assets_dest = os.path.join(output_folder, "data")
    copy_folder(folder_to_game, assets_dest)

    # Step 3: Copy only DLLs and the executable from bin/Release next to the executable
    root_dir = os.path.abspath(os.path.join(solution_dir, ".."))
    bin_release_src = os.path.join(root_dir, "bin", "Release")
    if os.path.isdir(bin_release_src):
        for item in os.listdir(bin_release_src):
            src_path = os.path.join(bin_release_src, item)
            if os.path.isfile(src_path) and os.path.splitext(item)[1].lower() in (".dll", ".exe"):
                shutil.copy2(src_path, output_folder)
        print(f"Copied DLLs and executables from {bin_release_src} -> {output_folder}")


if __name__ == "__main__":
    main()

