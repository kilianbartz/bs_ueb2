#!/bin/bash

# Check if the correct number of arguments is provided
if [ "$#" -ne 2 ]; then
    echo "Usage: $0 <source_list_file> <k>"
    exit 1
fi

# Read arguments
source_list_file=$1
k=$2

# Check if the source list file exists
if [ ! -f "$source_list_file" ]; then
    echo "Error: File '$source_list_file' not found!"
    exit 1
fi

# Loop through each source file in the list
while IFS= read -r source_file; do
    # Check if the source file exists
    if [ ! -f "$source_file" ]; then
        echo "Error: Source file '$source_file' not found!"
        continue
    fi

    # Extract the base name of the source file (without extension)
    base_name=$(basename "$source_file" .cpp)

    # Compile the source file
    g++ -o "$base_name" "$source_file" -O2 -lzmq -I../cppzmq
    if [ $? -ne 0 ]; then
        echo "Error: Failed to compile '$source_file'. Skipping..."
        continue
    fi

    # Run the compiled program `k` times and collect the output
    echo "Running '$base_name' $k times:"
    for ((i = 1; i <= k; i++)); do
        echo "Run $i:"
        chrt -r 90 ./"$base_name"
    done

    # Clean up the compiled executable
    rm -f "$base_name"

done < "$source_list_file"
