#include <iostream>
#include <new>
#include <stdexcept>
#include <chrono>
#include "state.h"

int main()
{
    std::cout << "Qubit limit stress test (n=25 to n=35)\n";
    std::cout << "Each amplitude = " << sizeof(Amplitude) << " bytes  (2x double)\n\n";

    for (int n = 25; n <= 35; ++n)
    {
        // 1 << n is UB for signed int when n >= 31, so use size_t arithmetic
        size_t count = size_t(1) << n;
        size_t bytes = count * sizeof(Amplitude);

        double mb = bytes / (1024.0 * 1024.0);
        double gb = mb / 1024.0;

        std::string size_str;
        if (gb >= 1.0)
            size_str = std::to_string((int)gb) + " GB";
        else
            size_str = std::to_string((int)mb) + " MB";

        std::cout << "n=" << n << "  (" << count << " amplitudes, ~" << size_str << ")  ... ";
        std::cout.flush();

        try
        {
            auto t0 = std::chrono::steady_clock::now();
            QuantumState q(n);
            auto t1 = std::chrono::steady_clock::now();

            long long ms = std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count();
            std::cout << "\033[32mOK\033[0m  (" << ms << " ms)\n";
        }
        catch (const std::bad_alloc &)
        {
            std::cout << "\033[31mFAIL\033[0m  (std::bad_alloc — out of memory)\n";
        }
        catch (const std::exception &e)
        {
            std::cout << "\033[31mFAIL\033[0m  (" << e.what() << ")\n";
        }
    }

    return 0;
}
