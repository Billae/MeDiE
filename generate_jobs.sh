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


gen_post_sh () {
result_path="$WORKDIR/results/$run_path/"
script_path="$SCRATCHDIR/prototype_MDS_scratch/benchmarks/scripts/"
mkdir -p $result_path
cat << EOF > $result_path/post.sh
    #!/bin/bash
    $script_path/post_process.sh $result_path sh $real_flux_path $flux_step
    deviation_av=\$(cat $result_path/average_error.txt)
    deviation_max=\$(cat $result_path/max_error.txt)

    #create deviation_recap file
    echo "deviation_av;deviation_max" > $result_path/deviation_recap.txt
    echo "\$deviation_av;\$deviation_max">> $result_path/deviation_recap.txt
EOF
chmod +x "$result_path/post.sh"
}

gen_post_dh () {
result_path="$WORKDIR/results/$run_path/"
script_path="$SCRATCHDIR/prototype_MDS_scratch/benchmarks/scripts/"
distrib_interval=$(echo "$redistrib * 5" |bc)
mkdir -p $result_path
cat << EOF > $result_path/post.sh
    #!/bin/bash
    $script_path/post_process.sh $result_path dh $real_flux_path $flux_step

    distrib_needed=\$(sort -u $result_path/needed_rebalancing.txt |wc -l |cut -d" " -f 1)
    distrib_useful=\$(sort -u $result_path/useful_rebalancing.txt |wc -l |cut -d" " -f 1)
    distrib_useless=\$(sort -u $result_path/useless_rebalancing.txt |wc -l |cut -d" " -f 1)
    
    deviation_av=\$(cat $result_path/average_error.txt)
    deviation_max=\$(cat $result_path/max_error.txt)
    #cost for DH:n_distrib * (2*N +N/2);    for indedh: n_distrib * (4N + N/2)
    cost=\$(echo "(\$distrib_useful + \$distrib_useless) * (2*4 + 4/2)" |bc)

    #create deviation_recap file
    echo "alpha;N_entry;redistribution_interval;redistribution_useless;resdistribution_useful;redistribution_needed;deviation_av;deviation_max;cost" > $result_path/deviation_recap.txt
    echo "$alpha;$nentry;$distrib_interval;\$distrib_useless;\$distrib_useful;\$distrib_needed;\$deviation_av;\$deviation_max;\$cost">> $result_path/deviation_recap.txt
EOF
chmod +x "$result_path/post.sh"
}


gen_post_indedh () {
result_path="$WORKDIR/results/$run_path/"
script_path="$SCRATCHDIR/prototype_MDS_scratch/benchmarks/scripts/"
mkdir -p $result_path
cat << EOF > $result_path/post.sh
    #!/bin/bash
    $script_path/post_process.sh $result_path indedh $real_flux_path $flux_step

    distrib_needed=\$(sort -u $result_path/needed_rebalancing.txt |wc -l |cut -d" " -f 1)
    distrib_useful=\$(sort -u $result_path/useful_rebalancing.txt |wc -l |cut -d" " -f 1)
    distrib_useless=\$(sort -u $result_path/useless_rebalancing.txt |wc -l |cut -d" " -f 1)
    
    deviation_av=\$(cat $result_path/average_error.txt)
    deviation_max=\$(cat $result_path/max_error.txt)
    #cost for DH:n_distrib * (2*N +N/2);    for indedh: n_distrib * (4N + N/2)
    cost=\$(echo "(\$distrib_useful + \$distrib_useless) * (4*4 + 4/2)" |bc)

    #create deviation_recap file
    echo "percent;N_entry;redistribution_useless;resdistribution_useful;redistribution_needed;deviation_av;deviation_max;cost" > $result_path/deviation_recap.txt
    echo "$percent;$nentry;\$distrib_useless;\$distrib_useful;\$distrib_needed;\$deviation_av;\$deviation_max;\$cost">> $result_path/deviation_recap.txt
EOF
chmod +x "$result_path/post.sh"
}


