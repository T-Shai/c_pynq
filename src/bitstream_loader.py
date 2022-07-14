from pynq import Overlay
from pynq.overlays.base import BaseOverlay

import sys
if len(sys.argv) < 2:
    print("usage")
    exit()

bitstream_name = sys.argv[1]

print("loading overlay...", bitstream_name)

if bitstream_name == "base.bit":
    BaseOverlay(bitstream_name)
else :
    Overlay(sys.argv[1])

print("done")