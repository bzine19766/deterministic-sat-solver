# Deterministic SAT Solver (Nitro-Basin)

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![C++17](https://img.shields.io/badge/C%2B%2B-17-blue.svg)](https://isocpp.org/)
[![Success Rate](https://img.shields.io/badge/success-99.3%25-brightgreen.svg)]()
[![Benchmarks](https://img.shields.io/badge/benchmarks-6%2C530-blue.svg)]()

A **deterministic** local search SAT solver that challenges the 30-year assumption that randomness is essential for solving Boolean Satisfiability problems.

## 🚀 Quick Start (Google Colab)

The easiest way to run Nitro-Basin is using our pre-configured Colab notebook:

**[🔗 Open in Google Colab](https://colab.research.google.com/drive/1VrEJf19LiVne1pP4TZoq3RRQwqBk-AbF?usp=sharing)**

### Step-by-Step Instructions:

1. **Open the Colab notebook** using the link above
2. **Mount Google Drive** when prompted (Cell 1)
3. **Prepare your CNF files** in your Google Drive at:
4. - Download benchmarks from [SATLIB](https://www.cs.ubc.ca/~hoos/SATLIB/benchm.html)
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

## 📊 Ablation Study Results

Run the Colab notebook to reproduce these results:

| Configuration | Success | Mean Time | vs FULL |
|---------------|---------|-----------|---------|
| FULL (baseline) | 99.3% | 0.534s | — |
| NO_TABU | 90.7% | 0.317s | -8.6% |
| NO_CLAUSE_WEIGHTING | 94.6% | 0.248s | -4.7% |
| NO_ADAPTIVE_MUTATION | 99.2% | 0.526s | -0.1% |
| NO_MUTATION | 99.2% | 0.537s | -0.1% |
| NO_LAST_CLAUSE | 99.3% | 0.372s | 0.0% |
| NO_CYCLE_DETECTION | 99.3% | 0.271s | 0.0% |
| NO_ANCHOR_LOCKING | 99.2% | — | -0.1% |
| NO_FREQUENCY_BIAS | 99.3% | 0.433s | 0.0% |

## 🔧 Local Installation (Alternative)

If you prefer to run locally:

```bash
# Clone the repository
git clone https://github.com/yourusername/deterministic-sat-solver.git
cd deterministic-sat-solver

# Compile
g++ -O3 -std=c++17 -o nitro-basin src/pagerank_sat_solver.cpp

# Run on a single file
./nitro-basin input.cnf output.sat
