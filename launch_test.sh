#! /bin/bash

# wait for VM to be "up and running"
#pushd / >/dev/null
#ready="/var/lib/cloud/instance/boot-finished"
#declare -a pids
#max_id=$((SLURM_NTASKS - 1))
#echo "max id of VMs: $max_id"
#for i in $(seq 0 $max_id); do
#    pcocc exec -u root -i$i bash -c "while [ ! -f $ready ]; do sleep 1; done" &
#    pids+=($!)
#done

#echo "waiting for vms to be up and running"
#wait ${pids[@]}
#echo "vms are up and running"

if [ -z $1 ] || [ -z $2 ]
then
    echo "please give a number of servers and clients"
    exit 1
fi
#popd >/dev/null

nb_srv=$1
nb_client=$2
total_vm=$(($1+$2))
total_step=4
#config clush groups
sudo sh -c "echo \"srv: vm[0-$(($nb_srv-1))]
client: vm[$nb_srv-$(($total_vm-1))]\">/etc/clustershell/groups"

# prepare trace files
#python setup.py /media/traces/changelog.csv > /media/traces/setup-changelog.csv
#python split.py /media/traces/changelog.csv 10
#python spread.py /media/traces/changelog-0.csv $(($nb_client))

#clean perf folder before testing
rm /mnt/server/*
rm /mnt/client/*

#clean tmp folder
rm /media/tmp_ack/*

#launch servers and manager
clush -w @srv -b  ./prototype_MDS/gen_srv_cfg.sh
clush -w vm16 -b ./prototype_MDS/bin/manager $(($nb_srv))&
clush -w @srv -b ./prototype_MDS/bin/server $(($nb_srv)) p&
#clush -w @client -b echo "test"
sleep 5

#prepare servers for traces
python36 spread.py /media/traces/setup-changelog.csv $(($nb_client))
clush -w @client -b ./prototype_MDS/client_launch.sh $(($nb_srv)) /media/traces/setup-changelog

printf "setup finished\n"

current_step=0
while [[ $current_step -lt 98 ]]
do
    for((turn = 0; turn < total_step; turn++))
    do
        #launch a step of traces
        python36 spread.py /media/traces/changelog-$(($current_step)).csv $(($nb_client))
        clush -w @client -b ./prototype_MDS/client_launch.sh $(($nb_srv)) /media/traces/changelog-$current_step
        ((current_step++))

            #mesuring
            clush -w @srv 'kill -s SIGUSR1 `/usr/sbin/pidof ./prototype_MDS/bin/server`'

            #wait for file creation "vm[id_srv]USR1"
            for ((i = 0; i < nb_srv; i++))
            do
                if ! [ -f "/media/tmp_ack/vm$(($i))USR1" ]
                then
                    ((i--))
                    sleep 1
                fi
            done
            rm /media/tmp_ack/*USR1
 #       fi
        printf "step $current_step finished\n"
    done

    #launch redistribution
    clush -w @srv 'kill -s SIGUSR2 `/usr/sbin/pidof ./prototype_MDS/bin/server`'

    #wait for file creation "vm[id_svr]USR2-1"
    for ((i = 0; i < nb_srv; i++))
    do
        if ! [ -f "/media/tmp_ack/vm$(($i))USR2-1" ]
        then
            ((i--))
            sleep 1
        fi
    done
    rm /media/tmp_ack/*USR2
done
