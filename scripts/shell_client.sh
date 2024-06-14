#!/bin/bash
set -e
A="${1-127.0.0.1:8888}"
F="/tmp/.shell_client_in.$RANDOM"
L="/tmp/.shell_client_lock.$RANDOM"

touch "$L"
(while test -f "$L"; do
    curl -sSLN -X GET "$A/read?id=&side=c" -H "Accept: application/octet-stream"
done) &

stty -icanon -echo
while true; do
    dd bs=1024 count=1 of="$F" 2>/dev/null
    [ -s "$F" ] || break
    curl -sSL -X POST "$A/write?id=&side=c" -H "Content-Type: application/octet-stream" --data-binary "@$F" > /dev/null
done
stty icanon echo

rm -f "$F" "$L"
wait
