/// @file test_lsb_flag.cpp
/// @brief Unit tests for NGIN::Utilities::LSBFlag

#include <NGIN/Utilities/LSBFlag.hpp>
#include <boost/ut.hpp>
#include <cstdint>
#include <limits>

using namespace boost::ut;
using namespace NGIN::Utilities;

suite<"LSBFlagTest"> lsbFlagTest = [] {
    "DefaultConstructor"_test = [] {
        LSBFlag<uint32_t> flag;
        expect(flag.GetValue() == 0_u32);
        expect(flag.GetFlag() == false);
        expect(flag.GetRaw() == 0_u32);
    };

    "ConstructorWithValueAndFlag"_test = [] {
        LSBFlag<uint32_t> flag(0xABCDEFu, true);
        expect(flag.GetValue() == 0xABCDEFu);
        expect(flag.GetFlag() == true);

        LSBFlag<uint32_t> flag2(0xFFFFu, false);
        expect(flag2.GetValue() == 0xFFFFu);
        expect(flag2.GetFlag() == false);
    };

    "SetValueCheck"_test = [] {
        LSBFlag<uint32_t> flag;
        flag.SetValue(12345u);
        expect(flag.GetValue() == 12345_u32);
        expect(flag.GetFlag() == false); // default flag was false
    };

    "SetFlagCheck"_test = [] {
        LSBFlag<uint32_t> flag(0x10u, false);
        flag.SetFlag(true);
        expect(flag.GetFlag() == true);
        expect(flag.GetValue() == 0x10);
    };

    "SetBothValueAndFlag"_test = [] {
        LSBFlag<uint32_t> flag;
        flag.Set(0x7777u, true);
        expect(flag.GetValue() == 0x7777);
        expect(flag.GetFlag() == true);

        flag.Set(0x8888u, false);
        expect(flag.GetValue() == 0x8888);
        expect(flag.GetFlag() == false);
    };

    "RawManipulation"_test = [] {
        LSBFlag<uint32_t> flag;
        flag.SetRaw(0xDEADBEEFu);
        // The LSB is the flag, so if 0xDEADBEEF is odd, the flag is 1
        expect(flag.GetRaw() == 0xDEADBEEFu);
        expect(flag.GetFlag() == true);
        // The stored value is 0xDEADBEEF >> 1
        expect(flag.GetValue() == (0xDEADBEEFu >> 1));

        // Overwrite with a new raw data
        flag.SetRaw(0x1234ABCEu); // note the last bit is 0
        expect(flag.GetFlag() == false);
        expect(flag.GetValue() == (0x1234ABCEu >> 1));
    };

    "EqualityInequalityCheck"_test = [] {
        LSBFlag<uint16_t> a(0x1234u, true);
        LSBFlag<uint16_t> b(0x1234u, true);
        LSBFlag<uint16_t> c(0x1234u, false);

        expect(a == b);
        expect(a != c);
        expect(b != c);
    };

    "BoundaryCheck"_test = [] {
        // For example, the maximum storable value is (std::numeric_limits<uint32_t>::max() >> 1).
        // The flag is the LSB bit, so ensure no overflow
        constexpr auto maxValue = std::numeric_limits<uint32_t>::max() >> 1; 
        LSBFlag<uint32_t> flag(maxValue, true);

        expect(flag.GetValue() == maxValue);
        expect(flag.GetFlag() == true);

        // If we set a value bigger than that, it will still store, 
        // but typically you should be aware the top bits might conflict 
        // with the LSB usage. This test just checks normal usage.
    };
};