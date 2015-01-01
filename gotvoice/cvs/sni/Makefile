# Variables

# External build directories.
EXTERNAL_INCLUDE_DIR		= ../include
EXTERNAL_LIB_DIR		= ../lib


# External files needed for the build.
NAVITE_FFTW_HEADER		= dfftw.h
NAVITE_RFFTW_HEADER		= drfftw.h
NAVITE_FFTW_LIBRARY		= libdfftw.a
NAVITE_RFFTW_LIBRARY		= libdrfftw.a

# We look for a different signature that the one given to us.
RECOGNIZED_FFTW_HEADER		= fftw.h
RECOGNIZED_RFFTW_HEADER		= rfftw.h
RECOGNIZED_FFTW_LIBRARY		= libfftw.a
RECOGNIZED_RFFTW_LIBRARY	= librfftw.a



all:	headers snicmp snicut snggen snigen sngcid sniview snicluster
	(cd bison; make)

install: all
	(cd bison; make -i install)
	-mv /usr/local/bin/snicmp /usr/local/bin/snicmp-
	-mv /usr/local/bin/snicluster /usr/local/bin/snicluster-
	cp makedigits snicmp snicluster snicut snggen snigen sngcid sniview /usr/local/bin
	chmod ugo+x /usr/local/bin/{snicmp,snicluster,snicut,snigen,snggen,sngcid,sniview}

install_cid:
	-rm -rf /var/gv/lib/{verizon,sprintpcs,twcable_proxy,cingular,attwgsm,cing_vzwla}
	-mkdir -p /var/gv/lib/{verizon,sprintpcs,twcable_proxy,cingular}
	cp callerid/verizon/*.sng /var/gv/lib/verizon
#	cp callerid/sprintpcs/*.sng /var/gv/lib/sprintpcs
	cp callerid/sprintpcs_chris/*.sng /var/gv/lib/sprintpcs
	cp callerid/twcable/*.sng /var/gv/lib/twcable_proxy
	cp callerid/cingular/*.sng /var/gv/lib/cingular
	ln -s /var/gv/lib/cingular /var/gv/lib/attwgsm
	ln -s /var/gv/lib/cingular /var/gv/lib/cing_vzwla

clean_cid:
	rm -f /var/gv/lib/{verizon,sprintpcs,twcable_proxy,cingular,attwgsm}

clean:	
	(cd bison; make clean)
	rm -f *.o
	-rm $(RECOGNIZED_FFTW_HEADER) $(RECOGNIZED_RFFTW_HEADER) $(RECOGNIZED_FFTW_LIBRARY) $(RECOGNIZED_RFFTW_LIBRARY) $(NAVITE_FFTW_HEADER) $(NAVITE_RFFTW_HEADER)
	rm -f snicmp snicut snggen snigen sngcid snicluster

headers:
	-rm $(RECOGNIZED_FFTW_HEADER) $(RECOGNIZED_RFFTW_HEADER) $(RECOGNIZED_FFTW_LIBRARY) $(RECOGNIZED_RFFTW_LIBRARY) $(NAVITE_FFTW_HEADER) $(NAVITE_RFFTW_HEADER)
	# Somethings reference the native headers, so bring those in first.
	ln -s $(EXTERNAL_INCLUDE_DIR)/$(NAVITE_FFTW_HEADER) $(NAVITE_FFTW_HEADER)
	ln -s $(EXTERNAL_INCLUDE_DIR)/$(NAVITE_RFFTW_HEADER) $(NAVITE_RFFTW_HEADER) 
	# Now bring in the stuff as we mostly recognize is.
	ln -s $(NAVITE_FFTW_HEADER) $(RECOGNIZED_FFTW_HEADER)
	ln -s $(NAVITE_RFFTW_HEADER) $(RECOGNIZED_RFFTW_HEADER)
	ln -s $(EXTERNAL_LIB_DIR)/$(NAVITE_FFTW_LIBRARY) $(RECOGNIZED_FFTW_LIBRARY)
	ln -s $(EXTERNAL_LIB_DIR)/$(NAVITE_RFFTW_LIBRARY) $(RECOGNIZED_RFFTW_LIBRARY)

snicut: snicut.o engine.o signature.o
	g++ -o snicut snicut.o  signature.o engine.o $(RECOGNIZED_RFFTW_LIBRARY) $(RECOGNIZED_FFTW_LIBRARY)

snicut.o: snicut.cc sni.h wave.h
	g++ -g -c -DLINUX snicut.cc

sngcid: sngcid.o signature.o engine.o 
	g++ -o sngcid sngcid.o signature.o engine.o $(RECOGNIZED_RFFTW_LIBRARY) $(RECOGNIZED_FFTW_LIBRARY)

sngcid.o: sngcid.cc sni.h wave.h sni_engine.h sni_sig.h
	g++ -g -c -DLINUX sngcid.cc

snigen: snigen.o sni_sig.o sni_engine.o sni_sig.h sni_engine.h sni.h 
	g++ -o snigen snigen.o sni_sig.o sni_engine.o $(RECOGNIZED_RFFTW_LIBRARY) $(RECOGNIZED_FFTW_LIBRARY)

snigen.o: snigen.cc sni.h wave.h sni_engine.h sni_sig.h
	g++ -g -c -DLINUX snigen.cc

sniview: sniview.o sni_sig.o sni_engine.o sni_sig.h sni_engine.h sni.h 
	g++ -o sniview sniview.o sni_sig.o sni_engine.o $(RECOGNIZED_RFFTW_LIBRARY) $(RECOGNIZED_FFTW_LIBRARY)

sniview.o: sniview.cc sni.h wave.h sni_engine.h sni_sig.h
	g++ -g -c -DLINUX sniview.cc

snggen: snggen.o signature.o engine.o 
	g++ -o snggen snggen.o signature.o engine.o $(RECOGNIZED_RFFTW_LIBRARY) $(RECOGNIZED_FFTW_LIBRARY)

snggen.o: snggen.cc sni.h wave.h
	g++ -g -c -DLINUX snggen.cc

snicmp: snicmp.o sni_sig.o sni_engine.o sni_sig.h sni_engine.h sni.h 
	g++ -o snicmp snicmp.o sni_sig.o sni_engine.o $(RECOGNIZED_RFFTW_LIBRARY) $(RECOGNIZED_FFTW_LIBRARY)

snicmp.o: snicmp.cc sni.h wave.h sni_engine.h sni_sig.h
	g++ -g -c -DLINUX snicmp.cc

snicluster: snicluster.o sni_sig.o sni_engine.o sni_sig.h sni_engine.h sni.h 
	g++ -o snicluster snicluster.o sni_sig.o sni_engine.o $(RECOGNIZED_RFFTW_LIBRARY) $(RECOGNIZED_FFTW_LIBRARY)

snicluster.o: snicluster.cc sni.h wave.h sni_engine.h sni_sig.h
	g++ -g -c -DLINUX snicluster.cc

engine.o: engine.cc engine.h sni.h signature.h enginestruct.h
	g++ -g -O3 -funroll-loops  -c -DLINUX $(X) engine.cc

signature.o: signature.cc engine.h enginestruct.h
	g++ -g -c -DLINUX  signature.cc

sni_engine.o: sni_engine.cc sni_engine.h sni_sig.h sni.h signature.h
	g++ -g -O3 -funroll-loops  -c -DLINUX $(X) sni_engine.cc

sni_sig.o: sni_sig.cc sni_engine.h sni_sig.h sni.h
	g++ -g -c -DLINUX  sni_sig.cc
