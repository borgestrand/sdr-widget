################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include subdir.mk
-include src/subdir.mk
-include src/SOFTWARE_FRAMEWORK/UTILS/STARTUP_FILES/GCC/subdir.mk
-include src/SOFTWARE_FRAMEWORK/UTILS/DEBUG/subdir.mk
-include src/SOFTWARE_FRAMEWORK/SERVICES/USB/CLASS/HID/subdir.mk
-include src/SOFTWARE_FRAMEWORK/SERVICES/USB/CLASS/AUDIO/subdir.mk
-include src/SOFTWARE_FRAMEWORK/SERVICES/FREERTOS/Source/subdir.mk
-include src/SOFTWARE_FRAMEWORK/SERVICES/FREERTOS/Source/portable/MemMang/subdir.mk
-include src/SOFTWARE_FRAMEWORK/SERVICES/FREERTOS/Source/portable/GCC/AVR32_UC3/subdir.mk
-include src/SOFTWARE_FRAMEWORK/DRIVERS/WDT/subdir.mk
-include src/SOFTWARE_FRAMEWORK/DRIVERS/USBB/subdir.mk
-include src/SOFTWARE_FRAMEWORK/DRIVERS/USBB/ENUM/subdir.mk
-include src/SOFTWARE_FRAMEWORK/DRIVERS/USBB/ENUM/DEVICE/subdir.mk
-include src/SOFTWARE_FRAMEWORK/DRIVERS/USART/subdir.mk
-include src/SOFTWARE_FRAMEWORK/DRIVERS/TWIM/subdir.mk
-include src/SOFTWARE_FRAMEWORK/DRIVERS/TC/subdir.mk
-include src/SOFTWARE_FRAMEWORK/DRIVERS/SSC/I2S/subdir.mk
-include src/SOFTWARE_FRAMEWORK/DRIVERS/RTC/subdir.mk
-include src/SOFTWARE_FRAMEWORK/DRIVERS/PM/subdir.mk
-include src/SOFTWARE_FRAMEWORK/DRIVERS/PDCA/subdir.mk
-include src/SOFTWARE_FRAMEWORK/DRIVERS/INTC/subdir.mk
-include src/SOFTWARE_FRAMEWORK/DRIVERS/GPIO/subdir.mk
-include src/SOFTWARE_FRAMEWORK/DRIVERS/FLASHC/subdir.mk
-include src/SOFTWARE_FRAMEWORK/DRIVERS/EIC/subdir.mk
-include src/SOFTWARE_FRAMEWORK/BOARDS/SDRwdgtLite/subdir.mk
-include src/SOFTWARE_FRAMEWORK/ASM/subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 

# All Target
all: ../src/signature.h widget.elf 

# Tool invocations
widget.elf: $(OBJS) $(USER_OBJS) ../src/signature.h
	@echo Link $@
	@avr32-gcc -nostartfiles -Wl,--gc-sections -Wl,-e,_trampoline -mpart=uc3a3256 -Wl,--gc-sections --rodata-writable -Wl,--direct-data -o"widget.elf" $(OBJS) $(USER_OBJS) $(LIBS)
# Other Targets
clean:
	-$(RM) $(OBJS)$(C_DEPS)$(EXECUTABLES) widget.elf ../src/signature.h

#
# Generates a source file containing the git HEAD SHA (the first twelve characters)
# IW0HDV May 9, 2011
#
../src/signature.h:
	@echo "#ifndef SIGNATURE_H_"                                    >  ../src/signature.h
	@echo "#define SIGNATURE_H_"                                    >> ../src/signature.h
	@echo ""                                                        >> ../src/signature.h
	@echo "// please do not version this file with git !"           >> ../src/signature.h
	@echo "// automatically generated @`date`, please don't change manually">> ../src/signature.h
	@echo ""                                                        >> ../src/signature.h
	@echo -n "#define FEATURE_SIGNATURE_GIT_TAG \""                 >> ../src/signature.h
	@echo -n `git describe --tag`                                   >> ../src/signature.h
	@echo "\""                                                      >> ../src/signature.h
	@echo ""                                                        >> ../src/signature.h
	@echo -n "#define FEATURE_SIGNATURE_GIT_VALUE \""               >> ../src/signature.h
	@echo -n `git rev-parse HEAD` | sed -e 's/^\(.\{12\}\).*$$/\1/' >> ../src/signature.h
	@echo "\""                                                      >> ../src/signature.h
	@echo ""                                                        >> ../src/signature.h
	@echo "#endif"                                                  >> ../src/signature.h

.PHONY: all clean dependents
.SECONDARY:

-include ../makefile.targets
