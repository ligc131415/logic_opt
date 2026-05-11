#!/bin/bash
# Generate coverage report script

set -e

echo "Generating coverage report..."

cd "$(dirname "$0")"

# Build with coverage if needed
if [ ! -f logic_opt_tests ]; then
    echo "Building with coverage..."
    mkdir -p build
    cd build
    cmake -DCMAKE_BUILD_TYPE=Coverage ..
    make
    cd ..
fi

# Run tests to generate coverage data
./logic_opt_tests || true

# Generate coverage report
if command -v gcov &> /dev/null; then
    echo ""
    echo "=== Coverage Report ==="

    # Run gcov on source files
    echo ""
    echo "Source File Coverage:"
    for file in src/**/*.cpp; do
        if [ -f "$file" ]; then
            echo ""
            echo "--- $file ---"
            gcov -p $file 2>/dev/null || true
        fi
    done

    # Generate summary
    echo ""
    echo "=== Summary ==="
    lcov --capture --directory . --output-file coverage.info 2>/dev/null || true
    genhtml coverage.info --output-directory cov_html 2>/dev/null || true

    if [ -f cov_html/index.html ]; then
        echo ""
        echo "HTML report generated at: cov_html/index.html"
    fi
else
    echo "gcov not installed. Install with: sudo apt-get install gcov lcov"
fi
