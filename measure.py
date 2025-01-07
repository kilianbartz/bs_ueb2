import subprocess
import json
import os
import sys
from pathlib import Path

def compile_program(file_path):
    """Compiles a C++ program and returns the output executable name."""
    executable = Path(file_path).stem  # Use the file name without extension as the executable name
    compile_command = ["g++", "-std=c++20", "-O2", file_path, "-o", executable, "-lzmq"]
    try:
        subprocess.run(compile_command, check=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        return executable
    except subprocess.CalledProcessError as e:
        print(f"Error compiling {file_path}: {e.stderr.decode()}")
        return None

def run_program(executable):
    """Runs the compiled program and captures its output."""
    try:
        command = [f"./{executable}"]
        result = subprocess.run(command, check=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        output = result.stdout.decode().strip()
        return output
    except subprocess.CalledProcessError as e:
        print(f"Error running {executable}: {e.stderr.decode()}")
        return None

def main(file_list, k):
    results = {}

    for file_path in file_list:
        print(f"Processing file: {file_path}")
        executable = compile_program(file_path)
        if not executable:
            continue

        # Run the program `k` times and collect results
        times = []
        for i in range(k):
            print(f"Running {executable} (Run {i + 1}/{k})...")
            output = run_program(executable)
            if output:
                try:
                    # Extract the average time per iteration from the program's output
                    time_per_iteration = float(output.split(":")[-1].strip().split()[0])
                    times.append(time_per_iteration)
                except (ValueError, IndexError):
                    print(f"Unexpected output format from {executable}: {output}")
                    continue

        # Store results for this program
        results[file_path] = times

        # Clean up the executable
        os.remove(executable)

    # Save results to a JSON file
    output_file = "results.json"
    with open(output_file, "w") as f:
        json.dump(results, f, indent=4)

    print(f"Results saved to {output_file}")

if __name__ == "__main__":
    if len(sys.argv) < 3:
        print("Usage: python script.py <file1.cpp> <file2.cpp> ... <k>")
        sys.exit(1)

    # Parse command-line arguments
    *file_list, k = sys.argv[1:]
    k = int(k)

    main(file_list, k)
