def can_build(env, platform):
    return not env.pixel_engine


def get_opts(platform):
    from SCons.Variables import BoolVariable

    return [
        BoolVariable("minimp3_extra_formats", "Build minimp3 with MP1/MP2 decoding support", False),
    ]


def configure(env):
    pass


def get_doc_classes():
    return [
        "AudioStreamMP3",
        "ResourceImporterMP3",
    ]


def get_doc_path():
    return "doc_classes"
