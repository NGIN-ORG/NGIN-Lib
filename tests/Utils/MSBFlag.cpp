
#include <NGIN/Utilities/MSBFlag.hpp>
#include <boost/ut.hpp>
#include <cstdint>
#include <limits>

using namespace boost::ut;
using namespace NGIN::Utilities;

suite<"MSBFlagTest"> msbFlagTest = [] {
    "DefaultConstructor"_test = [] {
        MSBFlag<uint32_t> flag;
        expect(flag.GetValue() == 0_u32);
        expect(flag.GetFlag() == false);
        expect(flag.GetRaw() == 0_u32);
    };

    "ConstructorWithValueAndFlag"_test = [] {
        MSBFlag<uint32_t> flag(0xABCDEFu, true);
        // The MSB bit is reserved for the flag
        // so the stored value is (0xABCDEFu & 0x7FFFFFFF) if 32-bit
        // let's check the raw bit carefully
        // But for simplicity, let's just verify we can read them back
        expect(flag.GetValue() == (0xABCDEFu & 0x7FFFFFFFu));
        expect(flag.GetFlag() == true);

        MSBFlag<uint32_t> flag2(0xFFFFu, false);
        expect(flag2.GetValue() == 0xFFFFu);
        expect(flag2.GetFlag() == false);
    };

    "SetValueCheck"_test = [] {
        MSBFlag<uint32_t> flag;
        flag.SetValue(12345u);
        expect(flag.GetValue() == 12345_u32);
        expect(flag.GetFlag() == false); // default was false
        // Setting the MSB doesn't alter the lower bits
    };

    "SetFlagCheck"_test = [] {
        MSBFlag<uint32_t> flag(0x10u, false);
        flag.SetFlag(true);
        expect(flag.GetFlag() == true);
        // Ensure the value is the same
        expect(flag.GetValue() == 0x10);

        // Turn the flag back off
        flag.SetFlag(false);
        expect(flag.GetFlag() == false);
        expect(flag.GetValue() == 0x10);
    };

    "SetBothValueAndFlag"_test = [] {
        MSBFlag<uint32_t> flag;
        flag.Set(0x7777u, true);
        expect(flag.GetValue() == 0x7777);
        expect(flag.GetFlag() == true);

        flag.Set(0x8888u, false);
        expect(flag.GetValue() == 0x8888);
        expect(flag.GetFlag() == false);
    };

    "RawManipulation"_test = [] {
        MSBFlag<uint32_t> flag;
        // Let's pick a number that sets the MSB bit
        // For a 32-bit number, the MSB is bit 31 (counting from 0)
        uint32_t rawData = (1u << 31) | 0x12345678u; 
        flag.SetRaw(rawData);

        expect(flag.GetRaw() == rawData);
        expect(flag.GetFlag() == true); // MSB is set
        expect(flag.GetValue() == (rawData & 0x7FFFFFFFu)); // mask out MSB

        // Now clear the MSB
        uint32_t rawData2 = 0x0FFFFFFFu; 
        flag.SetRaw(rawData2);
        expect(flag.GetRaw() == rawData2);
        expect(flag.GetFlag() == false);
        expect(flag.GetValue() == rawData2);
    };

    "EqualityInequalityCheck"_test = [] {
        MSBFlag<uint16_t> a(0x1234u, true);
        MSBFlag<uint16_t> b(0x1234u, true);
        MSBFlag<uint16_t> c(0x1234u, false);

        expect(a == b);
        expect(a != c);
        expect(b != c);
    };

    "BoundaryCheck"_test = [] {
        // For MSB, the maximum storable "value" is (maxValue >> 1)
        // because the top bit is used for the flag
        constexpr auto halfMax = std::numeric_limits<uint32_t>::max() >> 1; 
        MSBFlag<uint32_t> flag(halfMax, true);

        expect(flag.GetValue() == halfMax);
        expect(flag.GetFlag() == true);

        // If we try to store a bigger "value" than halfMax, 
        // we effectively lose that top bit to the flag. 
        // Typically you'd define usage constraints to avoid confusion.
    };
};
