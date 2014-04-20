################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/SOFTWARE_FRAMEWORK/DRIVERS/PM/pm.c \
../src/SOFTWARE_FRAMEWORK/DRIVERS/PM/pm_conf_clocks.c \
../src/SOFTWARE_FRAMEWORK/DRIVERS/PM/power_clocks_lib.c 

OBJS += \
./src/SOFTWARE_FRAMEWORK/DRIVERS/PM/pm.o \
./src/SOFTWARE_FRAMEWORK/DRIVERS/PM/pm_conf_clocks.o \
./src/SOFTWARE_FRAMEWORK/DRIVERS/PM/power_clocks_lib.o 

C_DEPS += \
./src/SOFTWARE_FRAMEWORK/DRIVERS/PM/pm.d \
./src/SOFTWARE_FRAMEWORK/DRIVERS/PM/pm_conf_clocks.d \
./src/SOFTWARE_FRAMEWORK/DRIVERS/PM/power_clocks_lib.d 


# Each subdirectory must supply rules for building sources it contributes
src/SOFTWARE_FRAMEWORK/DRIVERS/PM/%.o: ../src/SOFTWARE_FRAMEWORK/DRIVERS/PM/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: AVR32/GNU C Compiler'
	avr32-gcc $(CFLAGS) -DBOARD=SDRwdgtLite -DFREERTOS_USED -I../src/SOFTWARE_FRAMEWORK/DRIVERS/SSC/I2S -I../src/SOFTWARE_FRAMEWORK/DRIVERS/PDCA -I../src/SOFTWARE_FRAMEWORK/DRIVERS/TWIM -I../src/SOFTWARE_FRAMEWORK/UTILS/DEBUG -I../src/SOFTWARE_FRAMEWORK/SERVICES/USB/CLASS/AUDIO -I../src/SOFTWARE_FRAMEWORK/SERVICES/USB/CLASS/CDC -I../src/SOFTWARE_FRAMEWORK/SERVICES/FREERTOS/Source/portable/GCC/AVR32_UC3 -I../src/SOFTWARE_FRAMEWORK/SERVICES/FREERTOS/Source/include -I../src/SOFTWARE_FRAMEWORK/SERVICES/USB/CLASS/HID -I../src/SOFTWARE_FRAMEWORK/SERVICES/USB -I../src/CONFIG -I../src/SOFTWARE_FRAMEWORK/DRIVERS/USBB/ENUM/DEVICE -I../src/SOFTWARE_FRAMEWORK/DRIVERS/USBB/ENUM -I../src/SOFTWARE_FRAMEWORK/DRIVERS/USBB -I../src/SOFTWARE_FRAMEWORK/DRIVERS/USART -I../src/SOFTWARE_FRAMEWORK/DRIVERS/TC -I../src/SOFTWARE_FRAMEWORK/DRIVERS/WDT -I../src/SOFTWARE_FRAMEWORK/DRIVERS/CPU/CYCLE_COUNTER -I../src/SOFTWARE_FRAMEWORK/DRIVERS/EIC -I../src/SOFTWARE_FRAMEWORK/DRIVERS/RTC -I../src/SOFTWARE_FRAMEWORK/DRIVERS/PM -I../src/SOFTWARE_FRAMEWORK/DRIVERS/GPIO -I../src/SOFTWARE_FRAMEWORK/DRIVERS/FLASHC -I../src/SOFTWARE_FRAMEWORK/UTILS/LIBS/NEWLIB_ADDONS/INCLUDE -I../src/SOFTWARE_FRAMEWORK/UTILS/PREPROCESSOR -I../src/SOFTWARE_FRAMEWORK/UTILS -I../src/SOFTWARE_FRAMEWORK/DRIVERS/INTC -I../src/SOFTWARE_FRAMEWORK/BOARDS -I../src -O2 -fdata-sections -Wall -c -fmessage-length=0 -mpart=uc3a3256 -ffunction-sections -masm-addr-pseudos -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


