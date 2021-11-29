# include ../../make.cfg/flags.mak

# CMN_INCLUDES=-I../../fitslib/
COMMON_LIBS=-L$(BIGHORNS)/lib -lfitslib $(NDIR)/slib/libbaselib.a $(NDIR)/slib/libmathlib.a  `root-config --libs`  -lcfitsio -lnova 


read_dada : read_dada.c 
	mkdir -p $(CRAFT)/bin/
	g++ read_dada.c $(OPT) -o read_dada

read_eda2_dada : read_eda2_dada.c
	g++ read_eda2_dada.c -o read_eda2_dada $(OPT)	

dada2fits : dada2fits.cpp
	g++  dada2fits.cpp -I$(BIGHORNS)/software/analysis/fitslib -lcfitsio -L$(BIGHORNS)/lib -lfitslib $(OPT) $(COMMON_LIBS) -o dada2fits
	cp dada2fits  $(CRAFT)/bin/
	

all : read_dada read_eda2_dada dada2fits compare_filfiles dumpfilfile_float merge_coarse_channels avg_channels_fil dumpfilfile read_filfile
	mkdir -p $(CRAFT)/bin/
	cp dumpfilfile $(CRAFT)/bin/	
	cp dumpfilfile_float $(CRAFT)/bin/    
	cp read_filfile $(CRAFT)/bin/
	cp read_dada $(CRAFT)/bin/
	cp read_eda2_dada $(CRAFT)/bin/
	cp merge_coarse_channels $(CRAFT)/bin/
	chmod +x $(CRAFT)/bin/*

read_filfile : read_filfile.c
	gcc read_filfile.c -o read_filfile $(OPT)

dumpfilfile : CpuTimer.cpp  DataOrder.cpp  DataSource.cpp  InvalidSourceFormat.cpp  main.cpp  SigprocFile.cpp 
	g++ CpuTimer.cpp  DataOrder.cpp  DataSource.cpp  InvalidSourceFormat.cpp  main.cpp  SigprocFile.cpp -o dumpfilfile -I $(BIGHORNS)/software/analysis/fitslib/ $(OPT) $(COMMON_LIBS)
	
dumpfilfile_float : main_float.cpp
	g++ CpuTimer.cpp  DataOrder.cpp  DataSource.cpp  InvalidSourceFormat.cpp  main_float.cpp  SigprocFile.cpp -o dumpfilfile_float -I $(BIGHORNS)/software/analysis/fitslib/ $(OPT) $(COMMON_LIBS)
	cp dumpfilfile_float $(CRAFT)/bin/    

pipe_filfiles : pipe_filfiles.cpp SigprocFile.cpp SigprocFile.h
	g++ CpuTimer.cpp  DataOrder.cpp  DataSource.cpp  InvalidSourceFormat.cpp  pipe_filfiles.cpp  SigprocFile.cpp -o pipe_filfiles -I $(BIGHORNS)/software/analysis/fitslib/ $(OPT) $(COMMON_LIBS)
	cp pipe_filfiles $(CRAFT)/bin/    
	
avg_channels_fil : avg_channels_fil.cpp SigprocFile.cpp SigprocFile.h
	g++ CpuTimer.cpp  DataOrder.cpp  DataSource.cpp  InvalidSourceFormat.cpp  avg_channels_fil.cpp  SigprocFile.cpp -o avg_channels_fil -I $(BIGHORNS)/software/analysis/fitslib/ $(OPT) $(COMMON_LIBS)
	cp avg_channels_fil $(CRAFT)/bin/    

merge_coarse_channels : merge_coarse_channels.cpp SigprocFile.cpp SigprocFile.h
	g++ CpuTimer.cpp  DataOrder.cpp  DataSource.cpp  InvalidSourceFormat.cpp  merge_coarse_channels.cpp SigprocFile.cpp -o merge_coarse_channels -I $(BIGHORNS)/software/analysis/fitslib/ $(OPT) $(COMMON_LIBS)
	cp merge_coarse_channels $(CRAFT)/bin/     

compare_filfiles : main_compare.cpp
	g++ CpuTimer.cpp  DataOrder.cpp  DataSource.cpp  InvalidSourceFormat.cpp  main_compare.cpp  SigprocFile.cpp -o compare_filfiles
	cp compare_filfiles $(CRAFT)/bin/

update_fil_header : update_fil_header.cpp SigprocFile.cpp
	g++ update_fil_header.cpp CpuTimer.cpp  DataOrder.cpp  DataSource.cpp  InvalidSourceFormat.cpp  SigprocFile.cpp -o update_fil_header -I $(BIGHORNS)/software/analysis/fitslib/ $(OPT) $(COMMON_LIBS)
	cp update_fil_header $(CRAFT)/bin/


generate_fil_files : generate_fil_files.cpp SigprocFile.cpp random.cpp
	g++ generate_fil_files.cpp CpuTimer.cpp  DataOrder.cpp  DataSource.cpp  InvalidSourceFormat.cpp  SigprocFile.cpp random.cpp -o generate_fil_files -I $(BIGHORNS)/software/analysis/fitslib/ $(OPT) $(COMMON_LIBS)
	cp generate_fil_files $(CRAFT)/bin/
	
clean :
	rm -f *.o read_dada read_eda2_dada dada2fits dumpfilfile_float generate_fil_files update_fil_header compare_filfiles merge_coarse_channels avg_channels_fil pipe_filfiles 
		