################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Each subdirectory must supply rules for building sources it contributes
CC1310_LAUNCHXL.obj: ../CC1310_LAUNCHXL.c $(GEN_OPTS) | $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: ARM Compiler'
	"D:/ti/ccsv6/tools/compiler/arm_15.12.3.LTS/bin/armcl" -mv7M3 --code_state=16 --float_support=vfplib -me --include_path="D:/GitHub/72point5/hardware/CowTags" --include_path="D:/GitHub/72point5/hardware/CowTags/easylink" --include_path="D:/GitHub/72point5/hardware/CowTags/smartrf_settings" --include_path="D:/ti/tirtos_cc13xx_cc26xx_2_21_00_06/products/cc13xxware_2_04_03_17272" --include_path="D:/ti/ccsv6/tools/compiler/arm_15.12.3.LTS/include" --include_path="D:/TICLOUDAGENT" -g --define=ccs --diag_wrap=off --diag_warning=225 --diag_warning=255 --display_error_number --gen_func_subsections=on --abi=eabi --preproc_with_compile --preproc_dependency="CC1310_LAUNCHXL.d" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

IIC.obj: ../IIC.c $(GEN_OPTS) | $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: ARM Compiler'
	"D:/ti/ccsv6/tools/compiler/arm_15.12.3.LTS/bin/armcl" -mv7M3 --code_state=16 --float_support=vfplib -me --include_path="D:/GitHub/72point5/hardware/CowTags" --include_path="D:/GitHub/72point5/hardware/CowTags/easylink" --include_path="D:/GitHub/72point5/hardware/CowTags/smartrf_settings" --include_path="D:/ti/tirtos_cc13xx_cc26xx_2_21_00_06/products/cc13xxware_2_04_03_17272" --include_path="D:/ti/ccsv6/tools/compiler/arm_15.12.3.LTS/include" --include_path="D:/TICLOUDAGENT" -g --define=ccs --diag_wrap=off --diag_warning=225 --diag_warning=255 --display_error_number --gen_func_subsections=on --abi=eabi --preproc_with_compile --preproc_dependency="IIC.d" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

RadioReceive.obj: ../RadioReceive.c $(GEN_OPTS) | $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: ARM Compiler'
	"D:/ti/ccsv6/tools/compiler/arm_15.12.3.LTS/bin/armcl" -mv7M3 --code_state=16 --float_support=vfplib -me --include_path="D:/GitHub/72point5/hardware/CowTags" --include_path="D:/GitHub/72point5/hardware/CowTags/easylink" --include_path="D:/GitHub/72point5/hardware/CowTags/smartrf_settings" --include_path="D:/ti/tirtos_cc13xx_cc26xx_2_21_00_06/products/cc13xxware_2_04_03_17272" --include_path="D:/ti/ccsv6/tools/compiler/arm_15.12.3.LTS/include" --include_path="D:/TICLOUDAGENT" -g --define=ccs --diag_wrap=off --diag_warning=225 --diag_warning=255 --display_error_number --gen_func_subsections=on --abi=eabi --preproc_with_compile --preproc_dependency="RadioReceive.d" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

RadioSend.obj: ../RadioSend.c $(GEN_OPTS) | $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: ARM Compiler'
	"D:/ti/ccsv6/tools/compiler/arm_15.12.3.LTS/bin/armcl" -mv7M3 --code_state=16 --float_support=vfplib -me --include_path="D:/GitHub/72point5/hardware/CowTags" --include_path="D:/GitHub/72point5/hardware/CowTags/easylink" --include_path="D:/GitHub/72point5/hardware/CowTags/smartrf_settings" --include_path="D:/ti/tirtos_cc13xx_cc26xx_2_21_00_06/products/cc13xxware_2_04_03_17272" --include_path="D:/ti/ccsv6/tools/compiler/arm_15.12.3.LTS/include" --include_path="D:/TICLOUDAGENT" -g --define=ccs --diag_wrap=off --diag_warning=225 --diag_warning=255 --display_error_number --gen_func_subsections=on --abi=eabi --preproc_with_compile --preproc_dependency="RadioSend.d" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

alphaMain.obj: ../alphaMain.c $(GEN_OPTS) | $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: ARM Compiler'
	"D:/ti/ccsv6/tools/compiler/arm_15.12.3.LTS/bin/armcl" -mv7M3 --code_state=16 --float_support=vfplib -me --include_path="D:/GitHub/72point5/hardware/CowTags" --include_path="D:/GitHub/72point5/hardware/CowTags/easylink" --include_path="D:/GitHub/72point5/hardware/CowTags/smartrf_settings" --include_path="D:/ti/tirtos_cc13xx_cc26xx_2_21_00_06/products/cc13xxware_2_04_03_17272" --include_path="D:/ti/ccsv6/tools/compiler/arm_15.12.3.LTS/include" --include_path="D:/TICLOUDAGENT" -g --define=ccs --diag_wrap=off --diag_warning=225 --diag_warning=255 --display_error_number --gen_func_subsections=on --abi=eabi --preproc_with_compile --preproc_dependency="alphaMain.d" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

