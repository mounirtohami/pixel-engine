def can_build(env, platform):
    return not env.pixel_engine


def configure(env):
    pass


def get_doc_classes():
    return [
        "OggPacketSequence",
        "OggPacketSequencePlayback",
    ]


def get_doc_path():
    return "doc_classes"
