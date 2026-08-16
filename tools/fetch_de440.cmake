cmake_minimum_required(VERSION 3.20)

if(NOT DEFINED OE_DATA_DIR OR OE_DATA_DIR STREQUAL "")
  get_filename_component(OE_DATA_DIR "${CMAKE_CURRENT_LIST_DIR}/../data" ABSOLUTE)
endif()

set(DE440_URL
    "https://naif.jpl.nasa.gov/pub/naif/generic_kernels/spk/planets/de440.bsp")
set(DE440_SHA256
    "a4ce9bf9b3282becc9f4b2ac3cebe03a2ae7599981aabd7265fd8482fff7c4b5")
set(DE440_PATH "${OE_DATA_DIR}/de440.bsp")

file(MAKE_DIRECTORY "${OE_DATA_DIR}")

set(download_required TRUE)
if(EXISTS "${DE440_PATH}")
  file(SHA256 "${DE440_PATH}" existing_hash)
  if(existing_hash STREQUAL DE440_SHA256)
    set(download_required FALSE)
    message(STATUS "DE440 is already present and verified: ${DE440_PATH}")
  else()
    message(STATUS "Replacing DE440 because its checksum is incorrect")
    file(REMOVE "${DE440_PATH}")
  endif()
endif()

if(download_required)
  message(STATUS "Downloading DE440 (approximately 114 MB)")
  file(DOWNLOAD
      "${DE440_URL}"
      "${DE440_PATH}"
      EXPECTED_HASH "SHA256=${DE440_SHA256}"
      STATUS download_status
      SHOW_PROGRESS
      TLS_VERIFY ON)
  list(GET download_status 0 status_code)
  list(GET download_status 1 status_message)
  if(NOT status_code EQUAL 0)
    file(REMOVE "${DE440_PATH}")
    message(FATAL_ERROR "DE440 download failed: ${status_message}")
  endif()
endif()

file(WRITE "${OE_DATA_DIR}/manifest.json"
  "{\n"
  "  \"schema\": 1,\n"
  "  \"files\": [\n"
  "    {\n"
  "      \"name\": \"de440.bsp\",\n"
  "      \"source\": \"${DE440_URL}\",\n"
  "      \"sha256\": \"${DE440_SHA256}\",\n"
  "      \"coverage\": \"1550-2650\"\n"
  "    }\n"
  "  ]\n"
  "}\n")

message(STATUS "DE440 ready: ${DE440_PATH}")
