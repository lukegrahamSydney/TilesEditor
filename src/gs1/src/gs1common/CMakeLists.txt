add_library(
        gs1common
                              ../../include/gs1/common/GVariable.hpp
        GValue.cpp            ../../include/gs1/common/GValue.hpp
                              ../../include/gs1/common/ConstantTable.hpp
                              ../../include/gs1/common/PackedValue.hpp
        Util.cpp              ../../include/gs1/common/Util.hpp
        ByteBuffer.cpp        ../../include/gs1/common/ByteBuffer.hpp
                              ../../include/gs1/common/BufferReader.hpp
        Operation.cpp         ../../include/gs1/common/Operation.hpp
        Log.cpp               ../../include/gs1/common/Log.hpp
)
