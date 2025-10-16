from SCons.Script import (COMMAND_LINE_TARGETS, AlwaysBuild,
                          Builder, Default, DefaultEnvironment)
from os.path import join
Import("env")

platform = env.PioPlatform()

env.AutodetectUploadPort()

def get_esptoolpy_reset_flags(resetmethod):
    resets = ("no_reset_no_sync", "soft_reset")
    if resetmethod == "nodemcu":
        resets = ("default_reset", "hard_reset")
    elif resetmethod == "ck":
        resets = ("no_reset", "soft_reset")

    return ["--before", resets[0], "--after", resets[1]]

env.Replace(
    UPLOADER=join(
        platform.get_package_dir("tool-esptoolpy") or "", "esptool.py"),
    UPLOADERFLAGS=[
        "--chip", "esp8266",
        "--port", "$UPLOAD_PORT",
        "--baud", "$UPLOAD_SPEED",
        "write_flash",
        "0x1000", "$SOURCE"  # Example address, change as necessary
    ],
    UPLOADCMD='"$PYTHONEXE" "$UPLOADER" $UPLOADERFLAGS'
)
for image in env.get("FLASH_EXTRA_IMAGES", []):
    env.Append(UPLOADERFLAGS=[image[0], env.subst(image[1])])

if "uploadfs" in COMMAND_LINE_TARGETS:
    env.Replace(
        UPLOADERFLAGS=[
            "--chip", "esp8266",
            "--port", "$UPLOAD_PORT",
            "--baud", "$UPLOAD_SPEED",
            "write_flash",
            "$FS_START", "$SOURCE"
        ],
        UPLOADCMD='"$PYTHONEXE" "$UPLOADER" $UPLOADERFLAGS'
    )

env.Prepend(
    UPLOADERFLAGS=get_esptoolpy_reset_flags(env.subst("$UPLOAD_RESETMETHOD"))
)

upload_actions = [
    env.VerboseAction(env.AutodetectUploadPort,
                      "Looking for upload port..."),
    env.VerboseAction("$UPLOADCMD", "Uploading $SOURCE")
]
