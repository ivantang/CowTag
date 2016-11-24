################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Each subdirectory must supply rules for building sources it contributes
CC1310_LAUNCHXL.obj: ../CC1310_LAUNCHXL.c $(GEN_OPTS) | $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: ARM Compiler'
	"C:/ti/ccsv6/tools/compiler/arm_15.12.3.LTS/bin/armcl" -mv7M3 --code_state=16 --float_support=vfplib -me --include_path="C:/Users/Ivan/Documents/GitHub/72point5/hardware/CowTags" --include_path="C:/Users/Ivan/Documents/GitHub/72point5/hardware/CowTags/easylink" --include_path="C:/Users/Ivan/Documents/GitHub/72point5/hardware/CowTags/smartrf_settings" --include_path="C:/ti/tirex-content/tirtos_cc13xx_cc26xx_2_21_00_06/products/cc13xxware_2_04_03_17272" --include_path="C:/ti/ccsv6/tools/compiler/arm_15.12.3.LTS/include" -g --define=ccs --diag_wrap=off --diag_warning=225 --diag_warning=255 --display_error_number --gen_func_subsections=on --abi=eabi --preproc_with_compile --preproc_dependency="CC1310_LAUNCHXL.d" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

IIC.obj: ../IIC.c $(GEN_OPTS) | $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: ARM Compiler'
	"C:/ti/ccsv6/tools/compiler/arm_15.12.3.LTS/bin/armcl" -mv7M3 --code_state=16 --float_support=vfplib -me --include_path="C:/Users/Ivan/Documents/GitHub/72point5/hardware/CowTags" --include_path="C:/Users/Ivan/Documents/GitHub/72point5/hardware/CowTags/easylink" --include_path="C:/Users/Ivan/Documents/GitHub/72point5/hardware/CowTags/smartrf_settings" --include_path="C:/ti/tirex-content/tirtos_cc13xx_cc26xx_2_21_00_06/products/cc13xxware_2_04_03_17272" --include_path="C:/ti/ccsv6/tools/compiler/arm_15.12.3.LTS/include" -g --define=ccs --diag_wrap=off --diag_warning=225 --diag_warning=255 --display_error_number --gen_func_subsections=on --abi=eabi --preproc_with_compile --preproc_dependency="IIC.d" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

RadioSend.obj: ../RadioSend.c $(GEN_OPTS) | $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: ARM Compiler'
	"C:/ti/ccsv6/tools/compiler/arm_15.12.3.LTS/bin/armcl" -mv7M3 --code_state=16 --float_support=vfplib -me --include_path="C:/Users/Ivan/Documents/GitHub/72point5/hardware/CowTags" --include_path="C:/Users/Ivan/Documents/GitHub/72point5/hardware/CowTags/easylink" --include_path="C:/Users/Ivan/Documents/GitHub/72point5/hardware/CowTags/smartrf_settings" --include_path="C:/ti/tirex-content/tirtos_cc13xx_cc26xx_2_21_00_06/products/cc13xxware_2_04_03_17272" --include_path="C:/ti/ccsv6/tools/compiler/arm_15.12.3.LTS/include" -g --define=ccs --diag_wrap=off --diag_warning=225 --diag_warning=255 --display_error_number --gen_func_subsections=on --abi=eabi --preproc_with_compile --preproc_dependency="RadioSend.d" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

betaMain.obj: ../betaMain.c $(GEN_OPTS) | $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: ARM Compiler'
	"C:/ti/ccsv6/tools/compiler/arm_15.12.3.LTS/bin/armcl" -mv7M3 --code_state=16 --float_support=vfplib -me --include_path="C:/Users/Ivan/Documents/GitHub/72point5/hardware/CowTags" --include_path="C:/Users/Ivan/Documents/GitHub/72point5/hardware/CowTags/easylink" --include_path="C:/Users/Ivan/Documents/GitHub/72point5/hardware/CowTags/smartrf_settings" --include_path="C:/ti/tirex-content/tirtos_cc13xx_cc26xx_2_21_00_06/products/cc13xxware_2_04_03_17272" --include_path="C:/ti/ccsv6/tools/compiler/arm_15.12.3.LTS/include" -g --define=ccs --diag_wrap=off --diag_warning=225 --diag_warning=255 --display_error_number --gen_func_subsections=on --abi=eabi --preproc_with_compile --preproc_dependency="betaMain.d" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

