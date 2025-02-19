#!/usr/bin/env bash
set -Eeuo pipefail

trap "trap - SIGTERM && kill -- -$$" SIGINT SIGTERM EXIT

if [ ! -f "/etc/strfry.conf" ]; then
  cp /etc/strfry.conf.default /etc/strfry.conf
fi

cd /app
./strfry relay &
PID=$!

: ${STREAMS:=''}

for i in $(echo $STREAMS | sed "s/,/ /g")
do
    if [[ -n "$i" ]]; then
        sleep 2
        ./strfry stream wss://$i --dir down 2> /dev/null &
    fi
done

wait $PID
