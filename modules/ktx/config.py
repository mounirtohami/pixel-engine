def can_build(env, platform):
    env.module_add_dependencies("ktx", ["basis_universal"])
    return not env.pixel_engine


def configure(env):
    pass
