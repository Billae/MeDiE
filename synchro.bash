#! /bin/bash
#

function vm_list()
{
    local -i vm_count="$1"

    seq 0 $((vm_count - 1))
}

function wait_all()
{
    for vm in $(vm_list); do
        # XXX: does not work if $1 contains whitespaces
        while ! [ -f $vm_$1 ]; do
            sleep 1
        done
    done
}

# Wait for every server to create either "vmXUSR2-0" or "vmXUSR2-1"
wait_all "USR2-?"

shopt -s nullglob

declare -a acks=(vm*USR2-1)

# If there is at least one "vmXUSR2-1" file, wait for them all
[ ${#acks[@]} -gt 0 ] && wait_all "USR2-1"
