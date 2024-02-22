if(NOT executable)
   message(FATAL_ERROR "Variable executable not defined")
endif()

if(NOT expected)
   message(FATAL_ERROR "Variable expected not defined")
endif()

# message(WARNING "exec=${executable} ; expec=${expected} ; input=${input}")

if(input)
	execute_process(
		COMMAND ${executable}
		INPUT_FILE ${input}
		OUTPUT_VARIABLE output
		OUTPUT_STRIP_TRAILING_WHITESPACE
		RESULT_VARIABLE RETCODE
	)
else()
	execute_process(
		COMMAND ${executable}
		OUTPUT_VARIABLE output
		OUTPUT_STRIP_TRAILING_WHITESPACE
		RESULT_VARIABLE RETCODE
	)
endif()

if(RETCODE)
	message(FATAL_ERROR "execute_process failed: ${RETCODE}")
endif()

file(READ "${expected}" expected_output)
string(STRIP "${expected_output}" expected_output)
string(COMPARE EQUAL "${output}" "${expected_output}" cmp)
if(NOT cmp)
	message(FATAL_ERROR "Outputs differ. \"${output}\" != \"${expected_output}\"")
endif()
