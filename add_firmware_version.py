#!python3

import subprocess
import os
import shutil

def determine_version(branch):
    if branch == "master":
        try:
            return get_git_output("git describe --abbrev=0")
        except subprocess.CalledProcessError:
            return "UNKNOWN"
    else:
        return "DEV"

def get_git_output(command):
    try:
        return subprocess.check_output(command, shell=True, text=True, stderr=subprocess.DEVNULL).strip()
    except subprocess.CalledProcessError:
        return "UNKNOWN"

commit = get_git_output("git describe --match=NeVeRmAtCh --always --abbrev=7")
date = get_git_output(f"git show -s --format=%cd --date=format:%y_%m_%d {commit}")
branch = get_git_output("git rev-parse --abbrev-ref HEAD")
# ver = get_git_output("git describe --abbrev=0")
ver = determine_version(branch)

with open("./include/version.hpp", "w") as f:
    f.write(f'#define FW_VER "{ver}"\n')
    f.write(f'#define COMMIT_NUMBER "{commit}"\n')
    f.write(f'#define COMMIT_DATE "{date}"\n')
    f.write(f'#define COMMIT_BRANCH "{branch}"\n')
    f.write(f'#define FIRMWARE_VERSION "{ver}-{commit}-{date}"\n')

print(f"version:{ver} Commit:{commit} Branch:{branch} Date:{date}")
