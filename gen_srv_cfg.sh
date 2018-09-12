#!/bin/bash

host=$(hostname)

cat << EOF > etc/server.cfg
ID=${host//[^0-9]/}
EOF