alphaRadioTest.obj: ../alphaRadioTest.c $(GEN_OPTS) | $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: ARM Compiler'
	"D:/ti/ccsv6/tools/compiler/arm_15.12.3.LTS/bin/armcl" -mv7M3 --code_state=16 --float_support=vfplib -me --include_path="D:/GitHub/72point5/hardware/CowTags" --include_path="D:/GitHub/72point5/hardware/CowTags/easylink" --include_path="D:/GitHub/72point5/hardware/CowTags/smartrf_settings" --include_path="D:/ti/tirtos_cc13xx_cc26xx_2_21_00_06/products/cc13xxware_2_04_03_17272" --include_path="D:/ti/ccsv6/tools/compiler/arm_15.12.3.LTS/include" --include_path="D:/TICLOUDAGENT" -g --define=ccs --diag_wrap=off --diag_warning=225 --diag_warning=255 --display_error_number --gen_func_subsections=on --abi=eabi --preproc_with_compile --preproc_dependency="alphaRadioTest.d" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

ccfg.obj: ../ccfg.c $(GEN_OPTS) | $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: ARM Compiler'
	"D:/ti/ccsv6/tools/compiler/arm_15.12.3.LTS/bin/armcl" -mv7M3 --code_state=16 --float_support=vfplib -me --include_path="D:/GitHub/72point5/hardware/CowTags" --include_path="D:/GitHub/72point5/hardware/CowTags/easylink" --include_path="D:/GitHub/72point5/hardware/CowTags/smartrf_settings" --include_path="D:/ti/tirtos_cc13xx_cc26xx_2_21_00_06/products/cc13xxware_2_04_03_17272" --include_path="D:/ti/ccsv6/tools/compiler/arm_15.12.3.LTS/include" --include_path="D:/TICLOUDAGENT" -g --define=ccs --diag_wrap=off --diag_warning=225 --diag_warning=255 --display_error_number --gen_func_subsections=on --abi=eabi --preproc_with_compile --preproc_dependency="ccfg.d" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

config_parse.obj: ../config_parse.c $(GEN_OPTS) | $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: ARM Compiler'
	"D:/ti/ccsv6/tools/compiler/arm_15.12.3.LTS/bin/armcl" -mv7M3 --code_state=16 --float_support=vfplib -me --include_path="D:/GitHub/72point5/hardware/CowTags" --include_path="D:/GitHub/72point5/hardware/CowTags/easylink" --include_path="D:/GitHub/72point5/hardware/CowTags/smartrf_settings" --include_path="D:/ti/tirtos_cc13xx_cc26xx_2_21_00_06/products/cc13xxware_2_04_03_17272" --include_path="D:/ti/ccsv6/tools/compiler/arm_15.12.3.LTS/include" --include_path="D:/TICLOUDAGENT" -g --define=ccs --diag_wrap=off --diag_warning=225 --diag_warning=255 --display_error_number --gen_func_subsections=on --abi=eabi --preproc_with_compile --preproc_dependency="config_parse.d" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

build-1810874321: ../cowtagOS.cfg
	@echo 'Building file: $<'
	@echo 'Invoking: XDCtools'
	"D:/ti/xdctools_3_32_00_06_core/xs" --xdcpath="D:/ti/tirtos_cc13xx_cc26xx_2_21_00_06/packages;D:/ti/tirtos_cc13xx_cc26xx_2_21_00_06/products/tidrivers_cc13xx_cc26xx_2_21_00_04/packages;D:/ti/tirtos_cc13xx_cc26xx_2_21_00_06/products/bios_6_46_01_37/packages;D:/ti/tirtos_cc13xx_cc26xx_2_21_00_06/products/uia_2_01_00_01/packages;D:/ti/ccsv6/ccs_base;" xdc.tools.configuro -o configPkg -t ti.targets.arm.elf.M3 -p ti.platforms.simplelink:CC1310F128 -r release -c "D:/ti/ccsv6/tools/compiler/arm_15.12.3.LTS" --compileOptions "-mv7M3 --code_state=16 --float_support=vfplib -me --include_path=\"D:/GitHub/72point5/hardware/CowTags\" --include_path=\"D:/GitHub/72point5/hardware/CowTags/easylink\" --include_path=\"D:/GitHub/72point5/hardware/CowTags/smartrf_settings\" --include_path=\"D:/ti/tirtos_cc13xx_cc26xx_2_21_00_06/products/cc13xxware_2_04_03_17272\" --include_path=\"D:/ti/ccsv6/tools/compiler/arm_15.12.3.LTS/include\" --include_path=\"D:/TICLOUDAGENT\" -g --define=ccs --diag_wrap=off --diag_warning=225 --diag_warning=255 --display_error_number --gen_func_subsections=on --abi=eabi  " "$<"
	@echo 'Finished building: $<'
	@echo ' '

configPkg/linker.cmd: build-1810874321
configPkg/compiler.opt: build-1810874321
configPkg/: build-1810874321