betaRadioTest.obj: ../betaRadioTest.c $(GEN_OPTS) | $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: ARM Compiler'
	"C:/ti/ccsv6/tools/compiler/arm_15.12.3.LTS/bin/armcl" -mv7M3 --code_state=16 --float_support=vfplib -me --include_path="C:/Users/Ivan/Documents/GitHub/72point5/hardware/CowTags" --include_path="C:/Users/Ivan/Documents/GitHub/72point5/hardware/CowTags/easylink" --include_path="C:/Users/Ivan/Documents/GitHub/72point5/hardware/CowTags/smartrf_settings" --include_path="C:/ti/tirex-content/tirtos_cc13xx_cc26xx_2_21_00_06/products/cc13xxware_2_04_03_17272" --include_path="C:/ti/ccsv6/tools/compiler/arm_15.12.3.LTS/include" -g --define=ccs --diag_wrap=off --diag_warning=225 --diag_warning=255 --display_error_number --gen_func_subsections=on --abi=eabi --preproc_with_compile --preproc_dependency="betaRadioTest.d" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

ccfg.obj: ../ccfg.c $(GEN_OPTS) | $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: ARM Compiler'
	"C:/ti/ccsv6/tools/compiler/arm_15.12.3.LTS/bin/armcl" -mv7M3 --code_state=16 --float_support=vfplib -me --include_path="C:/Users/Ivan/Documents/GitHub/72point5/hardware/CowTags" --include_path="C:/Users/Ivan/Documents/GitHub/72point5/hardware/CowTags/easylink" --include_path="C:/Users/Ivan/Documents/GitHub/72point5/hardware/CowTags/smartrf_settings" --include_path="C:/ti/tirex-content/tirtos_cc13xx_cc26xx_2_21_00_06/products/cc13xxware_2_04_03_17272" --include_path="C:/ti/ccsv6/tools/compiler/arm_15.12.3.LTS/include" -g --define=ccs --diag_wrap=off --diag_warning=225 --diag_warning=255 --display_error_number --gen_func_subsections=on --abi=eabi --preproc_with_compile --preproc_dependency="ccfg.d" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

build-1810874321: ../cowtagOS.cfg
	@echo 'Building file: $<'
	@echo 'Invoking: XDCtools'
	"C:/ti/tirex-content/xdctools_3_32_00_06_core/xs" --xdcpath="C:/ti/tirex-content/tirtos_cc13xx_cc26xx_2_21_00_06/packages;C:/ti/tirex-content/tirtos_cc13xx_cc26xx_2_21_00_06/products/tidrivers_cc13xx_cc26xx_2_21_00_04/packages;C:/ti/tirex-content/tirtos_cc13xx_cc26xx_2_21_00_06/products/bios_6_46_01_37/packages;C:/ti/tirex-content/tirtos_cc13xx_cc26xx_2_21_00_06/products/uia_2_01_00_01/packages;C:/ti/ccsv6/ccs_base;" xdc.tools.configuro -o configPkg -t ti.targets.arm.elf.M3 -p ti.platforms.simplelink:CC1310F128 -r release -c "C:/ti/ccsv6/tools/compiler/arm_15.12.3.LTS" --compileOptions "-mv7M3 --code_state=16 --float_support=vfplib -me --include_path=\"C:/Users/Ivan/Documents/GitHub/72point5/hardware/CowTags\" --include_path=\"C:/Users/Ivan/Documents/GitHub/72point5/hardware/CowTags/easylink\" --include_path=\"C:/Users/Ivan/Documents/GitHub/72point5/hardware/CowTags/smartrf_settings\" --include_path=\"C:/ti/tirex-content/tirtos_cc13xx_cc26xx_2_21_00_06/products/cc13xxware_2_04_03_17272\" --include_path=\"C:/ti/ccsv6/tools/compiler/arm_15.12.3.LTS/include\" -g --define=ccs --diag_wrap=off --diag_warning=225 --diag_warning=255 --display_error_number --gen_func_subsections=on --abi=eabi  " "$<"
	@echo 'Finished building: $<'
	@echo ' '

configPkg/linker.cmd: build-1810874321
configPkg/compiler.opt: build-1810874321
configPkg/: build-1810874321

sensors.obj: ../sensors.c $(GEN_OPTS) | $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: ARM Compiler'
	"C:/ti/ccsv6/tools/compiler/arm_15.12.3.LTS/bin/armcl" -mv7M3 --code_state=16 --float_support=vfplib -me --include_path="C:/Users/Ivan/Documents/GitHub/72point5/hardware/CowTags" --include_path="C:/Users/Ivan/Documents/GitHub/72point5/hardware/CowTags/easylink" --include_path="C:/Users/Ivan/Documents/GitHub/72point5/hardware/CowTags/smartrf_settings" --include_path="C:/ti/tirex-content/tirtos_cc13xx_cc26xx_2_21_00_06/products/cc13xxware_2_04_03_17272" --include_path="C:/ti/ccsv6/tools/compiler/arm_15.12.3.LTS/include" -g --define=ccs --diag_wrap=off --diag_warning=225 --diag_warning=255 --display_error_number --gen_func_subsections=on --abi=eabi --preproc_with_compile --preproc_dependency="sensors.d" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '


