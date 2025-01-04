#pragma once
#include <NGIN/Defines.hpp>
#include <NGIN/Primatives.hpp>
#include <cassert>
#include <cmath>
#include <functional>
#include <limits>
#include <mutex>
#include <string>
#include <vector>

#include <NGIN/Timer.hpp>
#include <NGIN/Units.hpp>

namespace NGIN
{

    /// @brief Prevents the compiler from optimizing away a value.
    ///
    /// @details This function ensures that the compiler does not optimize away the given value.
    /// It is based on a technique discussed by Chandler Carruth in his benchmarking talk https://www.youtube.com/watch?v=nXaxk27zwlk&t=2441s.
    ///
    /// @tparam T The type of the value to prevent from being optimized away.
    /// @param value The value to prevent the compiler from optimizing away.
    ///
    /// @note This function has different implementations depending on the compiler:
    /// - For Clang and GCC, it uses an inline assembly statement with a memory clobber.
    /// - For MSVC, it uses the _ReadWriteBarrier intrinsic.
    /// - For other compilers, it uses a volatile dummy variable.
    ///
    /// @warning This function should be used with caution, as improper use can lead to
    /// undefined behavior or performance issues.
    template<typename T>
    NGIN_ALWAYS_INLINE void DoNotOptimize(T const& value)
    {
#if defined(__clang__) || defined(__GNUC__)
        asm volatile("" : : "g"(value) : "memory");
#elif defined(_MSC_VER)
        _ReadWriteBarrier();
        (void) value;
#else
        // Fallback for other compilers
        char volatile dummy = *reinterpret_cast<char const volatile*>(&value);
        (void) dummy;
#endif
    }

    /// @brief Prevents the compiler from reordering memory operations.
    ///
    /// @details This function uses compiler-specific intrinsics or assembly instructions
    /// to create a memory barrier, ensuring that memory operations are not reordered
    /// across the barrier. This is useful in benchmarking to ensure accurate timing
    /// measurements by preventing the compiler from optimizing away or reordering
    /// memory accesses.
    ///
    /// @note This function has different implementations depending on the compiler:
    /// - For Clang and GCC, it uses an empty inline assembly statement with a memory clobber.
    /// - For MSVC, it uses the _ReadWriteBarrier intrinsic.
    /// - For other compilers, it uses a volatile dummy variable.
    ///
    /// @warning This function should be used with caution, as improper use of memory barriers
    /// can lead to undefined behavior or performance issues.
    NGIN_ALWAYS_INLINE void ClobberMemory()
    {
#if defined(__clang__) || defined(__GNUC__)
        asm volatile("" : : : "memory");
#elif defined(_MSC_VER)
        _ReadWriteBarrier();
#else
        // Fallback for other compilers
        volatile int dummy = 0;
        (void) dummy;
#endif
    }

    struct BenchmarkConfig
    {
        /// @brief The number of iterations to run the benchmark.
        Int32 iterations = 1000;
        /// @brief The number of warmup iterations to run before the benchmark.
        Int32 warmupIterations = 100;
    };

    template<typename DesiredUnit>
        requires IsUnitOf<DesiredUnit, Time>
    struct BenchmarkResult
    {
        /// @brief The name of the benchmark.
        std::string name = "Unknown Benchmark";
        /// @brief The number of iterations run for the benchmark.
        Int32 numIterations = 0;
        /// @brief The average time taken to run the benchmark.
        DesiredUnit averageTime = DesiredUnit(0.0);
        /// @brief The minimum time taken to run the benchmark.
        DesiredUnit minTime = DesiredUnit(0.0);
        /// @brief The maximum time taken to run the benchmark.
        DesiredUnit maxTime = DesiredUnit(0.0);
        /// @brief The standard deviation of the time taken to run the benchmark.
        DesiredUnit standardDeviation = DesiredUnit(0.0);
    };


    /// @brief A simple benchmarking class.
    /// @details
    /// This class provides a simple way to measure the time taken by a block of code.
    class Benchmark
    {
    public:
        // Constructors
        Benchmark(std::string_view benchmarkName = "Unnamed Benchmark")
            : config(defaultConfig), benchmarkFunction(nullptr), name(benchmarkName)
        {
            RegisterBenchmark(this);
        }

        Benchmark(const BenchmarkConfig& cfg, std::string_view benchmarkName = "Unnamed Benchmark")
            : config(cfg), benchmarkFunction(nullptr), name(benchmarkName)
        {
            RegisterBenchmark(this);
        }

