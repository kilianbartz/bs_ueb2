#!/bin/bash

# Check if the user provided the required argument
if [ $# -ne 1 ]; then
    echo "Usage: $0 <k>"
    exit 1
fi

# Number of times to restart the client
k=$1

output_file="client_results.json"

echo "[]" > $output_file

for ((i=1; i<=k; i++)); do
    echo "Running client iteration $i..."

    output=$(./client)

    avg_time=$(echo "$output" | grep -oP 'Average time per iteration: \K[0-9.]+')

    if [ -z "$avg_time" ]; then
        echo "Error: Could not extract average time from client output."
        exit 1
    fi

    echo "Iteration $i: Average time per iteration: $avg_time cycles"

    jq ". += [{\"iteration\": $i, \"average_time\": $avg_time}]" $output_file > tmp.json && mv tmp.json $output_file
done

echo "Results saved to $output_file"
