################################################################################
# Automatically-generated file. Do not edit!
################################################################################

CG_TOOL_ROOT := /opt/ti/ccsv6/tools/compiler/arm_15.12.3.LTS

GEN_OPTS__FLAG := --cmd_file="configPkg/compiler.opt"
GEN_CMDS__FLAG := -l"configPkg/linker.cmd"

ORDERED_OBJS += \
"./CC1310_LAUNCHXL.obj" \
"./EventManager.obj" \
"./IIC.obj" \
"./Sleep.obj" \
"./alphaMain.obj" \
"./alphaRadioTest.obj" \
"./bootTimestamp.obj" \
"./ccfg.obj" \
"./config_parse.obj" \
"./eeprom.obj" \
"./eepromTest.obj" \
"./radioSendReceive.obj" \
"./sensors.obj" \
"./serialize.obj" \
"./uArtEcho.obj" \
"./easylink/EasyLink.obj" \
"./smartrf_settings/smartrf_settings.obj" \
"./smartrf_settings/smartrf_settings_predefined.obj" \
"../CC1310_LAUNCHXL.cmd" \
$(GEN_CMDS__FLAG) \
-l"/opt/ti/tirtos_cc13xx_cc26xx_2_21_00_06/products/cc13xxware_2_04_03_17272/driverlib/bin/ccs/driverlib.lib" \
-llibc.a \

-include ../makefile.init

