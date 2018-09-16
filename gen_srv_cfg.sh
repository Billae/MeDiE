#!/bin/bash

host=$(hostname)

cat << EOF > prototype_MDS/etc/server.cfg
ID=${host//[^0-9]/}
EOF
