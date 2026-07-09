#!/usr/bin/env python3
"""Generates the laser glow gradient (textures/glow.png): bright along the
centerline, fading across the beam and with distance from the nozzle."""
import math
import os

from PIL import Image

HERE = os.path.dirname(os.path.abspath(__file__))
W, H = 768, 160
SIGMA = 0.20        # radial (across) falloff
ALONG_DECAY = 2.6   # lengthwise fade rate
CORE = 0.06         # bright thin core half-width (in v units)


def main():
    img = Image.new("L", (W, H))
    px = img.load()
    for x in range(W):
        u = x / (W - 1)
        along = math.exp(-ALONG_DECAY * u)
        for y in range(H):
            v = y / (H - 1)
            d = abs(v - 0.5)
            across = math.exp(-((d / SIGMA) ** 2))
            # A brighter narrow core along the centerline.
            if d < CORE:
                across = max(across, 1.0 - (d / CORE) * 0.25)
            val = along * across
            px[x, y] = int(max(0.0, min(1.0, val)) * 255)
    os.makedirs(os.path.join(HERE, "textures"), exist_ok=True)
    img.save(os.path.join(HERE, "textures", "glow.png"))
    print("wrote", os.path.join(HERE, "textures", "glow.png"))


if __name__ == "__main__":
    main()
