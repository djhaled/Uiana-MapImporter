import time
import os
import json
from pathlib import Path
import shutil
Version = 0
with open("Uiana/Uiana.uplugin", "r") as jsonFile:
	data = json.load(jsonFile)
	data["VersionName"] = float(data["VersionName"]) + 0.00
	Version = data["VersionName"]

with open("Uiana/Uiana.uplugin", "w") as jsonFile:
	json.dump(data, jsonFile,indent=4)

#### git
cwd = Path(os.getcwd())
addon_filename = "Uiana" + '-' + str(Version)
shutil.make_archive(base_name=addon_filename,format="zip",root_dir=cwd.__str__(),base_dir="Uiana")




