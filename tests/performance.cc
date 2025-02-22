#include <assert.h>
#include <chrono>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <string>

#include "test.h"

// #define PREINPUT
#define GET
#define PUT
// #define SCAN
// #define DEL
// #define PUT_COMPACT

#define ALLCACHEED
// #define CHACHE_INDEX_ONLY
// #define NOCHACHEED

class PerformanceTest : public Test
{
  private:
    const uint64_t SIMPLE_TEST_MAX = 512;
    const uint64_t MIDDLE_TEST_MAX = 1024 * 16;
    const uint64_t BIGGER_MIDDLE_TEST_MAX = 1024 * 32;
    const uint64_t LARGE_TEST_MAX = 1024 * 64;
    const uint64_t GC_TEST_MAX = 1024 * 48;

    void regular_test(uint64_t max, std::ofstream &outfile)
    {
        uint64_t i;

        std::chrono::_V2::system_clock::time_point start; // start timer
        std::chrono::_V2::system_clock::time_point end;   // end timer
        std::chrono::duration<double> diff;               // calculate duration
        double duration;                                  // calculate duration
        double throughput;                                // calculate throughput

#ifdef PREINPUT
        for (i = 0; i < max; ++i) {
            store.put(i, std::string(i + 1, 's'));
        }
#endif

#ifdef PUT
        // Test put performance
        start = std::chrono::high_resolution_clock::now(); // start timer
                                                           // Test multiple key-value pairs
        for (i = 0; i < max; ++i) {
            store.put(i, std::string(i + 1, 's'));
        }
        end = std::chrono::high_resolution_clock::now();  // end timer
        diff = end - start; // calculate duration
        duration = diff.count();
        throughput = max / duration; // calculate throughput

        outfile << "Put performance test\n";
        outfile << "Number of operations: " << max << "\n";
        outfile << "Average latency: " << (duration / max) * 1000
                << " milliseconds\n"; // write average latency to file in milliseconds
        outfile << "Throughput: " << throughput << " operations per second\n"; // write throughput to file
        // output divider
        outfile << "----------------------------------------\n";
        outfile << "\n\n";
#endif

#ifdef GET
        // Test get performance
        start = std::chrono::high_resolution_clock::now(); // start timer
        // Test after all insertions
        for (i = 0; i < max; ++i)
            store.get(i);
        end = std::chrono::high_resolution_clock::now(); // end timer
        diff = end - start;                              // calculate duration
        duration = diff.count();
        throughput = max / duration; // calculate throughput

        // output basic information about this test, including test type and the number of operations
        outfile << "Get performance test\n";
#ifdef ALLCACHEED
        outfile << "Cache: All Cached\n";
#endif
#ifdef CHACHE_INDEX_ONLY
        outfile << "Cache: Index Only\n";
#endif
#ifdef NOCHACHEED
        outfile << "Cache: No Cached\n";
#endif
        outfile << "Number of operations: " << max << "\n";
        outfile << "Average latency: " << (duration / max) * 1000
                << " milliseconds\n"; // write average latency to file in milliseconds
        outfile << "Throughput: " << throughput << " operations per second\n"; // write throughput to file
        // output divider
        outfile << "----------------------------------------\n";
        outfile << "\n\n";
        outfile.flush();
#endif

#ifdef SCAN

        // Test scan performance
        // Test scan
        std::list<std::pair<uint64_t, std::string>> list_ans;
        std::list<std::pair<uint64_t, std::string>> list_stu;

        for (i = 0; i < max / 2; ++i) {
            list_ans.emplace_back(std::make_pair(i, std::string(i + 1, 's')));
        }

        start = std::chrono::high_resolution_clock::now(); // start timer
        store.scan(0, max / 2 - 1, list_stu);
        end = std::chrono::high_resolution_clock::now(); // end timer
        diff = end - start;                              // calculate duration
        duration = diff.count();
        throughput = max / duration; // calculate throughput

        // output basic information about this test, including test type and the number of operations
        outfile << "Scan performance test\n";
        outfile << "Number of operations: " << max / 2 << "\n";
        outfile << "Average latency: " << (duration / max) * 1000
                << " milliseconds\n"; // write average latency to file in milliseconds
        outfile << "Throughput: " << throughput << " operations per second\n"; // write throughput to file
        // output divider
        outfile << "----------------------------------------\n";
        outfile << "\n\n";
#endif

#ifdef DEL
        // Test del performance
        start = std::chrono::high_resolution_clock::now(); // start timer
        // Test deletions
        for (i = 0; i < max; i += 2) {
            store.del(i);
        }
        end = std::chrono::high_resolution_clock::now(); // end timer
        diff = end - start;                              // calculate duration
        duration = diff.count();
        throughput = max / duration; // calculate throughput

        // output basic information about this test, including test type and the number of operations
        outfile << "Delete performance test\n";
        outfile << "Number of operations: " << max / 2 << "\n";
        outfile << "Average latency: " << (duration / max) * 1000
                << " milliseconds\n"; // write average latency to file in milliseconds
        outfile << "Throughput: " << throughput << " operations per second\n"; // write throughput to file
        // output divider
        outfile << "----------------------------------------\n";
        outfile << "\n\n";

#endif

#ifdef PUT_COMPACT

        // Test put performance
        start = std::chrono::high_resolution_clock::now(); // start timer
        int operations = 0;
        std::ofstream throughput_file("throughput.txt");

        // Test multiple key-value pairs
        for (i = 0; i < max; ++i) {
            store.put(i, std::string(i + 1, 's'));
            operations++;

            // Check if 100ms has passed
            auto current = std::chrono::high_resolution_clock::now();
            auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(current - start);
            if (elapsed.count() >= 100) {
                // Write the throughput to the file and reset the counters
                throughput_file << operations << "\n";
                operations = 0;
                start = current;
            }
        }
        throughput_file.close();
#endif
    }

