import json
from scipy import stats
import numpy as np
import sys
import argparse
import os

CPU_FREQ = 5.44e9

def read_measurements(file_path):
    """Read measurements from either JSON or TXT file"""
    ext = os.path.splitext(file_path)[1].lower()

    if ext == '.json':
        with open(file_path, 'r') as f:
            return json.load(f)
    elif ext == '.txt':
        with open(file_path, 'r') as f:
            measurements = [float(line.strip()) for line in f if line.strip()]
            # Use filename without extension as category name
            category = os.path.splitext(os.path.basename(file_path))[0]
            return {category: measurements}
    else:
        raise ValueError(f"Unsupported file format: {ext}")

def calculate_confidence_interval(data, confidence=0.95):
    data_ns = [x * 1000000000 for x in data]
    n = len(data_ns)
    mean = np.mean(data_ns)
    se = stats.sem(data_ns)
    ci = stats.t.interval(confidence, n-1, mean, se)
    return mean, ci[0], ci[1]

def print_latex_table(data_dict):
    print(r"\begin{table}[h]")
    print(r"\centering")
    print(r"\begin{tabular}{lccc}")
    print(r"\toprule")
    print(r"\textbf{Category} & \textbf{Mean (ns)} & \textbf{Lower CI (ns)} & \textbf{Upper CI (ns)} \\")
    print(r"\midrule")

    for category, measurements in data_dict.items():
        mean, ci_lower, ci_upper = calculate_confidence_interval(measurements)
        print(f"{category} & {mean:.2f} & {ci_lower:.2f} & {ci_upper:.2f} \\\\")

    print(r"\bottomrule")
    print(r"\end{tabular}")
    print(r"\caption{95\% Confidence Intervals for Measurements}")
    print(r"\label{tab:confidence-intervals}")
    print(r"\end{table}")

def main():
    parser = argparse.ArgumentParser(description='Calculate confidence intervals from measurement files')
    parser.add_argument('files', nargs='+', help='Path to measurement files (JSON or TXT)')
    parser.add_argument('--convert-from-cycles', action='store_true', help='Convert measurements from cycles to nanoseconds')

    args = parser.parse_args()

    try:
        # Combine data from all files
        combined_data = {}
        for file_path in args.files:
            data = read_measurements(file_path)
            combined_data.update(data)

        if args.convert_from_cycles:
            for category, measurements in combined_data.items():
                combined_data[category] = [x / CPU_FREQ for x in measurements]
        

        # Print regular table
        print("95% Confidence Intervals:")
        print("-" * 90)
        print(f"{'Category':<20} {'Mean (ns)':>15} {'Lower CI (ns)':>20} {'Upper CI (ns)':>20}")
        print("-" * 90)

        for category, measurements in combined_data.items():
            mean, ci_lower, ci_upper = calculate_confidence_interval(measurements)
            print(f"{category:<20} {mean:15.2f} {ci_lower:20.2f} {ci_upper:20.2f}")

        print("\n\nLaTeX Table:")
        print("-" * 90)
        print_latex_table(combined_data)

    except FileNotFoundError as e:
        print(f"Error: File not found - {str(e)}")
        sys.exit(1)
    except json.JSONDecodeError as e:
        print(f"Error: Invalid JSON file - {str(e)}")
        sys.exit(1)
    except ValueError as e:
        print(f"Error: {str(e)}")
        sys.exit(1)
    except Exception as e:
        print(f"Error: {str(e)}")
        sys.exit(1)

if __name__ == "__main__":
    main()
