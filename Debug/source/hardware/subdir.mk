################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../source/hardware/L3GD20Gyro.cpp \
../source/hardware/LMS303.cpp \
../source/hardware/LPS331Altimeter.cpp 

OBJS += \
./source/hardware/L3GD20Gyro.o \
./source/hardware/LMS303.o \
./source/hardware/LPS331Altimeter.o 

CPP_DEPS += \
./source/hardware/L3GD20Gyro.d \
./source/hardware/LMS303.d \
./source/hardware/LPS331Altimeter.d 


# Each subdirectory must supply rules for building sources it contributes
source/hardware/%.o: ../source/hardware/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	arm-linux-gnueabihf-g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


