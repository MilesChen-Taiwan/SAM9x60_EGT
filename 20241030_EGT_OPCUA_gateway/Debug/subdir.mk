################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../OPCUA_gateway_EGT.cpp 

C_SRCS += \
../open62541.c 

CPP_DEPS += \
./OPCUA_gateway_EGT.d 

C_DEPS += \
./open62541.d 

OBJS += \
./OPCUA_gateway_EGT.o \
./open62541.o 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.cpp subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	arm-buildroot-linux-gnueabi-g++ -std=c++17 -O0 -g3 -Wall -c -fmessage-length=0  `pkg-config libegt --cflags` -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

%.o: ../%.c subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Compiler'
	arm-buildroot-linux-gnueabi-gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


clean: clean--2e-

clean--2e-:
	-$(RM) ./OPCUA_gateway_EGT.d ./OPCUA_gateway_EGT.o ./open62541.d ./open62541.o

.PHONY: clean--2e-

