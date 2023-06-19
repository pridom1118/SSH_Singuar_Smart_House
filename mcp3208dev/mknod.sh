#! /bin/sh

MODULE="mcp3208dev0.0" # [name}{master}.{chip}
MAJOR=$(awk "\$2==\"$MODULE\" {print \$1}" /proc/devices)

mknod /dev/$MODULE c $MAJOR 0
