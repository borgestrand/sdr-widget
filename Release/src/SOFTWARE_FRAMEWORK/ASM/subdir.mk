################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
X_SRCS += \
../src/SOFTWARE_FRAMEWORK/ASM/trampoline.x 

OBJS += \
./src/SOFTWARE_FRAMEWORK/ASM/trampoline.o 


# Each subdirectory must supply rules for building sources it contributes
src/SOFTWARE_FRAMEWORK/ASM/%.o: ../src/SOFTWARE_FRAMEWORK/ASM/%.x
	@echo Assemble $<
	@avr32-gcc $(ASFLAGS) -x assembler-with-cpp -c -Wa,-g -I../src/SOFTWARE_FRAMEWORK/UTILS/DEBUG -I../src/SOFTWARE_FRAMEWORK/DRIVERS/CPU/CYCLE_COUNTER -I../src/SOFTWARE_FRAMEWORK/DRIVERS/EIC -I../src/SOFTWARE_FRAMEWORK/DRIVERS/RTC -I../src/SOFTWARE_FRAMEWORK/SERVICES/FREERTOS/Source/portable/GCC/AVR32_UC3 -I../src/SOFTWARE_FRAMEWORK/SERVICES/FREERTOS/Source/include -I../src/SOFTWARE_FRAMEWORK/SERVICES/USB/CLASS/HID -I../src/SOFTWARE_FRAMEWORK/SERVICES/USB -I../src/CONFIG -I../src/SOFTWARE_FRAMEWORK/DRIVERS/USBB/ENUM/DEVICE -I../src/SOFTWARE_FRAMEWORK/DRIVERS/USBB/ENUM -I../src/SOFTWARE_FRAMEWORK/DRIVERS/USBB -I../src/SOFTWARE_FRAMEWORK/DRIVERS/USART -I../src/SOFTWARE_FRAMEWORK/DRIVERS/TC -I../src/SOFTWARE_FRAMEWORK/DRIVERS/PM -I../src/SOFTWARE_FRAMEWORK/DRIVERS/GPIO -I../src/SOFTWARE_FRAMEWORK/DRIVERS/FLASHC -I../src/SOFTWARE_FRAMEWORK/UTILS/LIBS/NEWLIB_ADDONS/INCLUDE -I../src/SOFTWARE_FRAMEWORK/UTILS/PREPROCESSOR -I../src/SOFTWARE_FRAMEWORK/UTILS -I../src/SOFTWARE_FRAMEWORK/DRIVERS/INTC -I../src/SOFTWARE_FRAMEWORK/BOARDS -o"$@" "$<"