gen_post_windowed () {
result_path="$WORKDIR/results/$run_path/"
script_path="$SCRATCHDIR/prototype_MDS_scratch/benchmarks/scripts/"
mkdir -p $result_path
cat << EOF > $result_path/post.sh
    #!/bin/bash
    $script_path/post_process.sh $result_path windowed $real_flux_path $flux_step

    distrib_needed=\$(sort -u $result_path/needed_rebalancing.txt |wc -l |cut -d" " -f 1)
    distrib_useful=\$(sort -u $result_path/useful_rebalancing.txt |wc -l |cut -d" " -f 1)
    distrib_useless=\$(sort -u $result_path/useless_rebalancing.txt |wc -l |cut -d" " -f 1)
    
    deviation_av=\$(cat $result_path/average_error.txt)
    deviation_max=\$(cat $result_path/max_error.txt)
    #cost for DH:n_distrib * (2*N +N/2);    for indedh: n_distrib * (4N + N/2)
    cost=\$(echo "(\$distrib_useful + \$distrib_useless) * (4*4 + 4/2)" |bc)

    #create deviation_recap file
    echo "alpha;size;redistribution_useless;resdistribution_useful;redistribution_needed;deviation_av;deviation_max;cost" > $result_path/deviation_recap.txt
    echo "$alpha;$size;\$distrib_useless;\$distrib_useful;\$distrib_needed;\$deviation_av;\$deviation_max;\$cost">> $result_path/deviation_recap.txt
EOF
chmod +x "$result_path/post.sh"
}


#flux=chaos_fort_10
#suffix_flux="generated/CHAOS/fort/10percent/traces"
#flux=real_3h
#suffix_flux="real/5min/12_clients/changelog"
#flux="pic_fort_10"
#suffix_flux="generated/PIC/fort/10percent/traces"
#flux="on_off_faible_10"
#suffix_flux="generated/ON_OFF/faible/10percent/traces"
#flux="mc_fort_10"
#suffix_flux="generated/MC/fort/10percent/traces"
#
#real_flux_path="$SCRATCHDIR/scratch_vm/traces/$suffix_flux"
#flux_path="/mnt/scratch/traces/$suffix_flux"
#flux_step=36
#flux_type="r"
#method="dh"
#for alpha in 0 0.1 0.2 0.3 0.4 0.5 0.6 0.7 0.8 0.9 1; do
#    for nentry in 50 100 500 1000 5000 10000; do
#        for redistrib in 2 6 12 24 60; do
#            run_path=flux_$flux/method_$method/alpha_$alpha/nentry_$nentry/rebalancing_$redistrib/
#            log_path=/mnt/scratch/logs/$run_path
#
#            # all escapes character to give " to source code
#            #for run launching
#            #def="-DALPHA=$alpha -DN_ENTRY=$nentry -DSCRATCH=\\\\\\\"/mnt/scratch/tmp_ack/$run_path\\\\\\\" -DPREFIX=\\\\\\\"/mnt/result/$run_path\\\\\\\" "
#            #for file generating
#            def="-DALPHA=$alpha -DN_ENTRY=$nentry -DSCRATCH=\\\\\\\\\\\\\\\"/mnt/scratch/tmp_ack/$run_path\\\\\\\\\\\\\\\" -DPREFIX=\\\\\\\\\\\\\\\"/mnt/result/$run_path\\\\\\\\\\\\\\\" "
#            #run_job
#            gen_job
#            gen_post_dh
#        done
#    done
#done
#
#method="indedh"
#for percent in 0 5 10 15 20 25 30 35 40 45 50; do
#    for nentry in 50 100 500 1000 5000 10000; do
#        for redistrib in 1; do
#            run_path=flux_$flux/method_$method/percent_$percent/nentry_$nentry/rebalancing_$redistrib/
#            log_path=/mnt/scratch/logs/$run_path
#
#            # all escapes character to give " to source code
#            #for run launching
#            #def="-DPERCENT=$percent -DN_ENTRY=$nentry -DSCRATCH=\\\\\\\"/mnt/scratch/tmp_ack/$run_path\\\\\\\" -DPREFIX=\\\\\\\"/mnt/result/$run_path\\\\\\\" "
#            #for file generating
#            def="-DPERCENT=$percent -DN_ENTRY=$nentry -DSCRATCH=\\\\\\\\\\\\\\\"/mnt/scratch/tmp_ack/$run_path\\\\\\\\\\\\\\\" -DPREFIX=\\\\\\\\\\\\\\\"/mnt/result/$run_path\\\\\\\\\\\\\\\" "
#            #run_job
#            gen_job
#            gen_post_indedh
#        done
#    done
#done

