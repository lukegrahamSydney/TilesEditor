add_library(
        gs1compiler

        CompileVisitor.cpp              ../../include/gs1/compiler/CompileVisitor.hpp
        BytecodeHeader.cpp              ../../include/gs1/compiler/BytecodeHeader.hpp
        BytecodeBody.cpp                ../../include/gs1/compiler/BytecodeBody.hpp
        DepthVisitor.cpp                ../../include/gs1/compiler/DepthVisitor.hpp
        )

target_link_libraries(gs1compiler gs1common)