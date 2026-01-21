#include <math.h>
#include <string.h>
#include <stdlib.h>
#include "../phonex.h"

// --- DETERMINISTIC RNG (LCG) ---
// Standard constants for a 32-bit generator. 
// Ensures the simulation is identical every time.
static unsigned long _seed = 123456789;

void seed_market(unsigned long seed) {
    _seed = seed;
}

double det_rand() {
    _seed = (_seed * 1103515245 + 12345) & 0x7fffffff;
    return (double)_seed / 2147483648.0;
}

// Box-Muller transform for Normal Distribution
double det_normal() {
    double u = det_rand();
    double v = det_rand();
    // Prevent log(0)
    if(u < 1e-9) u = 1e-9;
    return sqrt(-2.0 * log(u)) * cos(6.28318530718 * v);
}

// --- MARKET LOGIC ---

void market_init_universe(Asset *universe, MarketRegime regime) {
    // 0: NIFTY 50 (Index)
    strcpy(universe[0].ticker, "NIFTY_50");
    strcpy(universe[0].name, "Nifty 50 Index");
    universe[0].type = CLASS_NIFTY_EQ;
    universe[0].price = TO_MICROS(22500.00); // Base level
    universe[0].prev_price = universe[0].price;
    universe[0].volatility = 0.12; // 12% IV
    universe[0].correlation_beta = 1.0;
    universe[0].is_illiquid = false;

    // 1: 10Y G-SEC (Bonds)
    strcpy(universe[1].ticker, "IN_10Y_GS");
    strcpy(universe[1].name, "Govt Bond 7.26% 2033");
    universe[1].type = CLASS_GOVT_BOND;
    universe[1].price = TO_MICROS(100.00);
    universe[1].prev_price = universe[1].price;
    universe[1].volatility = 0.04; // Low vol
    universe[1].correlation_beta = -0.2; // Inverse corr
    universe[1].is_illiquid = false;

    // 2: RELIANCE (High Beta)
    strcpy(universe[2].ticker, "RELIANCE");
    strcpy(universe[2].name, "Reliance Ind.");
    universe[2].type = CLASS_NIFTY_EQ;
    universe[2].price = TO_MICROS(2900.00);
    universe[2].prev_price = universe[2].price;
    universe[2].volatility = 0.22;
    universe[2].correlation_beta = 1.15;
    universe[2].is_illiquid = false;

    // Apply initial Regime modifiers
    if (regime == REGIME_STAGFLATION) {
        universe[1].volatility = 0.15; // Bonds get volatile
    }
}

void market_tick(Asset *universe, int count, MarketRegime regime, int tick) {
    // 1. Determine Macro Factors based on Regime
    double market_drift = 0.0;
    double market_shock = 0.0;

    switch (regime) {
        case REGIME_STABLE_GROWTH:
            market_drift = 0.008; // ~10% annual
            break;
        case REGIME_STAGFLATION:
            market_drift = -0.002;
            market_shock = -0.01;
            break;
        case REGIME_LIQUIDITY_CRUNCH:
            market_drift = -0.05; // Crash
            market_shock = -0.02;
            break;
        default:
            market_drift = 0.005;
    }

    // 2. Apply updates to all assets
    for (int i = 0; i < count; i++) {
        Asset *a = &universe[i];
        a->prev_price = a->price;

        // Geometric Brownian Motion (Discrete)
        // dS = S * (drift * dt + sigma * dZ)
        
        // Calculate drift component
        double r_drift = market_drift * a->correlation_beta;
        
        // Calculate random shock component
        double shock = det_normal() * a->volatility * 0.28; // Monthly vol scaler
        
        // Add forced market shock if correlation is high
        if (a->correlation_beta > 0.5) {
            shock += market_shock;
        }

        // Apply change
        double pct_change = r_drift + shock;
        
        // Update Price (using integer math for storage)
        double new_price_d = FROM_MICROS(a->price) * (1.0 + pct_change);
        
        // Hard floor at 0.01
        if (new_price_d < 0.01) new_price_d = 0.01;
        
        a->price = TO_MICROS(new_price_d);
        
        // Illiquidity Check (Upper/Lower Circuit Mock)
        if (fabs(pct_change) > 0.10) { 
            a->is_illiquid = true; // Locked for trading this tick
        } else {
            a->is_illiquid = false;
        }
    }
}