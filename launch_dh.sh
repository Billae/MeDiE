#! /bin/bash

if [ -z $1 ] || [ -z $2 ]
then
    echo "please give a number of servers and clients"
    exit 1
fi

nb_srv=$1
nb_client=$2
total_vm=$(($1+$2))

#traces: mesuring 5min, rebalancing every hours
total_step=292
rebalance=30
traces_path=/mnt/scratch/traces/5min/12_clients/changelog
#traces_path=/mnt/scratch/traces/generated/_____/traces

#config clush groups
sudo sh -c "echo \"srv: vm[0-$(($nb_srv-1))]
client: vm[$nb_srv-$(($total_vm-1))]\">/etc/clustershell/groups"

# prepare trace files
#python setup.py /mnt/scratch/traces/changelog.csv > /mnt/result/traces/setup-changelog.csv
#python split.py /mnt/scratch/traces/changelog.csv 5
#for all step do:
#python spread.py /mnt/scratch/traces/changelog-0.csv $(($nb_client))

#clean perf folder before testing
rm /mnt/result/dh/server/*
rm /mnt/result/dh/client/*
rm /mnt/result/dh/*
#clean tmp folder
rm /mnt/scratch/tmp_ack/dh/*

#launch servers and manager
clush -w @srv -b  ./prototype_MDS/gen_srv_cfg.sh
clush -w vm0 -b ./prototype_MDS/bin/manager $(($nb_srv))&
clush -w @srv -b ./prototype_MDS/bin/server $(($nb_srv)) p&
sleep 5

#prepare servers for traces
#python36 spread.py /mnt/scratch/traces/setup-changelog.csv $(($nb_client))
clush -w @client -b ./prototype_MDS/client_launch.sh $(($nb_srv)) /mnt/scratch/traces/setup-changelog /mnt/result/dh

printf "setup finished\n"

current_step=0
turn=1

while [[ $current_step -lt $total_step ]]
do
    #launch a step of traces
    clush -w @client -b ./prototype_MDS/client_launch.sh $(($nb_srv)) $traces_path-$current_step /mnt/result/dh
    ((current_step++))

    if [ $turn -eq $rebalance ]
    then
        #launch redistribution
        clush -w @srv 'kill -s SIGUSR2 `/usr/sbin/pidof ./prototype_MDS/bin/server`'
        clush -w vm0 'kill -s SIGUSR2 `/usr/sbin/pidof ./prototype_MDS/bin/manager`'
        turn=1
    else
        #mesuring
        turn=$((turn+1))
        clush -w @srv 'kill -s SIGUSR1 `/usr/sbin/pidof ./prototype_MDS/bin/server`'
    fi

    #wait for file creation "vm<id_srv>USR"
    for ((i = 0; i < nb_srv; i++))
    do
        if ! [ -f "/mnt/scratch/tmp_ack/dh/vm$(($i))USR" ]
        then
            ((i--))
            sleep 1
        fi
        done
    rm /mnt/scratch/tmp_ack/dh/*USR

    printf "step $current_step finished\n"
done
