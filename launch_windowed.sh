#! /bin/bash

if [ -z $1 ] || [ -z $2 ]
then
    echo "please give a number of servers and clients"
    exit 1
fi

nb_srv=$1
nb_client=$2
total_vm=$(($1+$2))

#traces: 5 mins
total_step=292
traces_path=/mnt/scratch/traces/5min/12_clients/changelog
#traces_path=/mnt/scratch/traces/generated/_____/traces

#config clush groups
sudo sh -c "echo \"srv: vm[0-$(($nb_srv-1))]
client: vm[$nb_srv-$(($total_vm-1))]\">/etc/clustershell/groups"

# prepare trace files
#python setup.py /mnt/scratch/traces/changelog.csv > /mnt/scratch/traces/setup-changelog.csv
#python split.py /mnt/scratch/traces/changelog.csv 10
#for each step file:
#python spread.py /mnt/scratch/traces/changelog-0.csv $(($nb_client))

#clean perf folder before testing
rm /mnt/result/windowed/server/*
rm /mnt/result/windowed/client/*
rm /mnt/result/windowed/*
#clean tmp folder
rm /mnt/scratch/tmp_ack/windowed/*

#launch servers and manager
clush -w @srv -b  ./prototype_MDS/gen_srv_cfg.sh
clush -w vm0 -b ./prototype_MDS/bin/manager $(($nb_srv))&
clush -w @srv -b ./prototype_MDS/bin/server $(($nb_srv)) p&
sleep 5

#prepare servers for traces
#python36 spread.py /mnt/scratch/traces/setup-changelog.csv $(($nb_client))
clush -w @client -b ./prototype_MDS/client_launch.sh $(($nb_srv)) /mnt/scratch/traces/setup-changelog /mnt/result/windowed

printf "setup finished\n"

current_step=0
while [[ $current_step -lt $total_step ]]
do
    #launch a step of traces
    clush -w @client -b ./prototype_MDS/client_launch.sh $(($nb_srv)) $traces_path-$current_step /mnt/result/windowed
    ((current_step++))

    #mesuring and rebalancing if needed
    clush -w @srv 'kill -s SIGUSR2 `/usr/sbin/pidof ./prototype_MDS/bin/server`'
    clush -w vm0 'kill -s SIGUSR2 `/usr/sbin/pidof ./prototype_MDS/bin/manager`'

    ./prototype_MDS/protocol_test/synchro.bash $(($nb_srv)) $current_step windowed
    rm /mnt/scratch/tmp_ack/windowed/*USR-*
    printf "step $current_step finished\n"
done
