def can_build(env, platform):
    return not (env["disable_3d"] or env["disable_physics"])


def configure(env):
    pass
