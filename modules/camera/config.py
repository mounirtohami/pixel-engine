def can_build(env, platform):
    import sys

    if sys.platform.startswith("freebsd"):
        return False
    return not env.pixel_engine and platform in ["macos", "windows", "linuxbsd"]


def configure(env):
    pass
