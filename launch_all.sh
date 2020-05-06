#! /bin/bash

if [ $# -ne 8 ]
then
    echo "please give a number of servers and clients; the run prefix and run type; the traces path, the number of step and the type of traces (r or g); and the frequence of redistribution"
    exit 1
fi

nb_srv=$1
nb_client=$2
total_vm=$(($1+$2))

#prefix given by the run
run_path=$3
run=$4
#traces files and number of steps are given. Type is r (real) or g (generated)
traces_path=$5
total_step=$6
trace_type=$7
distrib=$8 #distribution action

#config clush groups
sudo sh -c "echo \"srv: vm[0-$(($nb_srv-1))]
client: vm[$nb_srv-$(($total_vm-1))]\">/etc/clustershell/groups"

#clean perf folder before testing
rm -f $(find /mnt/result/$run_path/* ! -name "post.sh")
mkdir -p /mnt/result/$run_path/server
mkdir -p /mnt/result/$run_path/client
#clean tmp folder
mkdir -p /mnt/scratch/tmp_ack/$run_path
rm -f /mnt/scratch/tmp_ack/$run_path/*

#launch servers and manager
clush -w @srv -b  ./prototype_MDS/gen_srv_cfg.sh
clush -w @srv -b ./prototype_MDS/bin/server $(($nb_srv)) p&
if [ $run != "sh" ]
then
    clush -w vm0 -b ./prototype_MDS/bin/manager $(($nb_srv))&
fi
sleep 5


#prepare servers for traces for real traces
if [ $trace_type = "r" ]
then
    clush -w @client -b ./prototype_MDS/client_launch.sh $(($nb_srv)) /mnt/scratch/traces/real/setup-changelog /mnt/result/$run_path
fi


printf "setup finished\n"

current_step=0
turn=1

while [[ $current_step -lt $total_step ]]
do
    #launch a step of traces
    time(clush -w @client -b ./prototype_MDS/client_launch.sh $(($nb_srv)) $traces_path-$current_step /mnt/result/$run_path)

    if [ $turn -eq $distrib ]
    then
        start=$(date +%s)
        #mesuring and rebalancing if needed
        clush -w @srv 'kill -s SIGUSR2 `/usr/sbin/pidof ./prototype_MDS/bin/server`'
        if [ $run != "sh" ]
        then
            clush -w vm0 'kill -s SIGUSR2 `/usr/sbin/pidof ./prototype_MDS/bin/manager`'
        fi
        turn=1
    else
        start=$(date +%s)
        #mesuring only
        turn=$((turn+1))
        clush -w @srv 'kill -s SIGUSR1 `/usr/sbin/pidof ./prototype_MDS/bin/server`'
    fi

    ./prototype_MDS/protocol_test/synchro.bash $(($nb_srv)) $current_step $run_path
    end=$(date +%s)
    synchro_time=$(($end - $start))
    rm /mnt/scratch/tmp_ack/$run_path/*USR-*
    printf "step $current_step finished\n"
    printf "synchro : $synchro_time\n"
    ((current_step++))
done
rc=$?
if [ $rc -eq 0 ]
then
    touch "/mnt/scratch/tmp_ack/$run_path/done"
else
    touch "/mnt/scratch/tmp_ack/$run_path/failed"
fi
