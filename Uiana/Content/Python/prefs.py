
from .utils.common import setup_logger
from .ui.funcs import has_paks

import os
import requests

logger = setup_logger(__name__)


def get_map_list():
    maps_data = requests.get(
        "https://gist.githubusercontent.com/djhaled/a09847dd39b48b7aff9480f8daebe313/raw/85ee6e756ee2b1dbdeabde47d825e4f2964ed3b7/umaps.json").json()

    maps: list = []
    name: str
    n: int = 0
    for name, value in maps_data.items():
        maps.append(
            (name, name.capitalize(), "", "", n)
        )
        n += 1

    return maps



