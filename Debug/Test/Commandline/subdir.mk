################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../Test/Commandline/CommandLineUtils.cpp \
../Test/Commandline/Nastran2Aster_test.cpp \
../Test/Commandline/Nastran2Nastran_test.cpp \
../Test/Commandline/Nastran2Systus_test.cpp 

OBJS += \
./Test/Commandline/CommandLineUtils.o \
./Test/Commandline/Nastran2Aster_test.o \
./Test/Commandline/Nastran2Nastran_test.o \
./Test/Commandline/Nastran2Systus_test.o 

CPP_DEPS += \
./Test/Commandline/CommandLineUtils.d \
./Test/Commandline/Nastran2Aster_test.d \
./Test/Commandline/Nastran2Nastran_test.d \
./Test/Commandline/Nastran2Systus_test.d 


# Each subdirectory must supply rules for building sources it contributes
Test/Commandline/%.o: ../Test/Commandline/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -std=c++1y -I/opt/boost_1_57_0/include -I/usr/lib/openmpi/include -I/usr/lib/openmpi/include/openmpi -I"/local/users/dallolio/devel/workspace-vega/vega/build" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

