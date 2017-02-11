# invoke SourceDir generated makefile for cowtagOS.pem3
cowtagOS.pem3: .libraries,cowtagOS.pem3
.libraries,cowtagOS.pem3: package/cfg/cowtagOS_pem3.xdl
	$(MAKE) -f /home/steven/Documents/school-assignments/cpen491/72point5/hardware/CowTags/src/makefile.libs

clean::
	$(MAKE) -f /home/steven/Documents/school-assignments/cpen491/72point5/hardware/CowTags/src/makefile.libs clean

