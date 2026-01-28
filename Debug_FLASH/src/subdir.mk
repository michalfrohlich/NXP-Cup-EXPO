################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/buttons.c \
../src/camera_emulator.c \
../src/display.c \
../src/esc.c \
../src/hbridge.c \
../src/linear_camera.c \
../src/main.c \
../src/main_functions.c \
../src/onboard_pot.c \
../src/pixy2.c \
../src/receiver.c \
../src/rgb_led.c \
../src/servo.c \
../src/timebase.c \
../src/ultrasonic.c \
../src/vision_debug.c \
../src/vision_linear_v2.c 

OBJS += \
./src/buttons.o \
./src/camera_emulator.o \
./src/display.o \
./src/esc.o \
./src/hbridge.o \
./src/linear_camera.o \
./src/main.o \
./src/main_functions.o \
./src/onboard_pot.o \
./src/pixy2.o \
./src/receiver.o \
./src/rgb_led.o \
./src/servo.o \
./src/timebase.o \
./src/ultrasonic.o \
./src/vision_debug.o \
./src/vision_linear_v2.o 

C_DEPS += \
./src/buttons.d \
./src/camera_emulator.d \
./src/display.d \
./src/esc.d \
./src/hbridge.d \
./src/linear_camera.d \
./src/main.d \
./src/main_functions.d \
./src/onboard_pot.d \
./src/pixy2.d \
./src/receiver.d \
./src/rgb_led.d \
./src/servo.d \
./src/timebase.d \
./src/ultrasonic.d \
./src/vision_debug.d \
./src/vision_linear_v2.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Standard S32DS C Compiler'
	arm-none-eabi-gcc "@src/buttons.args" -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


