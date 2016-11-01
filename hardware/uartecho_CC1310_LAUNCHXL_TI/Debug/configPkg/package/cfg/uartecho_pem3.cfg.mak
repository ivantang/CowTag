# invoke SourceDir generated makefile for uartecho.pem3
uartecho.pem3: .libraries,uartecho.pem3
.libraries,uartecho.pem3: package/cfg/uartecho_pem3.xdl
	$(MAKE) -f C:\Users\Ivan\workspace_v6_2\uartecho_CC1310_LAUNCHXL_TI/src/makefile.libs

clean::
	$(MAKE) -f C:\Users\Ivan\workspace_v6_2\uartecho_CC1310_LAUNCHXL_TI/src/makefile.libs clean

