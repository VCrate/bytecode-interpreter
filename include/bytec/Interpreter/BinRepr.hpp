#pragma once

#include <bytec/Interpreter/Operations.hpp>

namespace bytec { namespace bin_repr {

inline constexpr ui32 decode(ui32 mask, ui8 shift, ui32 value) {
    return (value & mask) >> shift;
}

inline constexpr ui32 encode(ui32 mask, ui8 shift, ui32 value) {
    return (value << shift) & mask;
}

#define MAKE_DECODER(__name, __mask, __shift) inline ui32 __name ## _decode(ui32 value) { return decode(__mask, __shift, value); }
#define MAKE_ENCODER(__name, __mask, __shift) inline ui32 __name ## _encode(ui32 value) { return encode(__mask, __shift, value); }
#define MAKE_MASK(__name, __mask) static constexpr ui32 __name ## _mask = __mask; 
#define MAKE_SHIFT(__name, __shift) static constexpr ui32 __name ## _shift = __shift; 
#define MAKE_MAX(__name, __mask, __shift) static constexpr ui32 __name ## _max = __mask >> __shift;
#define MAKE_ALL(__name, __mask, __shift) \
    MAKE_MASK(__name, __mask)             \
    MAKE_SHIFT(__name, __shift)           \
    MAKE_MAX(__name, __mask, __shift)     \
    MAKE_DECODER(__name, __mask, __shift) \
    MAKE_ENCODER(__name, __mask, __shift)

MAKE_ALL(operation,             0xFF'00'00'00,  24)
MAKE_ALL(arg0,                  0x00'FF'F0'00,  12)
MAKE_ALL(arg1,                  0x00'00'0F'FF,  0)
MAKE_ALL(arg,                   0x00'FF'FF'FF,  0)

MAKE_ALL(arg12_type,            0xE'00,         9)
MAKE_ALL(arg12_value,           0x1'FF,         0)
MAKE_ALL(arg12_value_wo_sign,   0x0'FF,         0)
MAKE_ALL(arg12_value_sign,      0x1'00,         8)
MAKE_ALL(arg12_register,        0x1'E0,         5)
MAKE_ALL(arg12_disp,            0x0'1F,         0)

MAKE_ALL(arg24_type,            0xE0'00'00,     21)
MAKE_ALL(arg24_value,           0x1F'FF'FF,     0)
MAKE_ALL(arg24_value_wo_sign,   0x0F'FF'FF,     0)
MAKE_ALL(arg24_value_sign,      0x10'00'00,     20)
MAKE_ALL(arg24_register,        0x1E'00'00,     17)
MAKE_ALL(arg24_disp,            0x01'FF'FF,     0)

#undef MAKE_ALL
#undef MAKE_DECODER
#undef MAKE_ENCODER
#undef MAKE_MAX
#undef MAKE_MASK
#undef MAKE_SHIFT

}}