#!/bin/bash

if [ -z $1 ]
then
    echo "Please give a job dir"
    exit
fi

job_dir=$1
mkdir -p $job_dir
time_limit=120

real_tmp_ack="/ccc/scratch/cont001/ocre/billae/scratch_vm/tmp_ack/"

gen_job () {
job_file="$job_dir${run_path//\//_}.sh"
cat << EOF > $job_file
    #!/bin/bash
    set -x
    if [[ -e "$real_tmp_ack/$run_path/done" ]]
    then
        echo "== INFO: Job $run_path already done, skipping...\n"
        exit
    fi
    pcocc_out=(\$(pcocc batch -p haswell -t $time_limit -c 4 all:17))
    job_id=\${pcocc_out[-1]}
    until [[ "\$(squeue -j \$job_id -o %T | tail -n 1)" = "RUNNING" ]]; do
        sleep 60
    done
    pcocc agent ping -j \$job_id
    for j in {0..16}; do
        pcocc ssh -j \$job_id vm\$j "make -j 4 -f prototype_MDS/Makefile_${method} CPPFLAGS=\"$def\"" &
    done
    wait
    echo "== INFO: Compiling job \$job_id finished"
    pcocc ssh -j \$job_id vm16 mkdir -p $log_path
    pcocc ssh -j \$job_id vm16 "./prototype_MDS/launch_all.sh 4 12 $run_path $method $flux_path $flux_step $flux_type $redistrib &> $log_path/\$(date -I).log &" 
    set +x
EOF
chmod +x $job_file
}

run_job () {
    job_id=$(pcocc batch -p haswell --qos=test -t $((2*60)) -c 4 all:17 | cut -d ' ' -f 4)
    until [[ "$(squeue -j $job_id -o %T | tail -n 1)" = "RUNNING" ]]; do
        sleep 60
    done
    pcocc agent ping -j $job_id
    for j in {0..16}; do
        pcocc ssh -j $job_id vm$j "make -j 4 -f prototype_MDS/Makefile_${method} CPPFLAGS=\"$def\"" &
    done
    wait
    echo "Compiling finished"
    pcocc ssh -j $job_id vm16 mkdir -p $log_path
    pcocc ssh -j $job_id vm16 "./prototype_MDS/launch_all.sh 4 12 $run_path $method /mnt/scratch/traces/real/5min/12_clients/changelog 292 r $redistrib &> $log_path/$(date -I).log &" 
}

gen_post () {
result_path="$WORKDIR/results/$run_path/"
script_path="$SCRATCHDIR/prototype_MDS_scratch/benchmarks/scripts/"
distrib_interval=$(echo "$redistrib * 5" |bc)
mkdir -p $result_path
cat << EOF > $result_path/post.sh
    #!/bin/bash
    $script_path/post_process.sh $result_path $method $real_flux_path $flux_step

    nb_distrib=\$(wc -l $result_path/server/rebalancing |cut -d" " -f 1)
    cost=\$(echo "\$nb_distrib * (2*4 + 4/2)" |bc)

    reward_av=\$(cat $result_path/average_error.txt)
    reward_max=\$(cat $result_path/max_error.txt)

    score_av=\$(echo "\$reward_av / \$cost" |bc -l)
    score_max=\$(echo "\$reward_max / \$cost" |bc -l)

    #create deviation_recap file
    echo "alpha;N_entry;redistribution_interval;nb_redistribution;cost;average_deviation;max_deviation;score_av;score_max" > $result_path/deviation_recap.txt
    echo "$alpha;$nentry;$distrib_interval;\$nb_distrib;\$cost;\$reward_av;\$reward_max;\$score_av;\$score_max">> $result_path/deviation_recap.txt
EOF
chmod +x "$result_path/post.sh"
}

flux=real_3h
suffix_flux="real/5min/12_clients/changelog"
#flux="on_off_faible_10"
#suffix_flux="generated/ON_OFF/faible/10percent/traces"
#flux="mc_faible_10"
#suffix_flux="generated/MC/faible/10percent/traces"
real_flux_path="$SCRATCHDIR/scratch_vm/traces/$suffix_flux"
flux_path="/mnt/scratch/traces/$suffix_flux"
flux_step=36
flux_type="r"
method="dh"
for alpha in 0 0.1 0.2 0.3 0.4 0.5 0.6 0.7 0.8 0.9 1; do
    for nentry in 50 100 500 1000 5000 10000; do
        for redistrib in 2 6 12 24 60; do
            run_path=flux_$flux/method_$method/alpha_$alpha/nentry_$nentry/rebalancing_$redistrib/
            log_path=/mnt/scratch/logs/$run_path

            # all escapes character to give " to source code
            #for run launching
            #def="-DALPHA=$alpha -DN_ENTRY=$nentry -DSCRATCH=\\\\\\\"/mnt/scratch/tmp_ack/$run_path\\\\\\\" -DPREFIX=\\\\\\\"/mnt/result/$run_path\\\\\\\" "
            #for file generating
            def="-DALPHA=$alpha -DN_ENTRY=$nentry -DSCRATCH=\\\\\\\\\\\\\\\"/mnt/scratch/tmp_ack/$run_path\\\\\\\\\\\\\\\" -DPREFIX=\\\\\\\\\\\\\\\"/mnt/result/$run_path\\\\\\\\\\\\\\\" "
            #run_job
            gen_job
            gen_post
        done
    done
done

#method=indedh
#for percent in 0 5 10 15 20 25 30 35 40 45 50; do
#    for nentry in 50 100 500 1000 5000 10000 ; do
#        for redistrib in 5; do
#            run_path=method_$method/percent_$percent/nentry_$nentry/rebalancing_$redistrib/
#            # all escapes character to give " to source code
#            def="-DPERCENT=$percent -DN_ENTRY=$nentry -DSCRATCH=\\\\\\\"/mnt/scratch/tmp_ack/$run_path\\\\\\\" -DPREFIX=\\\\\\\"/mnt/result/$run_path\\\\\\\" "
#            run_job
#        done
#    done
#done
#
#method=windowed
#for alpha in 0 0.1 0.2 0.3 0.4 0.5 0.6 0.7 0.8 0.9 1; do
#    for size in 2 6 12 24 60; do
#        for redistrib in 5; do
#            run_path=method_$method/alpha_$alpha/size_$nentry/rebalancing_$redistrib/
#            # all escapes character to give " to source code
#            def="-DALPHA=$alpha -DWINDOW_SIZE=$size -DSCRATCH=\\\\\\\"/mnt/scratch/tmp_ack/$run_path\\\\\\\" -DPREFIX=\\\\\\\"/mnt/result/$run_path\\\\\\\" "
#            run_job
#        done
#    done
#done
