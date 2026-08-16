cmake_minimum_required(VERSION 3.20)

if(NOT DEFINED OE_DATA_DIR OR OE_DATA_DIR STREQUAL "" OR
   NOT DEFINED OE_HORIZONS_DECODER OR OE_HORIZONS_DECODER STREQUAL "")
  message(FATAL_ERROR "OE_DATA_DIR and OE_HORIZONS_DECODER are required")
endif()

set(CHIRON_URL
    "https://ssd.jpl.nasa.gov/api/horizons.api?format=text&COMMAND=%272060%3B%27&EPHEM_TYPE=SPK&START_TIME=%271800-01-01%27&STOP_TIME=%272200-01-01%27&OBJ_DATA=YES")
set(CHIRON_PATH "${OE_DATA_DIR}/chiron-2060-1800-2200.bsp")
set(RESPONSE_PATH "${OE_DATA_DIR}/chiron-horizons-response.tmp")

function(chiron_is_pinned path result)
  file(STRINGS "${path}" solution REGEX "Horizons_SPK:JPL#171")
  file(STRINGS "${path}" object_id REGEX "Target SPK ID   : 20002060")
  file(STRINGS "${path}" start_time REGEX "Start time      : A.D. 1800-Jan-01")
  file(STRINGS "${path}" stop_time REGEX "Stop  time      : A.D. 2200-Jan-01")
  if(solution AND object_id AND start_time AND stop_time)
    set(${result} TRUE PARENT_SCOPE)
  else()
    set(${result} FALSE PARENT_SCOPE)
  endif()
endfunction()

file(MAKE_DIRECTORY "${OE_DATA_DIR}")
set(download_required TRUE)
if(EXISTS "${CHIRON_PATH}")
  chiron_is_pinned("${CHIRON_PATH}" kernel_is_pinned)
  if(kernel_is_pinned)
    set(download_required FALSE)
    message(STATUS "Chiron is already present and verified: ${CHIRON_PATH}")
  else()
    file(REMOVE "${CHIRON_PATH}")
  endif()
endif()

if(download_required)
  message(STATUS "Downloading the pinned Horizons Chiron solution JPL#171")
  file(DOWNLOAD "${CHIRON_URL}" "${RESPONSE_PATH}"
       STATUS download_status TLS_VERIFY ON)
  list(GET download_status 0 status_code)
  list(GET download_status 1 status_message)
  if(NOT status_code EQUAL 0)
    file(REMOVE "${RESPONSE_PATH}")
    message(FATAL_ERROR "Chiron download failed: ${status_message}")
  endif()
  execute_process(COMMAND "${OE_HORIZONS_DECODER}" "${RESPONSE_PATH}" "${CHIRON_PATH}"
                  RESULT_VARIABLE decode_status)
  file(REMOVE "${RESPONSE_PATH}")
  if(NOT decode_status EQUAL 0)
    message(FATAL_ERROR "Horizons did not return a valid SPK response")
  endif()
  chiron_is_pinned("${CHIRON_PATH}" kernel_is_pinned)
  if(NOT kernel_is_pinned)
    file(REMOVE "${CHIRON_PATH}")
    message(FATAL_ERROR
      "Horizons returned a different Chiron object, coverage, or orbit solution; update provenance and fixtures explicitly")
  endif()
endif()

file(SHA256 "${CHIRON_PATH}" CHIRON_SHA256)

file(WRITE "${OE_DATA_DIR}/chiron-manifest.json"
  "{\n"
  "  \"schema\": 1,\n"
  "  \"object\": \"2060 Chiron\",\n"
  "  \"naif_id\": 20002060,\n"
  "  \"orbit_solution\": \"JPL#171 (2026-06-05)\",\n"
  "  \"coverage\": \"1800-01-01 through 2200-01-01 TDB\",\n"
  "  \"source\": \"${CHIRON_URL}\",\n"
  "  \"sha256\": \"${CHIRON_SHA256}\"\n"
  "}\n")

message(STATUS "Chiron ready: ${CHIRON_PATH}")
