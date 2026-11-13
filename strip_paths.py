# PlatformIO pre-build script: keep local folder names out of the compiled binary.
# Library source paths end up in assert and log messages; these flags rewrite them.
Import("env")

replacements = {
    env.subst("$PROJECT_PACKAGES_DIR"): "/pio/packages",
    env.subst("$PROJECT_DIR"): "/project",
}

flags = []
for local, neutral in replacements.items():
    if not local:
        raise ValueError("strip_paths.py: a build path is empty")
    for form in {local, local.replace("\\", "/")}:
        flags += [f"-ffile-prefix-map={form}={neutral}", f"-fmacro-prefix-map={form}={neutral}"]

env.Append(CCFLAGS=flags)
