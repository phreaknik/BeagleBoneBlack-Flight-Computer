################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../source/BBB-FlightComputer/BBB-FlightComputer.cpp 

OBJS += \
./source/BBB-FlightComputer/BBB-FlightComputer.o 

CPP_DEPS += \
./source/BBB-FlightComputer/BBB-FlightComputer.d 


# Each subdirectory must supply rules for building sources it contributes
source/BBB-FlightComputer/%.o: ../source/BBB-FlightComputer/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	arm-linux-gnueabihf-g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


