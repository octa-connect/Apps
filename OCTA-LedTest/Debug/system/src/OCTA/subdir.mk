################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../system/src/OCTA/system_efm32lg.c 

OBJS += \
./system/src/OCTA/system_efm32lg.o 

C_DEPS += \
./system/src/OCTA/system_efm32lg.d 


# Each subdirectory must supply rules for building sources it contributes
system/src/OCTA/%.o: ../system/src/OCTA/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross ARM C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m3 -mthumb -Og -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-move-loop-invariants -Wall -Wextra  -g3 -DDEBUG -DOS_USE_SEMIHOSTING -DTRACE -DOS_USE_TRACE_SEMIHOSTING_DEBUG -DEFM32LG980F256 -I"../include" -I"../system/include" -I"../system/include/cmsis" -I"../system/include/OCTA" -I../system/include/emlib -std=gnu11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


