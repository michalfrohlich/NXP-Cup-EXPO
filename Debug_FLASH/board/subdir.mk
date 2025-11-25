################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../board/Port_Ci_Port_Ip_Cfg.c 

OBJS += \
./board/Port_Ci_Port_Ip_Cfg.o 

C_DEPS += \
./board/Port_Ci_Port_Ip_Cfg.d 


# Each subdirectory must supply rules for building sources it contributes
board/%.o: ../board/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Standard S32DS C Compiler'
	arm-none-eabi-gcc "@board/Port_Ci_Port_Ip_Cfg.args" -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


