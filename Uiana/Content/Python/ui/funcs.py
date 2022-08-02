import os
import re
import requests
from pathlib import Path
from ..utils.common import setup_logger

# def setup_logger(logfile: str) -> logging.Logger:
# logger = logging.getLogger("LV")

def get_umap_list(context, map_name) -> list:
    maps_data = requests.get("https://gist.githubusercontent.com/djhaled/a09847dd39b48b7aff9480f8daebe313/raw/85ee6e756ee2b1dbdeabde47d825e4f2964ed3b7/umaps.json").json()
    return maps_data[map_name]

logger = setup_logger(__name__)


def search_for_valorant() -> str:
    """
    Looks for the game path
    Returns:
        str: Path to the game
    """

    val_yaml = os.path.expandvars(r'%ProgramData%\Riot Games\Metadata\valorant.live\valorant.live.product_settings.yaml')

    with open(val_yaml, "r") as f:
        try:
            yo = f.readlines()
            for line in yo:
                if "product_install_full_path" in line:
                    l = line.rstrip('\n')
                    game_path = re.search('"(.*)"', l).group(1)
                    if os.path.exists(game_path):
                        logger.info(f"Found VALORANT : {game_path}")
                        return game_path
        except Exception as exc:
            return False


def is_valid_valorant_path(path: str) -> bool:
    """
    Checks if the path is valid
    Args:
        path (str): Path to the game
    """

    # print(path)
    path = Path(path)

    if path.exists() and path.is_dir():
        if has_paks(path):
            logger.info("Folder contains .pak files")
            if is_valorant_updated(path):
                logger.info("Valorant is up to date")
                return True
            else:
                return False
        else:

            return False
    else:
        logger.critical("Folder doesn't exists!")
        return False


def has_paks(path: str) -> bool:
    """
    Checks if the folder has paks
    """

    path = Path(path)

    try:
        if any(File.endswith(".pak") for File in os.listdir(path)):
            return True
    except Exception as exc:
        logger.critical("Folder doesn't contains .pak files! : %s" % exc)
        return False


def is_valorant_updated(path: str) -> bool:
    """
    Checks if the game is up to date
    Args:
        game_path (str): Path to the game
    """

    path = Path(path)

    game_path = path.parent.parent.joinpath("Binaries", "Win64", "VALORANT-Win64-Shipping.exe")

    live_val_version = get_latest_version()
    local_valorant_version = get_exec_version(game_path)

    if local_valorant_version == live_val_version:
        logger.info("VALORANT is up to date")
        return True
    else:
        logger.critical("Live VALORANT Version : %s" % live_val_version)
        logger.critical("Local VALORANT Version : %s" % local_valorant_version)
        return False

# Getters


def get_latest_version() -> str:
    """
    Returns the live version of the game
    """

    return requests.get("https://valorant-api.com/v1/version").json()["data"]["riotClientVersion"]


def get_exec_version(path: str) -> str:
    """
    Returns the version of the game
    """

    path = Path(path)

    with open(path.__str__(), 'rb') as game_bin:
        data = game_bin.read()
        pattern = '++Ares-Core+'.encode('utf-16-le')
        pos = data.find(pattern) + len(pattern)
        branch, build_date, build_version, version, *_ = filter(None, data[pos:pos+96].decode('utf-16-le').split('\x00'))
        # return f"{branch}.{int(version[-6:])}"
        local_valorant_version = '%s%s-%s-%d' % (branch, '-shipping', build_version, int(version[-6:]))    # just to follow what the game does
        return local_valorant_version
