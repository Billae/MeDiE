#! /bin/bash
#

n_srv=$1
n_step=$2
path="/media/tmp_ack/indedh/"
result_path="/mnt/indedh/server/rebalancing"

function vm_list()
{
    local -i vm_count="$1"

    printf 'vm%i ' $(seq 0 $((vm_count - 1)))
}

#function to wait all ack-1
function wait_1()
{
    for vm in $(vm_list $n_srv); do
        # XXX: does not work if $1 contains whitespaces
#        echo "search ack ${path}${vm}USR2-1"
        while ! [ -f "${path}${vm}USR-1" ]; do
#            echo "wait ack ${path}${vm}USR-1"
            sleep 1
        done
    done
    printf "$n_step\n" >> $result_path
}

# Wait for every server to create either "vmXUSR-0" or "vmXUSR-1"
for vm in $(vm_list $n_srv); do
    # XXX: does not work if $1 contains whitespaces
#     echo "search ack ${path}${vm}USR-?"
     while ! [ -f "${path}${vm}USR-0" ] && ! [ -f "${path}${vm}USR-1" ]; do
#        echo "wait ack ${path}${vm}USR-?"
        sleep 1
    done
done

shopt -s nullglob

declare -a acks=(${path}vm*USR-1)

# If there is at least one "vmXUSR2-1" file, wait for them all
[ ${#acks[@]} -gt 0 ] && wait_1
