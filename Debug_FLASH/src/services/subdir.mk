################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/services/steering_control_linear.c \
../src/services/vision_linear.c 

OBJS += \
./src/services/steering_control_linear.o \
./src/services/vision_linear.o 

C_DEPS += \
./src/services/steering_control_linear.d \
./src/services/vision_linear.d 


# Each subdirectory must supply rules for building sources it contributes
src/services/%.o: ../src/services/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Standard S32DS C Compiler'
	arm-none-eabi-gcc "@src/services/steering_control_linear.args" -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


