################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../system/src/emlib/em_acmp.c \
../system/src/emlib/em_adc.c \
../system/src/emlib/em_aes.c \
../system/src/emlib/em_assert.c \
../system/src/emlib/em_burtc.c \
../system/src/emlib/em_cmu.c \
../system/src/emlib/em_dac.c \
../system/src/emlib/em_dbg.c \
../system/src/emlib/em_dma.c \
../system/src/emlib/em_ebi.c \
../system/src/emlib/em_emu.c \
../system/src/emlib/em_gpio.c \
../system/src/emlib/em_i2c.c \
../system/src/emlib/em_idac.c \
../system/src/emlib/em_int.c \
../system/src/emlib/em_lcd.c \
../system/src/emlib/em_lesense.c \
../system/src/emlib/em_letimer.c \
../system/src/emlib/em_leuart.c \
../system/src/emlib/em_mpu.c \
../system/src/emlib/em_msc.c \
../system/src/emlib/em_opamp.c \
../system/src/emlib/em_pcnt.c \
../system/src/emlib/em_prs.c \
../system/src/emlib/em_rmu.c \
../system/src/emlib/em_rtc.c \
../system/src/emlib/em_system.c \
../system/src/emlib/em_timer.c \
../system/src/emlib/em_usart.c \
../system/src/emlib/em_vcmp.c \
../system/src/emlib/em_wdog.c 

OBJS += \
./system/src/emlib/em_acmp.o \
./system/src/emlib/em_adc.o \
./system/src/emlib/em_aes.o \
./system/src/emlib/em_assert.o \
./system/src/emlib/em_burtc.o \
./system/src/emlib/em_cmu.o \
./system/src/emlib/em_dac.o \
./system/src/emlib/em_dbg.o \
./system/src/emlib/em_dma.o \
./system/src/emlib/em_ebi.o \
./system/src/emlib/em_emu.o \
./system/src/emlib/em_gpio.o \
./system/src/emlib/em_i2c.o \
./system/src/emlib/em_idac.o \
./system/src/emlib/em_int.o \
./system/src/emlib/em_lcd.o \
./system/src/emlib/em_lesense.o \
./system/src/emlib/em_letimer.o \
./system/src/emlib/em_leuart.o \
./system/src/emlib/em_mpu.o \
./system/src/emlib/em_msc.o \
./system/src/emlib/em_opamp.o \
./system/src/emlib/em_pcnt.o \
./system/src/emlib/em_prs.o \
./system/src/emlib/em_rmu.o \
./system/src/emlib/em_rtc.o \
./system/src/emlib/em_system.o \
./system/src/emlib/em_timer.o \
./system/src/emlib/em_usart.o \
./system/src/emlib/em_vcmp.o \
./system/src/emlib/em_wdog.o 

C_DEPS += \
./system/src/emlib/em_acmp.d \
./system/src/emlib/em_adc.d \
./system/src/emlib/em_aes.d \
./system/src/emlib/em_assert.d \
./system/src/emlib/em_burtc.d \
./system/src/emlib/em_cmu.d \
./system/src/emlib/em_dac.d \
./system/src/emlib/em_dbg.d \
./system/src/emlib/em_dma.d \
./system/src/emlib/em_ebi.d \
./system/src/emlib/em_emu.d \
./system/src/emlib/em_gpio.d \
./system/src/emlib/em_i2c.d \
./system/src/emlib/em_idac.d \
./system/src/emlib/em_int.d \
./system/src/emlib/em_lcd.d \
./system/src/emlib/em_lesense.d \
./system/src/emlib/em_letimer.d \
./system/src/emlib/em_leuart.d \
./system/src/emlib/em_mpu.d \
./system/src/emlib/em_msc.d \
./system/src/emlib/em_opamp.d \
./system/src/emlib/em_pcnt.d \
./system/src/emlib/em_prs.d \
./system/src/emlib/em_rmu.d \
./system/src/emlib/em_rtc.d \
./system/src/emlib/em_system.d \
./system/src/emlib/em_timer.d \
./system/src/emlib/em_usart.d \
./system/src/emlib/em_vcmp.d \
./system/src/emlib/em_wdog.d 


# Each subdirectory must supply rules for building sources it contributes
system/src/emlib/%.o: ../system/src/emlib/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross ARM C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m3 -mthumb -Og -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-move-loop-invariants -Wall -Wextra  -g3 -DDEBUG -DOS_USE_SEMIHOSTING -DTRACE -DOS_USE_TRACE_SEMIHOSTING_DEBUG -DEFM32LG980F256 -I"../include" -I"../system/include" -I"../system/include/cmsis" -I"../system/include/OCTA" -I../system/include/emlib -std=gnu11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


