# invoke SourceDir generated makefile for uartecho.pem3
uartecho.pem3: .libraries,uartecho.pem3
.libraries,uartecho.pem3: package/cfg/uartecho_pem3.xdl
	$(MAKE) -f /home/ivan/workspace_v6_2/betaTag/src/makefile.libs

clean::
	$(MAKE) -f /home/ivan/workspace_v6_2/betaTag/src/makefile.libs clean

