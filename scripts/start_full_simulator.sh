#!/bin/bash

# mkfifo fil_list.txt

if [[ ! -p fredda_pipe ]]; then
    mkfifo fredda_pipe
fi


rm -f fil_list.txt  2*.fil merged_fil_list.fifo

nohup merger.sh > merger.out 2>&1 &
sleep 5

start_simulator.sh

# start fredda 
# export path_new=/home/msok//askap/craft/fredda/craft/cuda-fdmt/cudafdmt/src/
# fredda version using pipe :
sleep 5
export path_new=/home/msok/askap/craft/fredda/craft/cuda-fdmt/cudafdmt/fredda_current/craft/cuda-fdmt/cudafdmt/src
echo "nohup ${path_new}/cudafdmt fredda_pipe  -t 512 -d 2048 -S 0 -r 1 -s 1 -m 1 -x 7.0 -o candidates_pipe.cand > fredda.out 2>&1 &"
nohup ${path_new}/cudafdmt fredda_pipe  -t 512 -d 2048 -S 0 -r 1 -s 1 -m 1 -x 7.0 -o candidates_pipe.cand > fredda.out 2>&1 &

sleep 5
# start putting data to fredda pipe :
echo "nohup pipe_filfiles merged_fil_list.fifo fredda_pipe > pipe_fredda.out 2>&1 &"
nohup pipe_filfiles merged_fil_list.fifo fredda_pipe > pipe_fredda.out 2>&1 &
