def can_build(env, platform):
    return not env.pixel_engine


def configure(env):
    pass


def get_doc_classes():
    return [
        "SceneReplicationConfig",
        "SceneMultiplayer",
        "MultiplayerSpawner",
        "MultiplayerSynchronizer",
        "OfflineMultiplayerPeer",
    ]


def get_doc_path():
    return "doc_classes"
