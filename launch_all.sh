#! /bin/bash

if [ -z $1 ] || [ -z $2 ] || [ -z $3 ]
then
    echo "please give a number of servers and clients and a type of run"
    exit 1
fi

nb_srv=$1
nb_client=$2
total_vm=$(($1+$2))
run=$3

#traces: 1 min
total_step=1454
traces_path=/mnt/scratch/traces/real/1min/changelog
#traces_path=/mnt/scratch/traces/generated/____/traces
distrib=5 #distribution action

#config clush groups
sudo sh -c "echo \"srv: vm[0-$(($nb_srv-1))]
client: vm[$nb_srv-$(($total_vm-1))]\">/etc/clustershell/groups"

# prepare trace files
#python setup.py /mnt/scratch/traces/changelog.csv > /mnt/scratch/traces/setup-changelog.csv
#python split.py /mnt/scratch/traces/changelog.csv 10
#for each step file:
#python spread.py /mnt/scratch/traces/changelog-0.csv $(($nb_client))

#clean perf folder before testing
rm /mnt/result/$run/server/*
rm /mnt/result/$run/client/*
rm /mnt/result/$run/*
#clean tmp folder
rm /mnt/scratch/tmp_ack/$run/*

#launch servers and manager
clush -w @srv -b  ./prototype_MDS/gen_srv_cfg.sh
clush -w vm0 -b ./prototype_MDS/bin/manager $(($nb_srv))&
clush -w @srv -b ./prototype_MDS/bin/server $(($nb_srv)) p&
sleep 5

#prepare servers for traces
#python36 spread.py /mnt/scratch/traces/setup-changelog.csv $(($nb_client))
clush -w @client -b ./prototype_MDS/client_launch.sh $(($nb_srv)) /mnt/scratch/traces/real/setup-changelog /mnt/result/$run

printf "setup finished\n"

current_step=0
turn=1

while [[ $current_step -lt $total_step ]]
do
    #launch a step of traces
    clush -w @client -b ./prototype_MDS/client_launch.sh $(($nb_srv)) $traces_path-$current_step /mnt/result/$run
    ((current_step++))

    if [ $turn -eq $distrib ]
    then
        #mesuring and rebalancing if needed
        clush -w @srv 'kill -s SIGUSR2 `/usr/sbin/pidof ./prototype_MDS/bin/server`'
        clush -w vm0 'kill -s SIGUSR2 `/usr/sbin/pidof ./prototype_MDS/bin/manager`'
    else
        #mesuring only
        turn=$((turn+1))
        clush -w @srv 'kill -s SIGUSR1 `/usr/sbin/pidof ./prototype_MDS/bin/server`'
    fi

    ./prototype_MDS/protocol_test/synchro.bash $(($nb_srv)) $current_step $run
    rm /mnt/scratch/tmp_ack/$run/*USR-*
    printf "step $current_step finished\n"
done