eeprom.obj: ../eeprom.c $(GEN_OPTS) | $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: ARM Compiler'
	"D:/ti/ccsv6/tools/compiler/arm_15.12.3.LTS/bin/armcl" -mv7M3 --code_state=16 --float_support=vfplib -me --include_path="D:/GitHub/72point5/hardware/CowTags" --include_path="D:/GitHub/72point5/hardware/CowTags/easylink" --include_path="D:/GitHub/72point5/hardware/CowTags/smartrf_settings" --include_path="D:/ti/tirtos_cc13xx_cc26xx_2_21_00_06/products/cc13xxware_2_04_03_17272" --include_path="D:/ti/ccsv6/tools/compiler/arm_15.12.3.LTS/include" --include_path="D:/TICLOUDAGENT" -g --define=ccs --diag_wrap=off --diag_warning=225 --diag_warning=255 --display_error_number --gen_func_subsections=on --abi=eabi --preproc_with_compile --preproc_dependency="eeprom.d" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

eepromTest.obj: ../eepromTest.c $(GEN_OPTS) | $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: ARM Compiler'
	"D:/ti/ccsv6/tools/compiler/arm_15.12.3.LTS/bin/armcl" -mv7M3 --code_state=16 --float_support=vfplib -me --include_path="D:/GitHub/72point5/hardware/CowTags" --include_path="D:/GitHub/72point5/hardware/CowTags/easylink" --include_path="D:/GitHub/72point5/hardware/CowTags/smartrf_settings" --include_path="D:/ti/tirtos_cc13xx_cc26xx_2_21_00_06/products/cc13xxware_2_04_03_17272" --include_path="D:/ti/ccsv6/tools/compiler/arm_15.12.3.LTS/include" --include_path="D:/TICLOUDAGENT" -g --define=ccs --diag_wrap=off --diag_warning=225 --diag_warning=255 --display_error_number --gen_func_subsections=on --abi=eabi --preproc_with_compile --preproc_dependency="eepromTest.d" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

sensors.obj: ../sensors.c $(GEN_OPTS) | $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: ARM Compiler'
	"D:/ti/ccsv6/tools/compiler/arm_15.12.3.LTS/bin/armcl" -mv7M3 --code_state=16 --float_support=vfplib -me --include_path="D:/GitHub/72point5/hardware/CowTags" --include_path="D:/GitHub/72point5/hardware/CowTags/easylink" --include_path="D:/GitHub/72point5/hardware/CowTags/smartrf_settings" --include_path="D:/ti/tirtos_cc13xx_cc26xx_2_21_00_06/products/cc13xxware_2_04_03_17272" --include_path="D:/ti/ccsv6/tools/compiler/arm_15.12.3.LTS/include" --include_path="D:/TICLOUDAGENT" -g --define=ccs --diag_wrap=off --diag_warning=225 --diag_warning=255 --display_error_number --gen_func_subsections=on --abi=eabi --preproc_with_compile --preproc_dependency="sensors.d" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

serialize.obj: ../serialize.c $(GEN_OPTS) | $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: ARM Compiler'
	"D:/ti/ccsv6/tools/compiler/arm_15.12.3.LTS/bin/armcl" -mv7M3 --code_state=16 --float_support=vfplib -me --include_path="D:/GitHub/72point5/hardware/CowTags" --include_path="D:/GitHub/72point5/hardware/CowTags/easylink" --include_path="D:/GitHub/72point5/hardware/CowTags/smartrf_settings" --include_path="D:/ti/tirtos_cc13xx_cc26xx_2_21_00_06/products/cc13xxware_2_04_03_17272" --include_path="D:/ti/ccsv6/tools/compiler/arm_15.12.3.LTS/include" --include_path="D:/TICLOUDAGENT" -g --define=ccs --diag_wrap=off --diag_warning=225 --diag_warning=255 --display_error_number --gen_func_subsections=on --abi=eabi --preproc_with_compile --preproc_dependency="serialize.d" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

uArtEcho.obj: ../uArtEcho.c $(GEN_OPTS) | $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: ARM Compiler'
	"D:/ti/ccsv6/tools/compiler/arm_15.12.3.LTS/bin/armcl" -mv7M3 --code_state=16 --float_support=vfplib -me --include_path="D:/GitHub/72point5/hardware/CowTags" --include_path="D:/GitHub/72point5/hardware/CowTags/easylink" --include_path="D:/GitHub/72point5/hardware/CowTags/smartrf_settings" --include_path="D:/ti/tirtos_cc13xx_cc26xx_2_21_00_06/products/cc13xxware_2_04_03_17272" --include_path="D:/ti/ccsv6/tools/compiler/arm_15.12.3.LTS/include" --include_path="D:/TICLOUDAGENT" -g --define=ccs --diag_wrap=off --diag_warning=225 --diag_warning=255 --display_error_number --gen_func_subsections=on --abi=eabi --preproc_with_compile --preproc_dependency="uArtEcho.d" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '


