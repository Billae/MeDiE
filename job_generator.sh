#!/bin/bash

if [ -z $1 ]
    echo "Please give a job dir"

job_dir=$1
mkdir -p $job_dir

gen_job () {
jobfile="$job_dir${runpath//\//_}.sh"
cat << EOF > $jobfile
    #!/bin/bash
    jobid=\$(pcocc batch -p haswell -t $((2*60)) -c 4 all:17 | cut -d ' ' -f 4)
    until [[ "\$(squeue -j \$jobid -o %T | tail -n 1)" = "RUNNING" ]]; do
        sleep 60
    done
    pcocc agent ping -j \$jobid
    for j in {0..16}; do
        pcocc ssh -j \$jobid vm\$j "make -j 4 -f prototype_MDS/Makefile_${method} CPPFLAGS=\"$def\"" &
    done
    wait
    echo "Compiling job \$jobid finished"
    pcocc ssh -j \$jobid vm16 mkdir -p $logpath
    pcocc ssh -j \$jobid vm16 "./prototype_MDS/launch_all.sh 4 12 $runpath $method $flux_param $redistrib &> $logpath/\$(date -I).log &" 
EOF
chmod +x $jobfile
}
run_job () {
    jobid=$(pcocc batch -p haswell --qos=test -t $((2*60)) -c 4 all:17 | cut -d ' ' -f 4)
    until [[ "$(squeue -j $jobid -o %T | tail -n 1)" = "RUNNING" ]]; do
        sleep 60
    done
    pcocc agent ping -j $jobid
    for j in {0..16}; do
        pcocc ssh -j $jobid vm$j "make -j 4 -f prototype_MDS/Makefile_${method} CPPFLAGS=\"$def\"" &
    done
    wait
    echo "Compiling finished"
    pcocc ssh -j $jobid vm16 mkdir -p $logpath
    pcocc ssh -j $jobid vm16 "./prototype_MDS/launch_all.sh 4 12 $runpath $method /mnt/scratch/traces/real/5min/12_clients/changelog 292 r $redistrib &> $logpath/$(date -I).log &" 
}

flux=real_3h
flux_param="/mnt/scratch/traces/real/5min/12_clients/changelog 36 r"
method=dh
for alpha in 0 0.1 0.2 0.3 0.4 0.5 0.6 0.7 0.8 0.9 1; do
    for nentry in 50 100 500 1000 5000 10000; do
        for redistrib in 2 6 12 24 60; do
            runpath=flux_$flux/method_$method/alpha_$alpha/nentry_$nentry/rebalancing_$redistrib/
            logpath=/mnt/scratch/logs/$runpath

            # all escapes character to give " to source code
            #for run launching
            #def="-DALPHA=$alpha -DN_ENTRY=$nentry -DSCRATCH=\\\\\\\"/mnt/scratch/tmp_ack/$runpath\\\\\\\" -DPREFIX=\\\\\\\"/mnt/result/$runpath\\\\\\\" "
            #for file generating
            def="-DALPHA=$alpha -DN_ENTRY=$nentry -DSCRATCH=\\\\\\\\\\\\\\\"/mnt/scratch/tmp_ack/$runpath\\\\\\\\\\\\\\\" -DPREFIX=\\\\\\\\\\\\\\\"/mnt/result/$runpath\\\\\\\\\\\\\\\" "
            #run_job
            gen_job
        done
    done
done

#method=indedh
#for percent in 0 5 10 15 20 25 30 35 40 45 50; do
#    for nentry in 50 100 500 1000 5000 10000 ; do
#        for redistrib in 5; do
#            runpath=method_$method/percent_$percent/nentry_$nentry/rebalancing_$redistrib/
#            # all escapes character to give " to source code
#            def="-DPERCENT=$percent -DN_ENTRY=$nentry -DSCRATCH=\\\\\\\"/mnt/scratch/tmp_ack/$runpath\\\\\\\" -DPREFIX=\\\\\\\"/mnt/result/$runpath\\\\\\\" "
#            run_job
#        done
#    done
#done
#
#method=windowed
#for alpha in 0 0.1 0.2 0.3 0.4 0.5 0.6 0.7 0.8 0.9 1; do
#    for size in 2 6 12 24 60; do
#        for redistrib in 5; do
#            runpath=method_$method/alpha_$alpha/size_$nentry/rebalancing_$redistrib/
#            # all escapes character to give " to source code
#            def="-DALPHA=$alpha -DWINDOW_SIZE=$size -DSCRATCH=\\\\\\\"/mnt/scratch/tmp_ack/$runpath\\\\\\\" -DPREFIX=\\\\\\\"/mnt/result/$runpath\\\\\\\" "
#            run_job
#        done
#    done
#done
