#!/usr/bin/env python3
"""
COMPLETE ABLATION ANALYSIS - Which Configuration Solves What
Detailed breakdown by problem family and file-level comparison
"""

import os
import re
import glob
import pandas as pd
import numpy as np
from collections import defaultdict
from datetime import datetime

# ============================================================================
# CONFIGURATION
# ============================================================================

DRIVE_DIR = "/content/drive/MyDrive/NitroBasin_Ablation_Results"

# Problem family classification
FAMILY_PATTERNS = {
    'Logistics': r'logistics',
    'AIS': r'ais\d+',
    'Flat': r'flat\d+',
    'UF': r'uf\d+',
    'CBS': r'CBS',
    'QG': r'qg\d',
    'SW': r'sw',
    'Other': r'.*'
}

# All configurations
CONFIGURATIONS = [
    "FULL",
    "NO_TABU",
    "NO_ADAPTIVE_MUTATION",
    "NO_MUTATION",
    "NO_CLAUSE_WEIGHTING",
    "NO_LAST_CLAUSE",
    "NO_CYCLE_DETECTION",
    "NO_ANCHOR_LOCKING",
    "NO_FREQUENCY_BIAS"
]

# ============================================================================
# FUNCTIONS
# ============================================================================

def get_family(filename):
    """Determine problem family from filename"""
    for family, pattern in FAMILY_PATTERNS.items():
        if re.search(pattern, filename, re.IGNORECASE):
            return family
    return 'Other'

def get_solved_files_from_log(config_dir, config_name):
    """Extract solved file names from log file"""
    log_file = os.path.join(config_dir, f"{config_name}.log")
    solved = set()
    unsolved = set()

    if not os.path.exists(log_file):
        return solved, unsolved

    with open(log_file, 'r') as f:
        for line in f:
            match = re.match(r'^([^|]+)\s*\|\s*(SUCCESS|FAILED|TIMEOUT)', line)
            if match:
                file_name = match.group(1).strip()
                status = match.group(2).strip()
                if status == "SUCCESS":
                    solved.add(file_name)
                else:
                    unsolved.add(file_name)

    return solved, unsolved

def get_all_files():
    """Get all unique files across all configurations"""
    all_files = set()
    for config in CONFIGURATIONS:
        config_dir = os.path.join(DRIVE_DIR, config)
        solved, unsolved = get_solved_files_from_log(config_dir, config)
        all_files.update(solved)
        all_files.update(unsolved)
    return sorted(list(all_files))

# ============================================================================
# MAIN ANALYSIS
# ============================================================================

def analyze_configuration_solutions():
    """Analyze which configuration solves which file"""

    print("\n" + "="*80)
    print("📊 COMPLETE ABLATION ANALYSIS - Which Configuration Solves What")
    print("="*80)

    # Get all solutions for each config
    config_solutions = {}
    for config in CONFIGURATIONS:
        config_dir = os.path.join(DRIVE_DIR, config)
        solved, unsolved = get_solved_files_from_log(config_dir, config)
        config_solutions[config] = {
            'solved': solved,
            'unsolved': unsolved,
            'solved_count': len(solved),
            'unsolved_count': len(unsolved)
        }
        print(f"\n{config}:")
        print(f"  Solved: {len(solved)} files")
        print(f"  Unsolved: {len(unsolved)} files")

    # Get all files
    all_files = get_all_files()

    # Create detailed matrix
    print("\n" + "="*80)
    print("📋 DETAILED FILE-BY-FILE ANALYSIS")
    print("="*80)

    # Organize by family
    family_files = defaultdict(list)
    for file in all_files:
        family = get_family(file)
        family_files[family].append(file)

    for family in sorted(family_files.keys()):
        files = sorted(family_files[family])
        print(f"\n{'─'*60}")
        print(f"🔷 {family} Family ({len(files)} files)")
        print(f"{'─'*60}")

        # Create table for this family
        print(f"\n{'File':<35}", end='')
        for config in CONFIGURATIONS:
            short_name = config.replace('NO_', '').replace('_', '')[:5]
            print(f"{short_name:<6}", end='')
        print()
        print("-"*80)

        for file in files[:50]:  # Limit to 50 per family for display
            print(f"{file:<35}", end='')
            for config in CONFIGURATIONS:
                if file in config_solutions[config]['solved']:
                    print(f"{'✓':<6}", end='')
                else:
                    print(f"{'✗':<6}", end='')
            print()

