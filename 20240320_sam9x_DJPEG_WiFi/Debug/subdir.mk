################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../datasrc.cpp \
../djpeg.cpp \
../jpeg2yuv.cpp 

CPP_DEPS += \
./datasrc.d \
./djpeg.d \
./jpeg2yuv.d 

OBJS += \
./datasrc.o \
./djpeg.o \
./jpeg2yuv.o 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.cpp subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	arm-buildroot-linux-gnueabi-g++ -std=c++17 -O0 -g3 -Wall -c -fmessage-length=0 `pkg-config libegt --cflags` -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


clean: clean--2e-

clean--2e-:
	-$(RM) ./datasrc.d ./datasrc.o ./djpeg.d ./djpeg.o ./jpeg2yuv.d ./jpeg2yuv.o

.PHONY: clean--2e-

