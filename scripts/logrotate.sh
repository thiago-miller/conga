#!/usr/bin/env bash

set -euo pipefail

log="$1"
max="${2:-10}"

if [ -z "$log" ]; then
	echo "Usage: $0 <logfile> [max_rotations]"
	exit 1
fi

for ((i=max-1; i>=1; i--)); do
	if [ -e "$log.$i" ]; then
		mv "$log.$i" "$log.$((i+1))"
	fi
done

if [ -e "$log" ]; then
	mv "$log" "$log.1"
fi