    void gc_test(uint64_t max, std::ofstream &outfile)
    {
        uint64_t i;
        uint64_t gc_trigger = 1024;

        for (i = 0; i < max; ++i) {
            store.put(i, std::string(i + 1, 's'));
        }

        for (i = 0; i < max; ++i) {
            EXPECT(std::string(i + 1, 's'), store.get(i));
            switch (i % 3) {
            case 0:
                store.put(i, std::string(i + 1, 'e'));
                break;
            case 1:
                store.put(i, std::string(i + 1, '2'));
                break;
            case 2:
                store.put(i, std::string(i + 1, '3'));
                break;
            default:
                assert(0);
            }

            if (i % gc_trigger == 0) [[unlikely]] {
                check_gc(16 * MB);
            }
        }

        phase();

        for (i = 0; i < max; ++i) {
            switch (i % 3) {
            case 0:
                EXPECT(std::string(i + 1, 'e'), store.get(i));
                break;
            case 1:
                EXPECT(std::string(i + 1, '2'), store.get(i));
                break;
            case 2:
                EXPECT(std::string(i + 1, '3'), store.get(i));
                break;
            default:
                assert(0);
            }
        }

        phase();

        for (i = 1; i < max; i += 2) {
            EXPECT(true, store.del(i));

            if ((i - 1) % gc_trigger == 0) [[unlikely]] {
                check_gc(8 * MB);
            }
        }

        for (i = 0; i < max; i += 2) {
            switch (i % 3) {
            case 0:
                EXPECT(std::string(i + 1, 'e'), store.get(i));
                break;
            case 1:
                EXPECT(std::string(i + 1, '2'), store.get(i));
                break;
            case 2:
                EXPECT(std::string(i + 1, '3'), store.get(i));
                break;
            default:
                assert(0);
            }

            store.del(i);

            if (((i - 1) / 2) % gc_trigger == 0) [[unlikely]] {
                check_gc(32 * MB);
            }
        }

        for (i = 0; i < max; ++i) {
            EXPECT(not_found, store.get(i));
        }

        phase();

        report();
    }

  public:
    PerformanceTest(const std::string &dir, const std::string &vlog, bool v = true) : Test(dir, vlog, v)
    {
    }

    void start_test(void *args = NULL) override
    {
        std::ofstream outfile("./results.txt", std::ios_base::app); // open file in append mode
        auto start = std::chrono::high_resolution_clock::now();     // start timer

        std::cout << "KVStore Correctness Test" << std::endl;
        outfile << "Bloom Filter size: 12 kB\n";
        store.reset();

        std::cout << "[Simple Test]" << std::endl;
        regular_test(SIMPLE_TEST_MAX, outfile);

        store.reset();

        std::cout << "[Middle Test]" << std::endl;
        regular_test(MIDDLE_TEST_MAX, outfile);

        store.reset();

        std::cout << "[Bigger Middle Test]" << std::endl;
        regular_test(BIGGER_MIDDLE_TEST_MAX, outfile);

        store.reset();

        std::cout << "[Large Test]" << std::endl;
        regular_test(LARGE_TEST_MAX, outfile);
    }
};

int main(int argc, char *argv[])
{
    bool verbose = (argc == 2 && std::string(argv[1]) == "-v");

    std::cout << "Usage: " << argv[0] << " [-v]" << std::endl;
    std::cout << "  -v: print extra info for failed tests [currently ";
    std::cout << (verbose ? "ON" : "OFF") << "]" << std::endl;
    std::cout << std::endl;
    std::cout.flush();

    PerformanceTest test("./data", "./data/vlog", verbose);

    test.start_test();

    return 0;
}
