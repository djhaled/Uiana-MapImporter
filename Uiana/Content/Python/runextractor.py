import unreal
import argparse
import subprocess
import os
parser = argparse.ArgumentParser()
parser.add_argument('PakFolder',type=str,help='PakFolder')
parser.add_argument('GameVersion',type=str,help='GameVerison')
parser.add_argument('aesk',type=str,help='aesk')
parser.add_argument('PluginPath',type=str,help='PluginPath')
args = parser.parse_args()
exepath = args.PluginPath + "Uiana/Content/Python/tools/CUE4UmapsExtractor.exe"
args = [exepath.__str__(),
"--paks-directory", args.PakFolder.__str__(),
"--game", args.GameVersion,
"--ppath", args.PluginPath,
"--aes-key", args.aesk]
print(args)
subprocess.call(args)

