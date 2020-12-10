# MeDiE presentation
MeDiE stand for "Metadata Distribution Evaluator" and is a project developped during my PhD.

This project is based in a object storage context and its principal goal is to enable comparisons between different metadata distribution methods in a same environnement without ambiguous adaptations.

MeDiE is written in the C language for the main module (clients-servers simulator), the simulation protocol scripts are in bash, and the generation curves are done thanks to gnupmot and python scripts.
This projec was developped on the CEA internal calculator and uses proper tools available on it, such as PCOCC (https://github.com/cea-hpc/pcocc).
However, an abstraction of these tools could be done, as the conception of the simulator itself is in C.


# Usage


1. Chose the distribution method: *ln -s makefile makefile_<distribution_name>*
2. Compile on all virtual machines (clients and servers)
3. Launch the *launch_all.sh* script

The *lauch_all.sh* script needs a pcocc cluster (with n_servers + n_clients +1 vms) already allocated to run. Arguments are:
- the number of servers
- the number of clients
- the run prefix
- the run type
- the traces path
- the number of step
- the type of traces (r or g)
- the frequence of redistribution.

Output data will be created in the */mnt/result/$run_path/* folder of the virtual machines.
The script *post_proces.sh* in the *benchmarks/scripts* folder will generate evaluation curves from output data



The *generate_jobs.sh* and the *run_jobs.sh* enable to create and run all jobs associated with the parameter analysis for each method.
The script *all_proces.sh* in the *benchmarks/scripts* folder will generate all evaluation curves and then the *aggregate_err.sh* and *parameter_analyse.sh* will produce the parameter analysis curves.
