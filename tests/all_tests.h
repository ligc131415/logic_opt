#include <gtest/gtest.h>

// Main test file - will be compiled with main.cpp
// This file exists to allow test file grouping and organization

namespace logic {

// Force linker to include this file
struct Dummy {
    Dummy() { (void)0; }
};

} // namespace logic
