file(REMOVE "${OUTPUT}")

execute_process(
  COMMAND "${RT3}" "${SCENE}"
  WORKING_DIRECTORY "${WORKDIR}"
  RESULT_VARIABLE render_result
)

if(NOT render_result EQUAL 0)
  message(FATAL_ERROR "Project 6 smoke render failed with exit code ${render_result}")
endif()

if(NOT EXISTS "${OUTPUT}")
  message(FATAL_ERROR "Project 6 smoke render did not write ${OUTPUT}")
endif()

file(REMOVE "${OUTPUT}")
