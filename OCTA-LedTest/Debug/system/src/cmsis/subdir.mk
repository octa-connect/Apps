################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../system/src/cmsis/system_OCTA.c \
../system/src/cmsis/vectors_OCTA.c 

OBJS += \
./system/src/cmsis/system_OCTA.o \
./system/src/cmsis/vectors_OCTA.o 

C_DEPS += \
./system/src/cmsis/system_OCTA.d \
./system/src/cmsis/vectors_OCTA.d 


# Each subdirectory must supply rules for building sources it contributes
system/src/cmsis/%.o: ../system/src/cmsis/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross ARM C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m3 -mthumb -Og -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-move-loop-invariants -Wall -Wextra  -g3 -DDEBUG -DOS_USE_SEMIHOSTING -DTRACE -DOS_USE_TRACE_SEMIHOSTING_DEBUG -DEFM32LG980F256 -I"../include" -I"../system/include" -I"../system/include/cmsis" -I"../system/include/OCTA" -I../system/include/emlib -std=gnu11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


