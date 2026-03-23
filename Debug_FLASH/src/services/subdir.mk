################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/services/braking.c \
../src/services/steering_control_linear.c \
../src/services/steering_smoothing.c \
../src/services/vision_linear_v2.c 

OBJS += \
./src/services/braking.o \
./src/services/steering_control_linear.o \
./src/services/steering_smoothing.o \
./src/services/vision_linear_v2.o 

C_DEPS += \
./src/services/braking.d \
./src/services/steering_control_linear.d \
./src/services/steering_smoothing.d \
./src/services/vision_linear_v2.d 


# Each subdirectory must supply rules for building sources it contributes
src/services/%.o: ../src/services/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Standard S32DS C Compiler'
	arm-none-eabi-gcc "@src/services/braking.args" -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


