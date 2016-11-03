################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Each subdirectory must supply rules for building sources it contributes
CC1310_LAUNCHXL.obj: ../CC1310_LAUNCHXL.c $(GEN_OPTS) | $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: ARM Compiler'
	"/home/ivan/ti/ccsv6/tools/compiler/arm_15.12.3.LTS/bin/armcl" -mv7M3 --code_state=16 --float_support=vfplib -me --include_path="/home/ivan/workspace_v6_2/betaTag" --include_path="/home/ivan/ti/tirtos_cc13xx_cc26xx_2_21_00_06/products/cc13xxware_2_04_03_17272" --include_path="/home/ivan/ti/ccsv6/tools/compiler/arm_15.12.3.LTS/include" -g --define=ccs --display_error_number --diag_warning=225 --diag_warning=255 --diag_wrap=off --gen_func_subsections=on --abi=eabi --preproc_with_compile --preproc_dependency="CC1310_LAUNCHXL.d" $(GEN_OPTS__FLAG) "$(shell echo $<)"
	@echo 'Finished building: $<'
	@echo ' '

ccfg.obj: ../ccfg.c $(GEN_OPTS) | $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: ARM Compiler'
	"/home/ivan/ti/ccsv6/tools/compiler/arm_15.12.3.LTS/bin/armcl" -mv7M3 --code_state=16 --float_support=vfplib -me --include_path="/home/ivan/workspace_v6_2/betaTag" --include_path="/home/ivan/ti/tirtos_cc13xx_cc26xx_2_21_00_06/products/cc13xxware_2_04_03_17272" --include_path="/home/ivan/ti/ccsv6/tools/compiler/arm_15.12.3.LTS/include" -g --define=ccs --display_error_number --diag_warning=225 --diag_warning=255 --diag_wrap=off --gen_func_subsections=on --abi=eabi --preproc_with_compile --preproc_dependency="ccfg.d" $(GEN_OPTS__FLAG) "$(shell echo $<)"
	@echo 'Finished building: $<'
	@echo ' '

uartecho.obj: ../uartecho.c $(GEN_OPTS) | $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: ARM Compiler'
	"/home/ivan/ti/ccsv6/tools/compiler/arm_15.12.3.LTS/bin/armcl" -mv7M3 --code_state=16 --float_support=vfplib -me --include_path="/home/ivan/workspace_v6_2/betaTag" --include_path="/home/ivan/ti/tirtos_cc13xx_cc26xx_2_21_00_06/products/cc13xxware_2_04_03_17272" --include_path="/home/ivan/ti/ccsv6/tools/compiler/arm_15.12.3.LTS/include" -g --define=ccs --display_error_number --diag_warning=225 --diag_warning=255 --diag_wrap=off --gen_func_subsections=on --abi=eabi --preproc_with_compile --preproc_dependency="uartecho.d" $(GEN_OPTS__FLAG) "$(shell echo $<)"
	@echo 'Finished building: $<'
	@echo ' '

build-37901247: ../uartecho.cfg
	@echo 'Building file: $<'
	@echo 'Invoking: XDCtools'
	"/home/ivan/ti/xdctools_3_32_00_06_core/xs" --xdcpath="/home/ivan/ti/tirtos_cc13xx_cc26xx_2_21_00_06/packages;/home/ivan/ti/tirtos_cc13xx_cc26xx_2_21_00_06/products/tidrivers_cc13xx_cc26xx_2_21_00_04/packages;/home/ivan/ti/tirtos_cc13xx_cc26xx_2_21_00_06/products/bios_6_46_01_37/packages;/home/ivan/ti/tirtos_cc13xx_cc26xx_2_21_00_06/products/uia_2_01_00_01/packages;/home/ivan/ti/ccsv6/ccs_base;" xdc.tools.configuro -o configPkg -t ti.targets.arm.elf.M3 -p ti.platforms.simplelink:CC1310F128 -r release -c "/home/ivan/ti/ccsv6/tools/compiler/arm_15.12.3.LTS" --compileOptions "-mv7M3 --code_state=16 --float_support=vfplib -me --include_path=\"/home/ivan/workspace_v6_2/betaTag\" --include_path=\"/home/ivan/ti/tirtos_cc13xx_cc26xx_2_21_00_06/products/cc13xxware_2_04_03_17272\" --include_path=\"/home/ivan/ti/ccsv6/tools/compiler/arm_15.12.3.LTS/include\" -g --define=ccs --display_error_number --diag_warning=225 --diag_warning=255 --diag_wrap=off --gen_func_subsections=on --abi=eabi  " "$<"
	@echo 'Finished building: $<'
	@echo ' '

configPkg/linker.cmd: build-37901247
configPkg/compiler.opt: build-37901247
configPkg/: build-37901247


