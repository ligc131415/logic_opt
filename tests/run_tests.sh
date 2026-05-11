#!/bin/bash
# Run tests script

set -e

echo "Running GoogleTest tests..."

cd "$(dirname "$0")"

if [ ! -f logic_opt_tests ]; then
    echo "Error: logic_opt_tests not found. Run cmake and make first."
    exit 1
fi

./logic_opt_tests "$@"
