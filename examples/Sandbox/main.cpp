// Main.cpp
#include <NGIN/NGIN.hpp>
#include <iostream>
#include <string>
#include <vector>
#include <tuple>

// Using NGIN namespace for convenience
using namespace NGIN::Async;
using namespace NGIN;

// Number of iterations for each benchmark
constexpr int NUM_ITERATIONS = 100000;

// Benchmark functions (as previously defined)
void BenchmarkNGINStringConstruction()
{
    for (int i = 0; i < NUM_ITERATIONS; ++i)
    {
        NGIN::Containers::String str("Test String");
        DoNotOptimize(str);
    }
}

void BenchmarkStdStringConstruction()
{
    for (int i = 0; i < NUM_ITERATIONS; ++i)
    {
        std::string str("Test String");
        DoNotOptimize(str);
    }
}

void BenchmarkNGINStringCopy()
{
    NGIN::Containers::String original("Test String");
    for (int i = 0; i < NUM_ITERATIONS; ++i)
    {
        NGIN::Containers::String copy = original;
        DoNotOptimize(copy);
    }
}

void BenchmarkStdStringCopy()
{
    std::string original("Test String");
    for (int i = 0; i < NUM_ITERATIONS; ++i)
    {
        std::string copy = original;
        DoNotOptimize(copy);
    }
}

void BenchmarkNGINStringConcatenation()
{
    const char* str1 = "HelloWorldWorld";
    const char* str2 = "WorldWorldWorld";
    for (int i = 0; i < NUM_ITERATIONS; ++i)
    {
        NGIN::Containers::String result{std::move(str1)};
        result += str2;
        DoNotOptimize(result);
    }
}

void BenchmarkStdStringConcatenation()
{
    const char* str1 = "HelloWorldWorld";
    const char* str2 = "WorldWorldWorld";
    for (int i = 0; i < NUM_ITERATIONS; ++i)
    {
        std::string result = str1;
        result += str2;
        DoNotOptimize(result);
    }
}

namespace TEST
{
    struct Test
    {
    };

    struct Test2
    {
    };
    template <typename T>
    struct MyStruct {}; 
} // namespace TEST

template <typename T>
struct MyStruct {};

namespace Foo {
    struct Bar {};
    struct Baz {};
    namespace Nested {
        struct Quux {};
    }
}

using namespace TEST;

int main()
{
    using namespace NGIN::Meta;

 std::cout << "test: " << TypeTraits<std::string_view>::unqualifiedName << std::endl;

    constexpr auto rawInt = TypeTraits<int>::rawName; 
    constexpr auto qualInt = TypeTraits<const int*>::qualifiedName;
    constexpr auto rawMyTempl = TypeTraits<TEST::MyStruct<Foo::Bar>>::rawName;
    constexpr auto nsMyTempl = TypeTraits<TEST::MyStruct<Foo::Bar>>::namespaceName;
    constexpr auto qnMyTempl  = TypeTraits<TEST::MyStruct<Foo::Bar>>::qualifiedName;

    std::cout << "Raw int            = " << rawInt << '\n';
    std::cout << "Qualified int*     = " << qualInt << '\n';
    std::cout << "TEST::MyStruct<Foo::Bar> (raw)     = " << rawMyTempl << '\n';
    std::cout << "TEST::MyStruct<Foo::Bar> (namespace)  = " << nsMyTempl << '\n';
    std::cout << "MyStruct<Foo::Bar> (qualified) = " << qnMyTempl << '\n';

    // Define BenchmarkConfig
    BenchmarkConfig config;
    config.iterations        = 10; // Number of times to run each benchmark
    config.warmupIterations  = 2; // Number of warmup runs

    // Benchmark for NGIN::Containers::String construction
    Benchmark nginConstructBenchmark(config, BenchmarkNGINStringConstruction, "NGIN::String Construction");
    auto result_ngin_construct = nginConstructBenchmark.Run<Milliseconds>();

    // Benchmark for std::string construction
    Benchmark stdConstructBenchmark(config, BenchmarkStdStringConstruction, "std::string Construction");
    auto result_std_construct = stdConstructBenchmark.Run<Milliseconds>();

    // Benchmark for NGIN::Containers::String copy
    Benchmark nginCopyBenchmark(config, BenchmarkNGINStringCopy, "NGIN::String Copy");
    auto result_ngin_copy = nginCopyBenchmark.Run<Milliseconds>();

    // Benchmark for std::string copy
    Benchmark stdCopyBenchmark(config, BenchmarkStdStringCopy, "std::string Copy");
    auto result_std_copy = stdCopyBenchmark.Run<Milliseconds>();

    // Benchmark for NGIN::Containers::String concatenation
    Benchmark nginConcatBenchmark(config, BenchmarkNGINStringConcatenation, "NGIN::String Concatenation");
    auto result_ngin_concat = nginConcatBenchmark.Run<Milliseconds>();

    // Benchmark for std::string concatenation
    Benchmark stdConcatBenchmark(config, BenchmarkStdStringConcatenation, "std::string Concatenation");
    auto result_std_concat = stdConcatBenchmark.Run<Milliseconds>();

    // Function to print benchmark results
    auto PrintResult = [](const auto& res) {
        std::cout << "Benchmark: " << res.name << "\n";
        std::cout << "Iterations: " << res.numIterations << "\n";
        std::cout << "Average Time: " << res.averageTime << " ms\n";
        std::cout << "Min Time: " << res.minTime << " ms\n";
        std::cout << "Max Time: " << res.maxTime << " ms\n";
        std::cout << "Standard Deviation: " << res.standardDeviation << " ms\n";
        std::cout << "-----------------------------------------\n";
    };

    // Uncomment the following lines to print all benchmark results
    PrintResult(result_ngin_construct);
    PrintResult(result_std_construct);
    PrintResult(result_ngin_copy);
    PrintResult(result_std_copy);
    PrintResult(result_ngin_concat);
    PrintResult(result_std_concat);

    return 0;
}
