################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/otros.c \
../src/procesoAdminDeMemoria.c 

OBJS += \
./src/otros.o \
./src/procesoAdminDeMemoria.o 

C_DEPS += \
./src/otros.d \
./src/procesoAdminDeMemoria.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -I"/home/utnso/Escritorio/git/tp-2015-2c-killthepony/utiles" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


