# PHONEX Asset Management Simulator (India Edition)

**Version:** 1.0.0
**Platform:** Linux | macOS | WSL

A deterministic, terminal-based asset management simulator designed for the Indian market. PHONEX models portfolio dynamics across equities, bonds, and cash under various market regimes, emphasizing precision, risk management, and real-time visualization.

## Overview

PHONEX (Phoenix Systems) is built for educational, risk management, and strategy testing purposes. It simulates realistic portfolio behavior using:

- **Absolute precision** in currency calculations using micros (1,000,000 micros = ₹1.00)
- **Risk management enforcement** including drawdown limits and leverage constraints
- **Market regime modeling** spanning bull markets, stagflation, and liquidity crises
- **ANSI-based terminal UI** providing real-time portfolio performance visualization

## Features

### Multi-Asset Portfolio Engine
Tracks NAV, cash, liabilities, positions, P&L, and allocation percentages across multiple asset classes.

### Risk Management Simulation
Implements comprehensive risk controls:
- Drawdown monitoring
- Leverage limits
- Margin call and liquidation triggers

### Market Engine
Uses deterministic RNG and Geometric Brownian Motion to simulate asset price evolution under different macroeconomic regimes.

### UI & Visualization
ANSI-based terminal rendering featuring:
- Portfolio overview dashboard
- Market vector and volatility indicators
- Allocation bars
- Dynamic alerts for margin calls or insolvency

### Scenario Testing
Simulates bull, stagflation, liquidity crunch, and custom user regimes for comprehensive strategy stress testing.

## Getting Started

### Prerequisites

- **Operating System**: Linux, macOS, or WSL (Windows Subsystem for Linux)
- **Compiler**: GCC with C17 support
- **Terminal**: ANSI-compatible terminal emulator

### Installation

1. **Clone the repository**
```bash
git clone https://github.com/aam-007/phonex.git
cd phonex
```

2. **Build the project**
```bash
make
```

This compiles the source files and produces the executable `phonex_am`.

3. **Run the simulator**
```bash
make run
```

### Directory Structure

```
PHONEX/
├── Makefile
├── README.md
├── LICENSE
└── src/
    ├── core/
    │   ├── accounting.c
    │   └── main.c
    ├── fin/
    │   └── market_gen.c
    ├── phonex.h
    └── ui/
        └── render.c
```

## Usage

Upon launching, the simulator will:

1. Display a login screen
2. Prompt for simulation configuration:
   - **Duration**: 12 – 360 months
   - **Market Regime**: Growth, Stagflation, Crash
   - **Max Drawdown**: e.g., 15%
   - **Allow Margin**: Yes/No
3. Initialize the portfolio and market
4. Run a month-by-month simulation with ANSI-rendered output

**Tip**: Press `ESC` at any time to abort the simulation.

### Cleaning Build Artifacts

```bash
make clean
```

## Output Explanation

| Metric | Description |
|--------|-------------|
| **NAV** | Net Asset Value of the portfolio |
| **CASH** | Liquid cash in the portfolio |
| **DEBT** | Outstanding liabilities/margin |
| **DD** | Drawdown (% drop from high-water mark) |
| **Allocation Bars** | Visual representation of top positions |
| **RMS Alerts** | Indicates margin calls or risk limit breaches |
| **STATUS** | RUNNING, HALTED, or INSOLVENT |

## Market Regimes

| Regime | Description | Typical Effect on Portfolio |
|--------|-------------|----------------------------|
| **Growth** | Stable economic expansion | NAV generally rises, low drawdown |
| **Stagflation** | High inflation, low growth | Bond volatility increases, equities struggle |
| **Crash** | Sharp market decline | Significant NAV drop, possible margin calls |
| **Liquidity Crunch** | Credit freeze | Both bonds and equities face volatility |

## Real-World Applications

- **Portfolio Risk Management & Stress Testing**: Test strategies under adverse conditions
- **Asset Allocation Strategy Testing**: Optimize portfolio composition
- **Financial Education**: Training tool for students and analysts
- **Scenario Analysis**: Model outcomes based on macroeconomic conditions

## Technical Notes

- **Currency Precision**: All monetary values are stored in micros to prevent rounding errors
- **No Floating Point in Ledger Updates**: Only used for display and ratio calculations
- **ANSI Terminal Only**: Optimized for Linux/macOS terminals; Windows support via WSL


---

**Disclaimer**: This simulator is for educational and research purposes only. It should not be used as the sole basis for investment decisions.
