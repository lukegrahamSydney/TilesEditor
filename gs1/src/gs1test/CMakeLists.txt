add_executable(gs1test Main.cpp DebugVisitor.cpp DebugVisitor.hpp
        GFlagLibrary.hpp GStringLibrary.hpp GOutputLibrary.hpp)

target_link_libraries(gs1test gs1common gs1parse gs1compiler gs1vm)