RM := rm -rf
RMDIR := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include subdir_vars.mk
-include easylink/subdir_vars.mk
-include smartrf_settings/subdir_vars.mk
-include subdir_rules.mk
-include easylink/subdir_rules.mk
-include smartrf_settings/subdir_rules.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(S_DEPS)),)
-include $(S_DEPS)
endif
ifneq ($(strip $(S_UPPER_DEPS)),)
-include $(S_UPPER_DEPS)
endif
ifneq ($(strip $(S62_DEPS)),)
-include $(S62_DEPS)
endif
ifneq ($(strip $(C64_DEPS)),)
-include $(C64_DEPS)
endif
ifneq ($(strip $(ASM_DEPS)),)
-include $(ASM_DEPS)
endif
ifneq ($(strip $(CC_DEPS)),)
-include $(CC_DEPS)
endif
ifneq ($(strip $(SV7A_DEPS)),)
-include $(SV7A_DEPS)
endif
ifneq ($(strip $(S55_DEPS)),)
-include $(S55_DEPS)
endif
ifneq ($(strip $(C67_DEPS)),)
-include $(C67_DEPS)
endif
ifneq ($(strip $(CLA_DEPS)),)
-include $(CLA_DEPS)
endif
ifneq ($(strip $(C??_DEPS)),)
-include $(C??_DEPS)
endif
ifneq ($(strip $(CPP_DEPS)),)
-include $(CPP_DEPS)
endif
ifneq ($(strip $(S??_DEPS)),)
-include $(S??_DEPS)
endif
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
ifneq ($(strip $(C62_DEPS)),)
-include $(C62_DEPS)
endif
ifneq ($(strip $(CXX_DEPS)),)
-include $(CXX_DEPS)
endif
ifneq ($(strip $(C++_DEPS)),)
-include $(C++_DEPS)
endif
ifneq ($(strip $(ASM_UPPER_DEPS)),)
-include $(ASM_UPPER_DEPS)
endif
ifneq ($(strip $(K_DEPS)),)
-include $(K_DEPS)
endif
ifneq ($(strip $(C43_DEPS)),)
-include $(C43_DEPS)
endif
ifneq ($(strip $(INO_DEPS)),)
-include $(INO_DEPS)
endif
ifneq ($(strip $(S67_DEPS)),)
-include $(S67_DEPS)
endif
ifneq ($(strip $(SA_DEPS)),)
-include $(SA_DEPS)
endif
ifneq ($(strip $(S43_DEPS)),)
-include $(S43_DEPS)
endif
ifneq ($(strip $(OPT_DEPS)),)
-include $(OPT_DEPS)
endif
ifneq ($(strip $(PDE_DEPS)),)
-include $(PDE_DEPS)
endif
ifneq ($(strip $(S64_DEPS)),)
-include $(S64_DEPS)
endif
ifneq ($(strip $(C_UPPER_DEPS)),)
-include $(C_UPPER_DEPS)
endif
ifneq ($(strip $(C55_DEPS)),)
-include $(C55_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 
EXE_OUTPUTS += \
CowTag.out \

EXE_OUTPUTS__QUOTED += \
"CowTag.out" \

BIN_OUTPUTS += \
CowTag.hex \

BIN_OUTPUTS__QUOTED += \
"CowTag.hex" \


# All Target
all:
	@$(MAKE) --no-print-directory pre-build
	@$(MAKE) --no-print-directory main-build

# Main-build Target
main-build: CowTag.out

# Tool invocations
CowTag.out: $(OBJS) $(CMD_SRCS) $(GEN_CMDS)
	@echo 'Building target: $@'
	@echo 'Invoking: ARM Linker'
	"/opt/ti/ccsv6/tools/compiler/arm_15.12.3.LTS/bin/armcl" -mv7M3 --code_state=16 --float_support=vfplib -me -g --define=ccs --display_error_number --diag_warning=225 --diag_warning=255 --diag_wrap=off --gen_func_subsections=on --abi=eabi -z -m"CowTag.map" --heap_size=0 --stack_size=256 -i"/opt/ti/ccsv6/tools/compiler/arm_15.12.3.LTS/lib" -i"/opt/ti/ccsv6/tools/compiler/arm_15.12.3.LTS/include" --reread_libs --warn_sections --display_error_number --diag_wrap=off --xml_link_info="CowTag_linkInfo.xml" --rom_model -o "CowTag.out" $(ORDERED_OBJS)
	@echo 'Finished building target: $@'
	@echo ' '

CowTag.hex: $(EXE_OUTPUTS)
	@echo 'Invoking: ARM Hex Utility'
	"/opt/ti/ccsv6/tools/compiler/arm_15.12.3.LTS/bin/armhex" --memwidth=8 --romwidth=8 --intel -o "CowTag.hex" $(EXE_OUTPUTS__QUOTED)
	@echo 'Finished building: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(GEN_MISC_FILES__QUOTED)$(EXE_OUTPUTS__QUOTED)$(GEN_FILES__QUOTED)$(BIN_OUTPUTS__QUOTED)$(GEN_OPTS__QUOTED)$(GEN_CMDS__QUOTED)
	-$(RMDIR) $(GEN_MISC_DIRS__QUOTED)
	-$(RM) "CC1310_LAUNCHXL.d" "EventManager.d" "IIC.d" "Sleep.d" "alphaMain.d" "alphaRadioTest.d" "bootTimestamp.d" "ccfg.d" "config_parse.d" "eeprom.d" "eepromTest.d" "radioSendReceive.d" "sensors.d" "serialize.d" "uArtEcho.d" "easylink/EasyLink.d" "smartrf_settings/smartrf_settings.d" "smartrf_settings/smartrf_settings_predefined.d" 
	-$(RM) "CC1310_LAUNCHXL.obj" "EventManager.obj" "IIC.obj" "Sleep.obj" "alphaMain.obj" "alphaRadioTest.obj" "bootTimestamp.obj" "ccfg.obj" "config_parse.obj" "eeprom.obj" "eepromTest.obj" "radioSendReceive.obj" "sensors.obj" "serialize.obj" "uArtEcho.obj" "easylink/EasyLink.obj" "smartrf_settings/smartrf_settings.obj" "smartrf_settings/smartrf_settings_predefined.obj" 
	-@echo 'Finished clean'
	-@echo ' '

pre-build:
	-@echo 'Build and run the configuration setup. This updates the timestamp in the config file'
	-gcc /home/champ/Git/72point5/hardware/CowTags/../../add_timestamp_to_config.c -o /home/champ/Git/72point5/hardware/CowTags/../../config
	-/home/champ/Git/72point5/hardware/CowTags/../../config /home/champ/Git/72point5/hardware/CowTags/global_cfg.h
	-@echo ' '

.PHONY: all clean dependents
.SECONDARY: main-build pre-build

-include ../makefile.targets

