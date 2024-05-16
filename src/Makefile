# include ../../make.cfg/flags.mak

MWA_FRB=~/github/mwafrb/
# MWA_FRB=/usr/local/

# CMN_INCLUDES=-I../../fitslib/
COMMON_LIBS=/usr/local/lib/libmsfitslib.so -lcfitsio -lnova -lfftw3 -ldl


read_dada : read_dada.c 
	g++ read_dada.c $(OPT) -o read_dada

read_eda2_dada : read_eda2_dada.c
	g++ read_eda2_dada.c -o read_eda2_dada $(OPT)	

dada2fits : dada2fits.cpp
	g++  dada2fits.cpp -I$(MWA_FRB)/msfitslib/src/ -lcfitsio -L$(MWA_FRB)/slib/ -lmsfitslib $(OPT) $(COMMON_LIBS) -o dada2fits
	

all : read_dada read_eda2_dada dada2fits compare_filfiles dumpfilfile_float merge_coarse_channels avg_channels_fil dumpfilfile read_filfile fits2fil fits2fil2 update_fil_header n_fine_channels 

read_filfile : read_filfile.c
	gcc read_filfile.c -o read_filfile $(OPT)

dumpfilfile : CpuTimer.cpp  DataOrder.cpp  DataSource.cpp  InvalidSourceFormat.cpp  main.cpp  SigprocFile.cpp 
	g++ CpuTimer.cpp  DataOrder.cpp  DataSource.cpp  InvalidSourceFormat.cpp  main.cpp  SigprocFile.cpp -o dumpfilfile -I $(MWA_FRB)/msfitslib/src// $(OPT) $(COMMON_LIBS)
	
dumpfilfile_float : main_float.cpp
	g++ CpuTimer.cpp  DataOrder.cpp  DataSource.cpp  InvalidSourceFormat.cpp  main_float.cpp  SigprocFile.cpp -o dumpfilfile_float -I $(MWA_FRB)/msfitslib/src// $(OPT) $(COMMON_LIBS)

dumpfilfile_float_test : main_float_test.cpp
	g++ CpuTimer.cpp  DataOrder.cpp  DataSource.cpp  InvalidSourceFormat.cpp  main_float_test.cpp  SigprocFile.cpp -o dumpfilfile_float_test -I $(MWA_FRB)/msfitslib/src// $(OPT) $(COMMON_LIBS)

fits2fil_bad : fits2fil.cpp
	g++ CpuTimer.cpp  DataOrder.cpp  DataSource.cpp  InvalidSourceFormat.cpp  fits2fil.cpp  SigprocFile.cpp -o fits2fil -I $(MWA_FRB)/msfitslib/src// $(OPT) $(COMMON_LIBS)

fits2fil : fits2fil_working.cpp filfile.cpp SigprocFile.cpp
	g++ CpuTimer.cpp  DataOrder.cpp  DataSource.cpp  InvalidSourceFormat.cpp  fits2fil_working.cpp  SigprocFile.cpp filfile.cpp -o fits2fil -I $(MWA_FRB)/msfitslib/src// $(OPT) $(COMMON_LIBS)

fits2fil2 : fits2fil2.cpp filfile.cpp SigprocFile.cpp
	g++ CpuTimer.cpp  DataOrder.cpp  DataSource.cpp  InvalidSourceFormat.cpp  fits2fil2.cpp  SigprocFile.cpp filfile.cpp -o fits2fil2 -I $(MWA_FRB)/msfitslib/src// $(OPT) $(COMMON_LIBS)

pipe_filfiles : pipe_filfiles.cpp SigprocFile.cpp SigprocFile.h
	g++ CpuTimer.cpp  DataOrder.cpp  DataSource.cpp  InvalidSourceFormat.cpp  pipe_filfiles.cpp  SigprocFile.cpp -o pipe_filfiles -I $(MWA_FRB)/msfitslib/src// $(OPT) $(COMMON_LIBS)
	
avg_channels_fil : avg_channels_fil.cpp SigprocFile.cpp SigprocFile.h
	g++ CpuTimer.cpp  DataOrder.cpp  DataSource.cpp  InvalidSourceFormat.cpp  avg_channels_fil.cpp  SigprocFile.cpp -o avg_channels_fil -I $(MWA_FRB)/msfitslib/src// $(OPT) $(COMMON_LIBS)

merge_coarse_channels : merge_coarse_channels.cpp SigprocFile.cpp SigprocFile.h
	g++ CpuTimer.cpp  DataOrder.cpp  DataSource.cpp  InvalidSourceFormat.cpp  merge_coarse_channels.cpp SigprocFile.cpp -o merge_coarse_channels -I $(MWA_FRB)/msfitslib/src// $(OPT) $(COMMON_LIBS)

compare_filfiles : main_compare.cpp
	g++ CpuTimer.cpp  DataOrder.cpp  DataSource.cpp  InvalidSourceFormat.cpp  main_compare.cpp  SigprocFile.cpp -o compare_filfiles $(COMMON_LIBS)

update_fil_header : update_fil_header.cpp SigprocFile.cpp
	g++ update_fil_header.cpp CpuTimer.cpp  DataOrder.cpp  DataSource.cpp  InvalidSourceFormat.cpp  SigprocFile.cpp -o update_fil_header -I $(MWA_FRB)/msfitslib/src// $(OPT) $(COMMON_LIBS)


generate_fil_files : generate_fil_files.cpp SigprocFile.cpp random.cpp
	g++ generate_fil_files.cpp CpuTimer.cpp  DataOrder.cpp  DataSource.cpp  InvalidSourceFormat.cpp  SigprocFile.cpp random.cpp -o generate_fil_files -I $(MWA_FRB)/msfitslib/src// $(OPT) $(COMMON_LIBS)

n_fine_channels : n_fine_channels.cpp
	g++ n_fine_channels.cpp -o n_fine_channels

install :
	mkdir -p $(MWA_FRB)/bin/
	cp dumpfilfile $(MWA_FRB)/bin/
	cp dumpfilfile_float $(MWA_FRB)/bin/
#	cp fits2fil_bad $(MWA_FRB)/bin/    
	cp fits2fil $(MWA_FRB)/bin/
	cp fits2fil2 $(MWA_FRB)/bin/
#	cp pipe_filfiles $(MWA_FRB)/bin/
	cp avg_channels_fil $(MWA_FRB)/bin/
	cp merge_coarse_channels $(MWA_FRB)/bin/
	cp compare_filfiles $(MWA_FRB)/bin/
	cp update_fil_header $(MWA_FRB)/bin/
	cp dada2fits  $(MWA_FRB)/bin/
	cp n_fine_channels  $(MWA_FRB)/bin/
	chmod +x $(MWA_FRB)/bin/*
	
clean :
	rm -f *.o read_dada read_eda2_dada dada2fits dumpfilfile_float dumpfilfile_float_test generate_fil_files update_fil_header compare_filfiles merge_coarse_channels avg_channels_fil pipe_filfiles fits2fil fits2fil2 dumpfilfile n_fine_channels
		