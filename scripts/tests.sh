#!/usr/bin/env bash
cd test && cmake -S . -B build/. && cmake --build build/. && ./build/run_tests
