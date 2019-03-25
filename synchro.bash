#! /bin/bash
#
n_srv=4
path="./tmp/"

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
        echo "search ack ${path}${vm}USR2-1"
        while ! [ -f "${path}${vm}USR2-1" ]; do
            echo "wait ack ${path}${vm}USR2-1"
            sleep 1
        done
    done
}

# Wait for every server to create either "vmXUSR2-0" or "vmXUSR2-1"
for vm in $(vm_list $n_srv); do
    # XXX: does not work if $1 contains whitespaces
     echo "search ack ${path}${vm}USR2-?"
     while ! [ -f "${path}${vm}USR2-0" ] && ! [ -f "${path}${vm}USR2-1" ]; do
        echo "wait ack ${path}${vm}USR2-?"
        sleep 1
    done
done

shopt -s nullglob

declare -a acks=(${path}vm*USR2-1)

# If there is at least one "vmXUSR2-1" file, wait for them all
[ ${#acks[@]} -gt 0 ] && wait_1
