################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../source/L3GD20Gyro.cpp \
../source/LMS303.cpp \
../source/LPS331Altimeter.cpp \
../source/main.cpp 

OBJS += \
./source/L3GD20Gyro.o \
./source/LMS303.o \
./source/LPS331Altimeter.o \
./source/main.o 

CPP_DEPS += \
./source/L3GD20Gyro.d \
./source/LMS303.d \
./source/LPS331Altimeter.d \
./source/main.d 


# Each subdirectory must supply rules for building sources it contributes
source/%.o: ../source/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	arm-linux-gnueabihf-g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


