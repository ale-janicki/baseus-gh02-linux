#!/bin/sh
# Remove everything install.sh put on the system.
set -e
[ "$(id -u)" = 0 ] || { echo "run with sudo" >&2; exit 1; }
systemctl disable gh02-fix.service 2>/dev/null || true
rm -f /etc/systemd/system/gh02-fix.service /usr/local/sbin/gh02-reenumerate /etc/modules-load.d/gh02-fix.conf
systemctl daemon-reload
modprobe -r gh02_fix 2>/dev/null || true
dkms remove gh02-fix/1.0 --all 2>/dev/null || true
rm -rf /usr/src/gh02-fix-1.0
echo "gh02-fix removed"