#method="windowed"
#for alpha in 0 0.1 0.2 0.3 0.4 0.5 0.6 0.7 0.8 0.9 1; do
#    for size in 2 6 12 24 60; do
#        for redistrib in 1; do
#            run_path=flux_$flux/method_$method/alpha_$alpha/size_$size/rebalancing_$redistrib/
#            log_path=/mnt/scratch/logs/$run_path
#
#            # all escapes character to give " to source code
#            #for run launching
#            #def="-DALPHA=$alpha -DWINDOW_SIZE=$size -DSCRATCH=\\\\\\\"/mnt/scratch/tmp_ack/$run_path\\\\\\\" -DPREFIX=\\\\\\\"/mnt/result/$run_path\\\\\\\" "
#            #for file generating
#            def="-DALPHA=$alpha -DWINDOW_SIZE=$size -DSCRATCH=\\\\\\\\\\\\\\\"/mnt/scratch/tmp_ack/$run_path\\\\\\\\\\\\\\\" -DPREFIX=\\\\\\\\\\\\\\\"/mnt/result/$run_path\\\\\\\\\\\\\\\" "
#            #run_job
#            gen_job
#            gen_post_windowed
#        done
#    done
#done

##SH
method="sh"

for i in {1..11}; do
    case $i in
    1)  flux=real_3h
        suffix_flux="real/5min/12_clients/changelog";;
    
    2)  flux="on_off_faible_10"
        suffix_flux="generated/ON_OFF/faible/10percent/traces";;
    3)  flux="on_off_fort_10"
        suffix_flux="generated/ON_OFF/fort/10percent/traces";;
    4)  flux="on_off_random_80"
        suffix_flux="generated/ON_OFF/random/80percent/traces";;
    
    5)  flux="mc_faible_10"
        suffix_flux="generated/MC/faible/10percent/traces";;
    6)  flux="mc_fort_10"
        suffix_flux="generated/MC/fort/10percent/traces";;
    7)  flux="mc_random_80"
        suffix_flux="generated/MC/random/80percent/traces";;
 
    8)  flux="pic_fort_10"
        suffix_flux="generated/PIC/fort/10percent/traces";;

    9)  flux="chaos_faible_10"
        suffix_flux="generated/CHAOS/faible/10percent/traces";;
    10) flux="chaos_fort_10"
        suffix_flux="generated/CHAOS/fort/10percent/traces";;
    11) flux="chaos_random_80"
        suffix_flux="generated/CHAOS/random/80percent/traces";;
 
   esac

    real_flux_path="$SCRATCHDIR/scratch_vm/traces/$suffix_flux"
    flux_path="/mnt/scratch/traces/$suffix_flux"
    flux_step=36
    flux_type="r"
    redistrib=1

    run_path=flux_$flux/method_$method/rebalancing_$redistrib/
    log_path=/mnt/scratch/logs/$run_path
    # all escapes character to give " to source code
    #for file generating
    def="-DSCRATCH=\\\\\\\\\\\\\\\"/mnt/scratch/tmp_ack/$run_path\\\\\\\\\\\\\\\" -DPREFIX=\\\\\\\\\\\\\\\"/mnt/result/$run_path\\\\\\\\\\\\\\\" "
    gen_job
    gen_post_sh
done

