#ifndef GS1COMMON_PACKEDVALUE_HPP
#define GS1COMMON_PACKEDVALUE_HPP

namespace gs1
{
enum PackedValueType {
  PACKVALUE_CONST_NUMBER,
  PACKVALUE_CONST_STRING,
  PACKVALUE_CONST_ARRAY,
  PACKVALUE_NAMED
};

#ifdef _MSC_VER
#pragma pack(push, 1)
#endif

struct PackedValue {
    PackedValue(PackedValueType valueType, unsigned int index = 0)
        : valueType(valueType), value(index) {};

    // The type of value, see enum PackedValueType
    unsigned int valueType : 4;
    unsigned int value : 16;
    unsigned int unused : 12;
};
#ifndef _MSC_VER
__attribute__((packed));
#else
#pragma pack(push, 0)
#endif

static_assert(
    sizeof(PackedValue) == 4,
    "Size of PackedValue isn't 4 bytes, can cause problems when deserializing");
};

#endif