# ============================================================================
# UNIQUE SOLUTIONS ANALYSIS
# ============================================================================

def analyze_unique_solutions():
    """Find files uniquely solved by each configuration"""

    print("\n" + "="*80)
    print("🔍 UNIQUE SOLUTIONS - Files Solved by Only One Configuration")
    print("="*80)

    # Get all solutions
    config_solutions = {}
    for config in CONFIGURATIONS:
        config_dir = os.path.join(DRIVE_DIR, config)
        solved, _ = get_solved_files_from_log(config_dir, config)
        config_solutions[config] = solved

    # Find unique solutions
    unique_counts = defaultdict(int)
    unique_files = defaultdict(list)

    for file in get_all_files():
        solvers = []
        for config in CONFIGURATIONS:
            if file in config_solutions[config]:
                solvers.append(config)

        if len(solvers) == 1:
            unique_counts[solvers[0]] += 1
            unique_files[solvers[0]].append(file)

    print("\n📊 Unique Solutions Count:")
    print("-"*50)
    for config in CONFIGURATIONS:
        count = unique_counts[config]
        if count > 0:
            print(f"  {config}: {count} files uniquely solved")

    # Show examples
    print("\n📋 Examples of Uniquely Solved Files:")
    print("-"*50)
    for config in CONFIGURATIONS:
        if unique_files[config]:
            print(f"\n  {config} (only this config solves):")
            for file in unique_files[config][:5]:
                family = get_family(file)
                print(f"    - {file} [{family}]")

# ============================================================================
# COMPLEMENTARY PAIRS ANALYSIS
# ============================================================================

def analyze_complementary_pairs():
    """Find files where two configurations complement each other"""

    print("\n" + "="*80)
    print("🔄 COMPLEMENTARY PAIRS - Configurations That Solve Different Files")
    print("="*80)

    config_solutions = {}
    for config in CONFIGURATIONS:
        config_dir = os.path.join(DRIVE_DIR, config)
        solved, _ = get_solved_files_from_log(config_dir, config)
        config_solutions[config] = solved

    # Compare each pair
    for i, cfg1 in enumerate(CONFIGURATIONS):
        for cfg2 in CONFIGURATIONS[i+1:]:
            files1 = config_solutions[cfg1]
            files2 = config_solutions[cfg2]

            only_cfg1 = files1 - files2
            only_cfg2 = files2 - files1
            common = files1 & files2

            if len(only_cfg1) > 0 and len(only_cfg2) > 0:
                print(f"\n  {cfg1} ↔ {cfg2}:")
                print(f"    Common: {len(common)} files")
                print(f"    Only {cfg1}: {len(only_cfg1)} files")
                print(f"    Only {cfg2}: {len(only_cfg2)} files")

                # Show examples
                if len(only_cfg1) > 0:
                    example = list(only_cfg1)[0]
                    family = get_family(example)
                    print(f"    Example only {cfg1}: {example} [{family}]")
                if len(only_cfg2) > 0:
                    example = list(only_cfg2)[0]
                    family = get_family(example)
                    print(f"    Example only {cfg2}: {example} [{family}]")

# ============================================================================
# PORTFOLIO ANALYSIS
# ============================================================================

