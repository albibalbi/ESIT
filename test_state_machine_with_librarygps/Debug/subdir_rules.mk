################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Each subdirectory must supply rules for building sources it contributes
gps.obj: ../gps.c $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: Arm Compiler'
	"/home/marco/ti/ccs1230/ccs/tools/compiler/ti-cgt-arm_20.2.7.LTS/bin/armcl" -mv7M4 --code_state=16 --float_support=FPv4SPD16 -me --include_path="/home/marco/ti/ccs1230/ccs/ccs_base/arm/include" --include_path="/home/marco/UNI/5°semestre/ESIoT/progetto/MSP423/library/simplelink/simplelink_msp432p4_sdk_3_40_01_02/source" --include_path="/home/marco/ti/ccs1230/ccs/ccs_base/arm/include/CMSIS" --include_path="/home/marco/UNI/5°semestre/ESIoT/progetto/MSP423/projects/test_state_machine_with_librarygps" --include_path="/home/marco/ti/ccs1230/ccs/tools/compiler/ti-cgt-arm_20.2.7.LTS/include" --advice:power=all --define=__MSP432P401R__ --define=ccs -g --c11 --gcc --diag_warning=225 --diag_wrap=off --display_error_number --abi=eabi --preproc_with_compile --preproc_dependency="$(basename $(<F)).d_raw" $(GEN_OPTS__FLAG) "$(shell echo $<)"
	@echo 'Finished building: "$<"'
	@echo ' '

main.obj: ../main.c $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: Arm Compiler'
	"/home/marco/ti/ccs1230/ccs/tools/compiler/ti-cgt-arm_20.2.7.LTS/bin/armcl" -mv7M4 --code_state=16 --float_support=FPv4SPD16 -me --include_path="/home/marco/ti/ccs1230/ccs/ccs_base/arm/include" --include_path="/home/marco/UNI/5°semestre/ESIoT/progetto/MSP423/library/simplelink/simplelink_msp432p4_sdk_3_40_01_02/source" --include_path="/home/marco/ti/ccs1230/ccs/ccs_base/arm/include/CMSIS" --include_path="/home/marco/UNI/5°semestre/ESIoT/progetto/MSP423/projects/test_state_machine_with_librarygps" --include_path="/home/marco/ti/ccs1230/ccs/tools/compiler/ti-cgt-arm_20.2.7.LTS/include" --advice:power=all --define=__MSP432P401R__ --define=ccs -g --c11 --gcc --diag_warning=225 --diag_wrap=off --display_error_number --abi=eabi --preproc_with_compile --preproc_dependency="$(basename $(<F)).d_raw" $(GEN_OPTS__FLAG) "$(shell echo $<)"
	@echo 'Finished building: "$<"'
	@echo ' '

race.obj: ../race.c $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: Arm Compiler'
	"/home/marco/ti/ccs1230/ccs/tools/compiler/ti-cgt-arm_20.2.7.LTS/bin/armcl" -mv7M4 --code_state=16 --float_support=FPv4SPD16 -me --include_path="/home/marco/ti/ccs1230/ccs/ccs_base/arm/include" --include_path="/home/marco/UNI/5°semestre/ESIoT/progetto/MSP423/library/simplelink/simplelink_msp432p4_sdk_3_40_01_02/source" --include_path="/home/marco/ti/ccs1230/ccs/ccs_base/arm/include/CMSIS" --include_path="/home/marco/UNI/5°semestre/ESIoT/progetto/MSP423/projects/test_state_machine_with_librarygps" --include_path="/home/marco/ti/ccs1230/ccs/tools/compiler/ti-cgt-arm_20.2.7.LTS/include" --advice:power=all --define=__MSP432P401R__ --define=ccs -g --c11 --gcc --diag_warning=225 --diag_wrap=off --display_error_number --abi=eabi --preproc_with_compile --preproc_dependency="$(basename $(<F)).d_raw" $(GEN_OPTS__FLAG) "$(shell echo $<)"
	@echo 'Finished building: "$<"'
	@echo ' '

%.obj: ../%.c $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: Arm Compiler'
	"/home/marco/ti/ccs1230/ccs/tools/compiler/ti-cgt-arm_20.2.7.LTS/bin/armcl" -mv7M4 --code_state=16 --float_support=FPv4SPD16 -me --include_path="/home/marco/ti/ccs1230/ccs/ccs_base/arm/include" --include_path="/home/marco/UNI/5°semestre/ESIoT/progetto/MSP423/library/simplelink/simplelink_msp432p4_sdk_3_40_01_02/source" --include_path="/home/marco/ti/ccs1230/ccs/ccs_base/arm/include/CMSIS" --include_path="/home/marco/UNI/5°semestre/ESIoT/progetto/MSP423/projects/test_state_machine_with_librarygps" --include_path="/home/marco/ti/ccs1230/ccs/tools/compiler/ti-cgt-arm_20.2.7.LTS/include" --advice:power=all --define=__MSP432P401R__ --define=ccs -g --gcc --diag_warning=225 --diag_wrap=off --display_error_number --abi=eabi --preproc_with_compile --preproc_dependency="$(basename $(<F)).d_raw" $(GEN_OPTS__FLAG) "$(shell echo $<)"
	@echo 'Finished building: "$<"'
	@echo ' '


