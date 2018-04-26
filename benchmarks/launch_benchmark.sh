#! /bin/bash

#echo "haha"

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

#config clush groups
echo "srv: vm[0-$(($nb_srv-1))]
client: vm[$nb_srv-$(($total_vm-1))]">~/.local/etc/clustershell/groups

#wait vm to be up
#sleep 10
clush -w @srv -b ./prototype_MDS/bin/server p&
clush -w @client -b ./prototype_MDS/client_launch.sh $(($nb_srv))
#wait process to be finished
sleep 20
#clush -w @srv -b "ls prototype_MDS/dataStore/ | wc -l"
python /ccc/home/cont001/ocre/billae/scripts/perf_compute.py $nb_srv $nb_client
