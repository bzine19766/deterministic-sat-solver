# Deterministic SAT Solver (Nitro-Basin)

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![C++17](https://img.shields.io/badge/C%2B%2B-17-blue.svg)](https://isocpp.org/)
[![Success Rate](https://img.shields.io/badge/success-99.3%25-brightgreen.svg)]()
[![Benchmarks](https://img.shields.io/badge/benchmarks-6%2C530-blue.svg)]()

A **deterministic** local search SAT solver that challenges the 30-year assumption that randomness is essential for solving Boolean Satisfiability problems.

> *"Randomness is not necessary for SAT solving – it only accelerates convergence."*

## 📋 Table of Contents

- [Key Results](#key-results)
- [Complete Ablation Results](#complete-ablation-results)
- [Quick Start (Google Colab)](#quick-start-google-colab)
- [Local Installation](#local-installation)
- [Ablation Study Results](#ablation-study-results)
- [Benchmark Sources](#benchmark-sources)
- [Repository Structure](#repository-structure)
- [Citation](#citation)
- [License](#license)

## 🔬 Key Results

| Metric | Value |
|--------|-------|
| **Success Rate** | 99.3% (6,484/6,530) |
| **Median Runtime** | 0.05 seconds |
| **Geometric Mean** | 0.108 seconds |
| **Deterministic Only** | 99.2% (no mutations) |
| **Portfolio Coverage** | 99.6% |

## 📦 Complete Ablation Results  
### What's Included:

| File Type | Description |
|-----------|-------------|
| **`.log` files** | Complete execution logs with per-file results (SUCCESS/FAILED/TIMEOUT) |
| **`.sat` files** | Solution files for successfully solved CNF instances |
| **`.csv` files** | Summary statistics for all configurations |
| **`.json` files** | Machine-readable results for further analysis |

## 🚀 Quick Start (Google Colab)

The easiest way to run Nitro-Basin is using our pre-configured Colab notebook:

**[🔗 Open in Google Colab](https://colab.research.google.com/drive/1VrEJf19LiVne1pP4TZoq3RRQwqBk-AbF?usp=sharing)**

### Step-by-Step Instructions:

1. **Open the Colab notebook** using the link above
2. **Mount Google Drive** when prompted (Cell 1)
3. **Prepare your CNF files** in your Google Drive at:
   - Download benchmarks from [SATLIB](https://www.cs.ubc.ca/~hoos/SATLIB/benchm.html)
   - Place `.cnf` files in the `cnffiles` folder
4. **Run Cell 2** – Copies CNF files to local RAM for maximum performance
5. **Run Cell 3** – Compiles the C++ solver
6. **Run Cell 4** – Runs ablation analysis on results

### Features of the Colab Notebook:

| Feature | Description |
|---------|-------------|
| **Auto-mount Drive** | Automatically connects to your Google Drive |
| **RAM Caching** | Copies CNF files to local RAM for faster I/O |
| **Progress Tracking** | Shows real-time copy progress with speed and ETA |
| **Ablation Analysis** | Complete configuration comparison matrix |
| **Resume Capable** | Can resume interrupted ablation runs |

## 🔧 Local Installation

If you prefer to run locally:

```bash
# Clone the repository
git clone https://github.com/yourusername/deterministic-sat-solver.git
cd deterministic-sat-solver

# Compile with optimizations
g++ -O3 -std=c++17 -o nitro-basin src/pagerank_sat_solver.cpp

# Run on a single CNF file
./nitro-basin input.cnf output.sat

# Example output:
# TIME:0.053
# FLIPS:1234
# MUTATIONS:0
# RESULT:SUCCESS

All ablation study results, including detailed logs and solution files, are available in the `results/` folder of this repository:
