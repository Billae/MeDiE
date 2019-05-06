#! /bin/bash

if [ -z $1 ] || [ -z $2 ]
then
    echo "please give a number of servers and clients"
    exit 1
fi

nb_srv=$1
nb_client=$2
total_vm=$(($1+$2))

#traces: mesuring 5min
total_step=292
traces_path=/media/traces/5min/12_clients

#config clush groups
sudo sh -c "echo \"srv: vm[0-$(($nb_srv-1))]
client: vm[$nb_srv-$(($total_vm-1))]\">/etc/clustershell/groups"

# prepare trace files
#python setup.py /media/traces/changelog.csv > /media/traces/setup-changelog.csv
#python split.py /media/traces/changelog.csv 5
#for all step do:
#python spread.py /media/traces/changelog-0.csv $(($nb_client))


#clean perf folder before testing
rm /mnt/sh/server/*
rm /mnt/sh/*
rm /mnt/sh/client/*

#clean tmp folder
rm /media/tmp_ack/sh/*

#launch servers
clush -w @srv -b  ./prototype_MDS/gen_srv_cfg.sh
clush -w @srv -b ./prototype_MDS/bin/server $(($nb_srv)) p&
sleep 5

#prepare servers for traces
#python36 spread.py /media/traces/setup-changelog.csv $(($nb_client))
clush -w @client -b ./prototype_MDS/client_launch.sh $(($nb_srv)) /media/traces/setup-changelog /mnt/sh

printf "setup finished\n"

current_step=0
while [[ $current_step -lt $total_step ]]
do
        #launch a step of traces
        clush -w @client -b ./prototype_MDS/client_launch.sh $(($nb_srv)) $traces_path/changelog-$current_step /mnt/sh
        ((current_step++))

        #mesuring
        clush -w @srv 'kill -s SIGUSR1 `/usr/sbin/pidof ./prototype_MDS/bin/server`'

        #wait for file creation "vm<id_srv>USR"
        for ((i = 0; i < nb_srv; i++))
        do
            if ! [ -f "/media/tmp_ack/sh/vm$(($i))USR" ]
            then
                ((i--))
                sleep 1
            fi
        done
        rm /media/tmp_ack/sh/*USR

        printf "step $current_step finished\n"
done
