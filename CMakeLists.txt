cmake_minimum_required(VERSION 3.15)
project(main CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Conan tarafından sağlanan paketleri buluyorum
find_package(OpenCV REQUIRED)

# Edge Impulse SDK kaynaklarını glob ile topladım
file(GLOB_RECURSE EI_SDK_SOURCES
    "lib/edge-impulse-sdk/tensorflow/lite/micro/*.cc"
    "lib/edge-impulse-sdk/tensorflow/lite/micro/kernels/*.cc"
    "lib/edge-impulse-sdk/tensorflow/lite/c/*.c"
    "lib/edge-impulse-sdk/tensorflow/lite/core/api/*.cc"
    "lib/edge-impulse-sdk/tensorflow/lite/kernels/internal/*.cc"
    "lib/edge-impulse-sdk/tensorflow/lite/kernels/*.cc"
    "lib/edge-impulse-sdk/CMSIS/DSP/Source/**/*.c"
    "lib/edge-impulse-sdk/dsp/**/*.cpp"
    "lib/edge-impulse-sdk/classifier/**/*.cpp"
    "lib/edge-impulse-sdk/porting/**/*.cpp"
    "lib/tflite-model/*.cpp"
)

# Manuel olarak common.c dosyasını ekliyorum nedenini çözemediğim bazı sorunlardan dolayı
list(APPEND EI_SDK_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/lib/edge-impulse-sdk/tensorflow/lite/c/common.c")

# İstenmeyen test dosyalarını filtreliyorum warning flan almaya gerek yok
list(FILTER EI_SDK_SOURCES EXCLUDE REGEX ".*test.*")
list(FILTER EI_SDK_SOURCES EXCLUDE REGEX ".*_test\.cc$")
list(FILTER EI_SDK_SOURCES EXCLUDE REGEX ".*mock_micro_graph\.cc$")
list(FILTER EI_SDK_SOURCES EXCLUDE REGEX ".*kernel_runner\.cc$")
list(FILTER EI_SDK_SOURCES EXCLUDE REGEX ".*benchmark.*")

add_executable(main src/main.cpp ${EI_SDK_SOURCES})

# Include dizinlerini ve derleme tanımlarını ayarlıyorum
target_include_directories(main PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/lib
    ${CMAKE_CURRENT_SOURCE_DIR}/lib/edge-impulse-sdk
    ${CMAKE_CURRENT_SOURCE_DIR}/lib/edge-impulse-sdk/tensorflow
    ${CMAKE_CURRENT_SOURCE_DIR}/lib/edge-impulse-sdk/third_party
    ${CMAKE_CURRENT_SOURCE_DIR}/lib/edge-impulse-sdk/third_party/flatbuffers/include
    ${CMAKE_CURRENT_SOURCE_DIR}/lib/edge-impulse-sdk/third_party/gemmlowp
    ${CMAKE_CURRENT_SOURCE_DIR}/lib/edge-impulse-sdk/third_party/ruy
    ${CMAKE_CURRENT_SOURCE_DIR}/lib/model-parameters
    ${CMAKE_CURRENT_SOURCE_DIR}/lib/tflite-model
)

# Derleme tanımlarını ekliyorum
target_compile_definitions(main PRIVATE
    TF_LITE_DISABLE_X86_NEON=1
    TF_LITE_STATIC_MEMORY=1
    EI_CLASSIFIER_TFLITE_ENABLE_CMSIS_NN=0
)

target_link_libraries(main opencv::opencv pthread dl)

install(TARGETS main DESTINATION "."
        RUNTIME DESTINATION bin
        ARCHIVE DESTINATION lib
        LIBRARY DESTINATION lib
        )
