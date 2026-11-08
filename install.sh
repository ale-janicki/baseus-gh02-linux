#!/bin/sh
set -e
[ "$(id -u)" = 0 ] || { echo "run with sudo" >&2; exit 1; }
command -v dkms >/dev/null || { echo "dkms not found: sudo apt install dkms build-essential linux-headers-$(uname -r)" >&2; exit 1; }
SRC=$(cd "$(dirname "$0")" && pwd)
VER=1.0
DST=/usr/src/gh02-fix-$VER

dkms remove gh02-fix/$VER --all >/dev/null 2>&1 || true
rm -rf "$DST"
mkdir -p "$DST"
cp "$SRC/gh02_fix.c" "$SRC/gh02_cfg291.h" "$SRC/Makefile" "$SRC/dkms.conf" "$DST/"
dkms add gh02-fix/$VER
for k in /lib/modules/*; do
	k=$(basename "$k")
	[ -e "/lib/modules/$k/build" ] || { echo "skip $k (no headers)"; continue; }
	dkms install gh02-fix/$VER -k "$k"
done

install -m 0644 "$SRC/gh02-fix.modules-load.conf" /etc/modules-load.d/gh02-fix.conf
install -m 0755 "$SRC/gh02-reenumerate" /usr/local/sbin/gh02-reenumerate
install -m 0644 "$SRC/gh02-fix.service" /etc/systemd/system/gh02-fix.service
systemctl daemon-reload
systemctl enable gh02-fix.service

modprobe gh02_fix
/usr/local/sbin/gh02-reenumerate

dkms status gh02-fix
echo "gh02-fix installed and loaded"
