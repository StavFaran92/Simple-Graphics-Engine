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


def clean_output_folder(output_folder, protected_paths):
    """Delete and recreate *output_folder*, refusing paths that would wipe sources."""
    output = os.path.normcase(os.path.realpath(output_folder))

    unsafe = (
        output == os.path.normcase(os.path.realpath(os.path.splitdrive(output)[0] + os.sep))  # drive root
        or os.path.isdir(os.path.join(output, ".git"))
    )
    for path in protected_paths:
        protected = os.path.normcase(os.path.realpath(path))
        # Output must not be (or contain) the solution dir or the game content
        if protected == output or protected.startswith(output + os.sep):
            unsafe = True

    if unsafe:
        print(f"Error: refusing to clean unsafe output folder: {output_folder}")
        sys.exit(1)

    if os.path.exists(output_folder):
        shutil.rmtree(output_folder)
        print(f"Cleaned {output_folder}")
    os.makedirs(output_folder)


def find_msbuild():
    """Locate MSBuild.exe of the latest Visual Studio install (MSBUILD_PATH env var overrides)."""
    env_path = os.environ.get("MSBUILD_PATH")
    if env_path and os.path.isfile(env_path):
        return env_path

    program_files_x86 = os.environ.get("ProgramFiles(x86)", r"C:\Program Files (x86)")
    vswhere = os.path.join(program_files_x86, "Microsoft Visual Studio", "Installer", "vswhere.exe")
    if os.path.isfile(vswhere):
        try:
            output = subprocess.check_output(
                [vswhere, "-latest", "-prerelease", "-products", "*",
                 "-requires", "Microsoft.Component.MSBuild",
                 "-find", r"MSBuild\**\Bin\MSBuild.exe"],
                text=True)
            for line in output.splitlines():
                if line.strip() and os.path.isfile(line.strip()):
                    return line.strip()
        except subprocess.CalledProcessError:
            pass

    msbuild = shutil.which("MSBuild.exe")
    if msbuild:
        return msbuild

    print("Error: could not locate MSBuild.exe. Install Visual Studio or set MSBUILD_PATH.")
    sys.exit(1)


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

    msbuild_path = find_msbuild()
    project_path = os.path.join(solution_dir, "Game", "Game.vcxproj")

    # Step 1: Build the project
    build_project(msbuild_path, project_path)

    bin_release_src = os.path.join(solution_dir, "bin", "Release")
    if not os.path.isfile(os.path.join(bin_release_src, "Game.exe")):
        print(f"Error: Game.exe was not found in {bin_release_src}, refusing to ship a stale build.")
        sys.exit(1)

    # Step 2: Start from an empty output folder so no stale files survive
    clean_output_folder(output_folder, [solution_dir, folder_to_game])

    # Step 3: Copy the asset folder into output/data
    assets_dest = os.path.join(output_folder, "data")
    copy_folder(folder_to_game, assets_dest)

    # Step 4: Copy only DLLs and the executable from bin/Release next to the executable
    if os.path.isdir(bin_release_src):
        for item in os.listdir(bin_release_src):
            src_path = os.path.join(bin_release_src, item)
            if item.lower() == "editorapp.exe":
                continue  # editor is not part of the shipped game
            if os.path.isfile(src_path) and os.path.splitext(item)[1].lower() in (".dll", ".exe"):
                shutil.copy2(src_path, output_folder)
        print(f"Copied DLLs and executables from {bin_release_src} -> {output_folder}")


if __name__ == "__main__":
    main()

