# deterministic-sat-solver
Deterministic SAT solver using PageRank-guided local search. Challenges 30-year assumption that randomness is essential. 99.3% success on 6,530 benchmarks. Ablation study shows deterministic-only achieves 99.2%.

# Deterministic SAT Solver (Nitro-Basin)

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![C++17](https://img.shields.io/badge/C%2B%2B-17-blue.svg)](https://isocpp.org/)
[![Success Rate](https://img.shields.io/badge/success-99.3%25-brightgreen.svg)]()
[![Benchmarks](https://img.shields.io/badge/benchmarks-6%2C530-blue.svg)]()

A **deterministic** local search SAT solver that challenges the 30-year assumption that randomness is essential for solving Boolean Satisfiability problems.

## 🔬 Key Results

| Metric | Value |
|--------|-------|
| **Success Rate** | 99.3% (6,484/6,530) |
| **Median Runtime** | 0.05 seconds |
| **Geometric Mean** | 0.108 seconds |
| **Deterministic Only** | 99.2% (no mutations) |
| **Portfolio Coverage** | 99.6% |

## 🏆 Revolutionary Finding

> *Randomness is not necessary for SAT solving – it only accelerates convergence.*

A purely deterministic configuration achieves **99.2% success** – statistically indistinguishable from stochastic variants (99.3%).

## 📊 Ablation Study Results

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

## 🚀 Building

```bash
# Clone the repository
git clone https://github.com/yourusername/deterministic-sat-solver.git
cd deterministic-sat-solver

# Compile with optimizations
g++ -O3 -std=c++17 -o nitro-basin src/pagerank_sat_solver.cpp

# Or use the Makefile
make
