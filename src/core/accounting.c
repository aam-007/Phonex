#include <stdio.h>
#include <stdlib.h>
#include "../phonex.h"

// --- INIT ---

void portfolio_init(Portfolio *p, currency_t initial_capital) {
    p->cash_balance = initial_capital;
    p->total_asset_value = 0;
    p->total_liabilities = 0;
    p->nav = initial_capital;
    
    p->high_water_mark = initial_capital;
    p->current_drawdown = 0.0;
    p->leverage_ratio = 0.0;
    p->status = STATUS_ACTIVE;
    p->months_underwater = 0;
    p->position_count = 0;

    // Clear positions
    for(int i=0; i<MAX_ASSETS; i++) {
        p->positions[i].units = 0;
        p->positions[i].asset_index = -1;
    }
}

// --- VALUATION ---

void portfolio_update_valuation(Portfolio *p, Asset *universe) {
    currency_t sum_market_val = 0;

    // 1. Mark to Market all positions
    for (int i = 0; i < p->position_count; i++) {
        Position *pos = &p->positions[i];
        Asset *a = &universe[pos->asset_index];
        
        // Calculate current value
        pos->current_val = pos->units * a->price;
        
        // Calculate Unrealized PnL (Current - Cost)
        pos->pnl_unrealized = pos->current_val - (pos->units * pos->cost_basis);
        
        sum_market_val += pos->current_val;
    }

    p->total_asset_value = sum_market_val;

    // 2. Calculate NAV
    // NAV = (Cash + Assets) - Liabilities
    p->nav = (p->cash_balance + p->total_asset_value) - p->total_liabilities;

    // 3. Update Risk Metrics
    
    // Leverage = Total Exposure / NAV
    if (p->nav > 0) {
        p->leverage_ratio = (double)(p->total_asset_value) / (double)p->nav;
    } else {
        p->leverage_ratio = 999.9; // Infinite/Insolvent
    }

    // High Water Mark & Drawdown
    if (p->nav > p->high_water_mark) {
        p->high_water_mark = p->nav;
        p->current_drawdown = 0.0;
        p->months_underwater = 0;
    } else {
        currency_t diff = p->high_water_mark - p->nav;
        p->current_drawdown = -((double)diff / (double)p->high_water_mark);
        p->months_underwater++;
    }
}

// --- THE AUDIT (CRITICAL) ---

bool portfolio_audit(Portfolio *p) {
    // The Accounting Equation: Equity = Assets - Liabilities
    // In our struct: NAV = (Cash + Securities) - Liabilities
    
    currency_t calculated_nav = (p->cash_balance + p->total_asset_value) - p->total_liabilities;
    
    // Check for exact equality
    if (calculated_nav != p->nav) {
        // LOG ERROR INTERNAL
        return false; // CORRUPTION DETECTED
    }

    if (p->nav < 0) {
        p->status = STATUS_INSOLVENT;
    }

    return true;
}

// --- EXECUTION LOGIC ---

void execution_check_constraints(Portfolio *p, SimConfig *cfg) {
    if (p->status == STATUS_INSOLVENT) return;

    // 1. Check Drawdown Limit (Stop Loss)
    // Note: drawdown is stored as negative (e.g., -0.20)
    if (p->current_drawdown < -(cfg->max_drawdown_limit)) {
        // RMS Trigger
        p->status = STATUS_LIQUIDATED;
        // In a real engine, this would trigger a sell-all order
    }

    // 2. Check Margin (Leverage)
    if (p->leverage_ratio > cfg->max_leverage) {
        p->status = STATUS_MARGIN_CALL;
    }
    
    // 3. Recovery
    if (p->status == STATUS_MARGIN_CALL && p->leverage_ratio <= cfg->max_leverage) {
        p->status = STATUS_WARNING;
    }
}