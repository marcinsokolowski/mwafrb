# include ../../make.cfg/flags.mak

# CMN_INCLUDES=-I../../fitslib/
COMMON_LIBS=-L$(MWA_FRB)/lib/ -lmsfitslib $(MWA_FRB)/lib/libmscommonlib.a -lcfitsio -lnova -lfftw3


read_dada : read_dada.c 
	mkdir -p $(MWA_FRB)/bin/
	g++ read_dada.c $(OPT) -o read_dada

read_eda2_dada : read_eda2_dada.c
	g++ read_eda2_dada.c -o read_eda2_dada $(OPT)	

dada2fits : dada2fits.cpp
	g++  dada2fits.cpp -I$(MWA_FRB)/msfitslib/src/ -lcfitsio -L$(MWA_FRB)/slib/ -lmsfitslib $(OPT) $(COMMON_LIBS) -o dada2fits
	cp dada2fits  $(MWA_FRB)/bin/
	

all : read_dada read_eda2_dada dada2fits compare_filfiles dumpfilfile_float merge_coarse_channels avg_channels_fil dumpfilfile read_filfile fits2fil
	mkdir -p $(MWA_FRB)/bin/
	cp dumpfilfile $(MWA_FRB)/bin/	
	cp dumpfilfile_float $(MWA_FRB)/bin/    
	cp read_filfile $(MWA_FRB)/bin/
	cp read_dada $(MWA_FRB)/bin/
	cp read_eda2_dada $(MWA_FRB)/bin/
	cp merge_coarse_channels $(MWA_FRB)/bin/
	cp fits2fil $(MWA_FRB)/bin/
	chmod +x $(MWA_FRB)/bin/*

read_filfile : read_filfile.c
	gcc read_filfile.c -o read_filfile $(OPT)

dumpfilfile : CpuTimer.cpp  DataOrder.cpp  DataSource.cpp  InvalidSourceFormat.cpp  main.cpp  SigprocFile.cpp 
	g++ CpuTimer.cpp  DataOrder.cpp  DataSource.cpp  InvalidSourceFormat.cpp  main.cpp  SigprocFile.cpp -o dumpfilfile -I $(MWA_FRB)/msfitslib/src// $(OPT) $(COMMON_LIBS)
	
dumpfilfile_float : main_float.cpp
	g++ CpuTimer.cpp  DataOrder.cpp  DataSource.cpp  InvalidSourceFormat.cpp  main_float.cpp  SigprocFile.cpp -o dumpfilfile_float -I $(MWA_FRB)/msfitslib/src// $(OPT) $(COMMON_LIBS)
	cp dumpfilfile_float $(MWA_FRB)/bin/    

fits2fil : fits2fil.cpp
	g++ CpuTimer.cpp  DataOrder.cpp  DataSource.cpp  InvalidSourceFormat.cpp  fits2fil.cpp  SigprocFile.cpp -o fits2fil -I $(MWA_FRB)/msfitslib/src// $(OPT) $(COMMON_LIBS)
	cp fits2fil $(MWA_FRB)/bin/    

pipe_filfiles : pipe_filfiles.cpp SigprocFile.cpp SigprocFile.h
	g++ CpuTimer.cpp  DataOrder.cpp  DataSource.cpp  InvalidSourceFormat.cpp  pipe_filfiles.cpp  SigprocFile.cpp -o pipe_filfiles -I $(MWA_FRB)/msfitslib/src// $(OPT) $(COMMON_LIBS)
	cp pipe_filfiles $(MWA_FRB)/bin/    
	
avg_channels_fil : avg_channels_fil.cpp SigprocFile.cpp SigprocFile.h
	g++ CpuTimer.cpp  DataOrder.cpp  DataSource.cpp  InvalidSourceFormat.cpp  avg_channels_fil.cpp  SigprocFile.cpp -o avg_channels_fil -I $(MWA_FRB)/msfitslib/src// $(OPT) $(COMMON_LIBS)
	cp avg_channels_fil $(MWA_FRB)/bin/    

merge_coarse_channels : merge_coarse_channels.cpp SigprocFile.cpp SigprocFile.h
	g++ CpuTimer.cpp  DataOrder.cpp  DataSource.cpp  InvalidSourceFormat.cpp  merge_coarse_channels.cpp SigprocFile.cpp -o merge_coarse_channels -I $(MWA_FRB)/msfitslib/src// $(OPT) $(COMMON_LIBS)
	cp merge_coarse_channels $(MWA_FRB)/bin/     

compare_filfiles : main_compare.cpp
	g++ CpuTimer.cpp  DataOrder.cpp  DataSource.cpp  InvalidSourceFormat.cpp  main_compare.cpp  SigprocFile.cpp -o compare_filfiles
	cp compare_filfiles $(MWA_FRB)/bin/

update_fil_header : update_fil_header.cpp SigprocFile.cpp
	g++ update_fil_header.cpp CpuTimer.cpp  DataOrder.cpp  DataSource.cpp  InvalidSourceFormat.cpp  SigprocFile.cpp -o update_fil_header -I $(MWA_FRB)/msfitslib/src// $(OPT) $(COMMON_LIBS)
	cp update_fil_header $(MWA_FRB)/bin/


generate_fil_files : generate_fil_files.cpp SigprocFile.cpp random.cpp
	g++ generate_fil_files.cpp CpuTimer.cpp  DataOrder.cpp  DataSource.cpp  InvalidSourceFormat.cpp  SigprocFile.cpp random.cpp -o generate_fil_files -I $(MWA_FRB)/msfitslib/src// $(OPT) $(COMMON_LIBS)
	cp generate_fil_files $(MWA_FRB)/bin/
	
clean :
	rm -f *.o read_dada read_eda2_dada dada2fits dumpfilfile_float generate_fil_files update_fil_header compare_filfiles merge_coarse_channels avg_channels_fil pipe_filfiles fits2fil
		