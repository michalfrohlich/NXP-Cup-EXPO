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
Nxp_Cup_Linear_Camera_S32K144.siz \


# All Target
all: Nxp_Cup_Linear_Camera_S32K144.elf secondary-outputs

# Tool invocations
Nxp_Cup_Linear_Camera_S32K144.elf: $(OBJS) C:/Users/misof/workspaceS32DS.3.6.3/Nxp_Cup_Linear_Camera_S32K144/Project_Settings/Linker_Files/linker_flash_s32k144.ld $(USER_OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: Standard S32DS C Linker'
	arm-none-eabi-gcc -o "Nxp_Cup_Linear_Camera_S32K144.elf" "@Nxp_Cup_Linear_Camera_S32K144.args"  $(USER_OBJS)
	@echo 'Finished building target: $@'
	@echo ' '

Nxp_Cup_Linear_Camera_S32K144.siz: Nxp_Cup_Linear_Camera_S32K144.elf
	@echo 'Invoking: Standard S32DS Print Size'
	arm-none-eabi-size --format=berkeley Nxp_Cup_Linear_Camera_S32K144.elf
	@echo 'Finished building: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) ./*
	-@echo ' '

secondary-outputs: $(SECONDARY_SIZE)

.PHONY: all clean dependents

-include ../makefile.targets
