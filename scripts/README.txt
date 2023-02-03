Examples :
   
 - folding with PRESTO (see /home/msok/Desktop/FRBs/FRB_ASKAP_RW/MWA_FRB_searches_real-time/20230124_test_observation_proposal_PART2.odt ) :
   export PATH=~/mwafrb/scripts/:$PATH
   ~/mwafrb/scripts/presto_fold_all.sh


 - Fredda manual options ( see /home/msok/Desktop/FRBs/FRB_ASKAP_RW/MWA_FRB_searches_real-time/20230124_test_observation_proposal_PART2-FREDDA.odt ) :   
   - normalised files :
     /home/msok/askap/craft/fredda/craft/cuda-fdmt/cudafdmt/fredda_current//craft/cuda-fdmt/cudafdmt/src/cudafdmt 1359220824_20230131172006_ch120_02_norm.fil  -t 512 -d 2048 -S 0 -r 1 -s 1 -m 0 -x 10 -o test.cand


   - not normalised files :
     /home/msok/askap/craft/fredda/craft/cuda-fdmt/cudafdmt/fredda_current//craft/cuda-fdmt/cudafdmt/src/cudafdmt 1359220824_20230131172006_ch120_02.fil  -t 512 -d 2048 -S 0 -r 1 -s 1 -m 0 -x 10 -o test.cand -b 32


 - Show candidates using Keith's viewer :

    1/ check both non normalised candidates in 1st directory and also normalised candidates. 
       They can be slightly different and this is worth keeping in mind. Main command is :

       showcand.sh 1359220824_20230131172006_ch120_02.fil 10


  - show candidates using ds9 viewer :
    /home/msok/github/mwafrb/scripts/ds9_fredda_all!
    (also in ~/bighorns/software/analysis/scripts/shell )