        Benchmark(std::function<void()> func, std::string_view benchmarkName = "Unnamed Benchmark")
            : config(defaultConfig), benchmarkFunction(func), name(benchmarkName)
        {
            RegisterBenchmark(this);
        }

        Benchmark(const BenchmarkConfig& cfg, std::function<void()> func, std::string_view benchmarkName = "Unnamed Benchmark")
            : config(cfg), benchmarkFunction(func), name(benchmarkName)
        {
            RegisterBenchmark(this);
        }

        Benchmark(const Benchmark& other)                = default;
        Benchmark(Benchmark&& other) noexcept            = default;
        Benchmark& operator=(const Benchmark& other)     = default;
        Benchmark& operator=(Benchmark&& other) noexcept = default;
        ~Benchmark()                                     = default;

        /// @brief Starts the benchmark.
        /// @return A BenchmarkResult containing the benchmarking statistics.
        template<typename DesiredUnit>
            requires IsUnitOf<DesiredUnit, Time>
        [[nodiscard]] BenchmarkResult<DesiredUnit> Run()
        {
            BenchmarkResult<DesiredUnit> result;
            result.name          = name;
            result.numIterations = static_cast<std::size_t>(config.iterations);

            if (!benchmarkFunction)
            {
                // No function to benchmark
                return result;
            }

            // Warmup iterations
            for (Int32 i = 0; i < config.warmupIterations; ++i)
            {
                benchmarkFunction();
            }

            // Measurement iterations
            std::vector<Nanoseconds> timings;
            timings.reserve(config.iterations);

            Timer timer;

            for (Int32 i = 0; i < config.iterations; ++i)
            {
                timer.Reset();
                timer.Start();
                benchmarkFunction();
                timer.Stop();

                Nanoseconds elapsed = timer.GetElapsed<Nanoseconds>();
                timings.emplace_back(elapsed);
            }

            // Compute statistics
            double sum     = 0.0;
            double minTime = std::numeric_limits<double>::max();
            double maxTime = std::numeric_limits<double>::lowest();

            for (const auto& time: timings)
            {
                double timeVal = time.GetValue();
                sum += timeVal;
                if (timeVal < minTime)
                    minTime = timeVal;
                if (timeVal > maxTime)
                    maxTime = timeVal;
            }

            double average = sum / timings.size();

            // Compute standard deviation
            double variance = 0.0;
            for (const auto& time: timings)
            {
                variance += (time.GetValue() - average) * (time.GetValue() - average);
            }
            variance /= timings.size();
            double stdDev = std::sqrt(variance);

            // Populate result
            result.averageTime       = UnitCast<DesiredUnit>(Nanoseconds(average));
            result.minTime           = UnitCast<DesiredUnit>(Nanoseconds(minTime));
            result.maxTime           = UnitCast<DesiredUnit>(Nanoseconds(maxTime));
            result.standardDeviation = UnitCast<DesiredUnit>(Nanoseconds(stdDev));

            return result;
        }

    private:
        BenchmarkConfig config;
        std::function<void()> benchmarkFunction;
        std::string name;

        // Benchmark Registry
        static std::vector<Benchmark*>& GetRegistry()
        {
            static std::vector<Benchmark*> registry;
            return registry;
        }

        static std::mutex& GetRegistryMutex()
        {
            static std::mutex registryMutex;
            return registryMutex;
        }

        void RegisterBenchmark(Benchmark* benchmark)
        {
            std::lock_guard<std::mutex> lock(GetRegistryMutex());
            GetRegistry().emplace_back(benchmark);
        }

    public:
        inline static BenchmarkConfig defaultConfig = BenchmarkConfig();

        /// @brief Runs all registered benchmarks using the defaultConfig.
        /// @return A vector of BenchmarkResults for each benchmark.
        template<typename DesiredUnit>
            requires IsUnitOf<DesiredUnit, Time>
        static std::vector<BenchmarkResult<DesiredUnit>> RunAll()
        {
            std::vector<BenchmarkResult<DesiredUnit>> results;
            std::lock_guard<std::mutex> lock(GetRegistryMutex());

            for (auto& benchmark: GetRegistry())
            {
                if (benchmark)
                {
                    // Use the default config for all benchmarks in RunAll
                    BenchmarkConfig originalConfig = benchmark->config;
                    benchmark->config              = defaultConfig;

                    BenchmarkResult<DesiredUnit> result = benchmark->Run<DesiredUnit>();
                    results.emplace_back(result);

                    // Restore original config
                    benchmark->config = originalConfig;
                }
            }

            return results;
        }
    };


}// namespace NGIN