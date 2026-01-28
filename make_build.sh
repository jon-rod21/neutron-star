#!/bin/bash
cmake -DCMAKE_EXPORT_COMMANDS=1 -B build
ln -sf build/compile_commands.json .
