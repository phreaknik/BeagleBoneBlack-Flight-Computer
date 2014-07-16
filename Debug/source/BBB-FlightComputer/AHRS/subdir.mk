################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../source/BBB-FlightComputer/AHRS/ahrs.cpp 

OBJS += \
./source/BBB-FlightComputer/AHRS/ahrs.o 

CPP_DEPS += \
./source/BBB-FlightComputer/AHRS/ahrs.d 


# Each subdirectory must supply rules for building sources it contributes
source/BBB-FlightComputer/AHRS/%.o: ../source/BBB-FlightComputer/AHRS/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	arm-linux-gnueabihf-g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