def analyze_portfolio_coverage():
    """Analyze how many files can be solved by combining configurations"""

    print("\n" + "="*80)
    print("🎯 PORTFOLIO ANALYSIS - Combining Configurations")
    print("="*80)

    config_solutions = {}
    for config in CONFIGURATIONS:
        config_dir = os.path.join(DRIVE_DIR, config)
        solved, _ = get_solved_files_from_log(config_dir, config)
        config_solutions[config] = solved

    all_files = get_all_files()
    total_files = len(all_files)

    # Calculate coverage for different combinations
    print("\n📊 Coverage by Configuration Combination:")
    print("-"*60)

    # Single best config
    best_config = max(CONFIGURATIONS, key=lambda c: len(config_solutions[c]))
    best_coverage = len(config_solutions[best_config])
    print(f"  Best single ({best_config}): {best_coverage}/{total_files} ({best_coverage/total_files*100:.1f}%)")

    # FULL + Best alternative
    full_solved = config_solutions['FULL']
    for alt in ['NO_LAST_CLAUSE', 'NO_CYCLE_DETECTION', 'NO_ANCHOR_LOCKING', 'NO_FREQUENCY_BIAS']:
        combined = full_solved | config_solutions[alt]
        coverage = len(combined)
        gain = coverage - len(full_solved)
        print(f"  FULL + {alt}: {coverage}/{total_files} ({coverage/total_files*100:.1f}%) [+{gain} new files]")

    # All configurations
    all_solved = set()
    for config in CONFIGURATIONS:
        all_solved.update(config_solutions[config])
    all_coverage = len(all_solved)
    print(f"  ALL configurations: {all_coverage}/{total_files} ({all_coverage/total_files*100:.1f}%)")

    # Files unsolved by any configuration
    unsolved_files = set(all_files) - all_solved
    print(f"\n❌ Files unsolved by ANY configuration: {len(unsolved_files)}")

    if unsolved_files:
        print("\n  Unsolved files by family:")
        unsolved_by_family = defaultdict(list)
        for f in unsolved_files:
            family = get_family(f)
            unsolved_by_family[family].append(f)

        for family, files in sorted(unsolved_by_family.items()):
            print(f"    {family}: {len(files)} files")
            for f in files[:3]:
                print(f"      - {f}")

# ============================================================================
# FAMILY-WISE SUCCESS MATRIX
# ============================================================================

def create_family_success_matrix():
    """Create success matrix by problem family"""

    print("\n" + "="*80)
    print("📊 FAMILY-WISE SUCCESS MATRIX")
    print("="*80)

    # Collect data
    config_solutions = {}
    for config in CONFIGURATIONS:
        config_dir = os.path.join(DRIVE_DIR, config)
        solved, _ = get_solved_files_from_log(config_dir, config)
        config_solutions[config] = solved

    # Get all files
    all_files = get_all_files()

    # Organize by family
    family_files = defaultdict(list)
    for file in all_files:
        family = get_family(file)
        family_files[family].append(file)

    # Create matrix
    families = sorted(family_files.keys())

    print(f"\n{'Family':<12}", end='')
    for config in CONFIGURATIONS:
        short_name = config.replace('NO_', '').replace('_', '')[:8]
        print(f"{short_name:<10}", end='')
    print()
    print("-"*120)

    for family in families:
        files = family_files[family]
        total = len(files)

        print(f"{family:<12}", end='')
        for config in CONFIGURATIONS:
            solved_count = sum(1 for f in files if f in config_solutions[config])
            pct = solved_count / total * 100 if total > 0 else 0
            if pct >= 99:
                print(f"{'██':<10}", end='')
            elif pct >= 90:
                print(f"{'█▌':<10}", end='')
            elif pct >= 80:
                print(f"{'█ ':5}{pct:.0f}%{'':<3}", end='')
            else:
                print(f"{pct:>5.0f}%{'':<5}", end='')
        print()

# ============================================================================
# MAIN
# ============================================================================

if __name__ == "__main__":
    print("\n" + "="*80)
    print("🔬 NITRO-BASIN ABLATION ANALYSIS")
    print("   Which Configuration Solves What?")
    print("="*80)

    # Run all analyses
    analyze_configuration_solutions()
    analyze_unique_solutions()
    analyze_complementary_pairs()
    analyze_portfolio_coverage()
    create_family_success_matrix()

    print("\n" + "="*80)
    print("✅ ANALYSIS COMPLETE")
    print("="*80)
