#! /bin/sh

MODULE="gasdev0.0"
MAJOR=$(awk "\$2==\"$MODULE\" {print \$1}" /proc/devices)

mknod /dev/$MODULE c $MAJOR 0

