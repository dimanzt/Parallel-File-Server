################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../pfs.c \
../test1-c1.c \
../test1-c2.c \
../test1-c3.c \
../test2.c \
../test3.c 

OBJS += \
./pfs.o \
./test1-c1.o \
./test1-c2.o \
./test1-c3.o \
./test2.o \
./test3.o 

C_DEPS += \
./pfs.d \
./test1-c1.d \
./test1-c2.d \
./test1-c3.d \
./test2.d \
./test3.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -pthread -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


