################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include src/subdir.mk
-include generate/src/subdir.mk
-include board/subdir.mk
-include RTD/src/subdir.mk
-include Project_Settings/Startup_Code/subdir.mk
-include subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 
SECONDARY_SIZE += \
EXPO_03_Nxp_Cup_project.siz \


# All Target
all: EXPO_03_Nxp_Cup_project.elf secondary-outputs

# Tool invocations
EXPO_03_Nxp_Cup_project.elf: $(OBJS) C:/Users/misof/workspaceS32DS.3.6.3/EXPO_03_Nxp_Cup_project/Project_Settings/Linker_Files/linker_flash_s32k144.ld $(USER_OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: Standard S32DS C Linker'
	arm-none-eabi-gcc -o "EXPO_03_Nxp_Cup_project.elf" "@EXPO_03_Nxp_Cup_project.args"  $(USER_OBJS)
	@echo 'Finished building target: $@'
	@echo ' '

EXPO_03_Nxp_Cup_project.siz: EXPO_03_Nxp_Cup_project.elf
	@echo 'Invoking: Standard S32DS Print Size'
	arm-none-eabi-size --format=berkeley EXPO_03_Nxp_Cup_project.elf
	@echo 'Finished building: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) ./*
	-@echo ' '

secondary-outputs: $(SECONDARY_SIZE)

.PHONY: all clean dependents

-include ../makefile.targets
