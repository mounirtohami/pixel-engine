def can_build(env, platform):
    return not env.pixel_engine


def configure(env):
    pass


def get_doc_classes():
    return [
        "WebSocketClient",
        "WebSocketMultiplayerPeer",
        "WebSocketPeer",
        "WebSocketServer",
    ]


def get_doc_path():
    return "doc_classes"
