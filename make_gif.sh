#!/bin/sh
ffmpeg -y -i output/%d.png -vf palettegen output/palette.png
ffmpeg -y -i output/%d.png -c:v ffv1 -qscale:v 0 output.avi
ffmpeg -y -framerate $2 -i output/%d.png -i output/palette.png -lavfi paletteuse $1/output.gif