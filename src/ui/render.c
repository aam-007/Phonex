#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "../phonex.h"

// --- ANSI CONSTANTS ---
#define CLS         "\033[2J\033[H"
#define COLOR_RESET "\033[0m"
#define COLOR_CYAN  "\033[36m"   // Cash / Safe
#define COLOR_YEL   "\033[33m"   // Warning
#define COLOR_RED   "\033[31m"   // Danger
#define COLOR_WHT   "\033[37m"   // Standard
#define COLOR_GRY   "\033[90m"   // Muted
#define BG_BLK      "\033[40m"

// --- HELPERS ---

void fmt_inr(char *buffer, currency_t val) {
    if (val < 0) {
        sprintf(buffer, "-");
        fmt_inr(buffer + 1, -val);
        return;
    }

    double decimal_part = (val % CURRENCY_SCALE) / (double)CURRENCY_SCALE;
    long long whole_part = val / CURRENCY_SCALE;
    
    char whole_str[32];
    sprintf(whole_str, "%lld", whole_part);
    
    int len = strlen(whole_str);
    char vedic_str[48] = {0};
    int v_idx = 0;
    
    // Vedic logic: 3 digits, then 2, then 2...
    for (int i = 0; i < len; i++) {
        if (i > 0 && (len - i) % 2 == 1 && (len - i) < len - 1) {
             vedic_str[v_idx++] = ',';
        }
        vedic_str[v_idx++] = whole_str[i];
    }
    vedic_str[v_idx] = '\0';

    sprintf(buffer, "%s%s.%.2f", CURRENCY_SYMBOL, vedic_str, decimal_part);
    // Remove leading 0 before decimal if handled poorly by sprintf
    char *dot = strchr(buffer, '.');
    if(dot && *(dot-1) == '0' && *(dot-2) == '.') { 
        // quick fix for purely fractional printing if needed
    }
}

void print_bar(double pct, int width, char *color) {
    int filled = (int)(pct * width);
    if (filled > width) filled = width;
    if (filled < 0) filled = 0;
    
    printf("%s[", color);
    for (int i=0; i<width; i++) {
        printf(i < filled ? "=" : "-");
    }
    printf("]%s", COLOR_RESET);
}

// --- SCREENS ---

void ui_render_login(void) {
    printf("%s%s", CLS, BG_BLK);
    printf("\n\n");
    printf(COLOR_WHT "   PHONEX SYSTEMS <%s>\n", CURRENCY_CODE);
    printf(COLOR_GRY "   ----------------------------------------\n");
    printf(COLOR_WHT "   TERMINAL ID:  PX-01\n");
    printf(COLOR_RED "   RESTRICTED ACCESS // AUTHORIZED PERSONNEL ONLY\n\n");
    printf(COLOR_CYAN "   [ CONNECTING TO NSE FEED... ]\n");
    printf(COLOR_RESET);
}

void ui_render_frame(Portfolio *p, Asset *universe, SimConfig *cfg, int tick) {
    char s_nav[64], s_cash[64], s_liab[64];
    
    fmt_inr(s_nav, p->nav);
    fmt_inr(s_cash, p->cash_balance);
    fmt_inr(s_liab, p->total_liabilities);

    printf("%s", CLS); // Repaint full screen (brute force for simplicity)

    // HEADER
    printf(BG_BLK COLOR_WHT);
    printf("+--------------------------------------------------+--------------------------+\n");
    printf("|  PORTFOLIO (PHONEX<%s>)                         |  MARKET VECTOR           |\n", CURRENCY_CODE);

    // ROW 1: AUM & REGIME
    printf("|  AUM:      %-26s            |  REGIME: %-15d|\n", s_nav, cfg->regime);
    
    // ROW 2: CASH & RATES (Mock rate)
    printf("|  CASH:     %-26s ", s_cash);
    print_bar((double)p->cash_balance / p->nav, 10, COLOR_CYAN);
    printf("        |  RATES:  6.50%% (REPO)   |\n");
    
    // ROW 3: LIABILITIES
    printf("|  DEBT:     %-26s            |  VIX:    %-6.1f       |\n", 
           s_liab, universe[0].volatility * 100);

    // ROW 4: DRAWDOWN
    printf("|  DD:       %-.2f%%        ", p->current_drawdown * 100);
    char *dd_col = p->current_drawdown < -0.10 ? COLOR_RED : COLOR_YEL;
    print_bar(abs(p->current_drawdown) * 5, 12, dd_col); // Scale for visual
    // [FIX] Added (long long) cast below to satisfy %lld
    printf("      |  NIFTY:  %-10lld   |\n", (long long)(universe[0].price / CURRENCY_SCALE));

    printf("+--------------------------------------------------+--------------------------+\n");

    // ALLOCATION SECTION
    printf("|  ALLOCATION (TOP 3)                              |  RISK STATUS             |\n");
    
    for(int i=0; i<3 && i<p->position_count; i++) {
        Position *pos = &p->positions[i];
        if(pos->units == 0) continue;
        
        double alloc_pct = (double)pos->current_val / p->nav;
        char name[16];
        strncpy(name, universe[pos->asset_index].ticker, 10);
        name[10] = '\0';
        
        printf("|  %-10s ", name);
        print_bar(alloc_pct, 20, COLOR_WHT);
        printf(" %3.0f%%           ", alloc_pct * 100);
        
        // Dynamic Risk Column
        if(i==0) {
            if(p->status == STATUS_MARGIN_CALL) 
                printf("|  " COLOR_RED "MARGIN CALL ALERT" COLOR_WHT "     |\n");
            else 
                printf("|   RMS: OK                |\n");
        } else {
            printf("|                          |\n");
        }
    }
    
    // FILLER
    for(int k=0; k < (3 - p->position_count); k++) {
         printf("|                                                  |                          |\n");
    }

    printf("+--------------------------------------------------+--------------------------+\n");
    printf("|  STATUS: %s >> MONTH %d / %d             |  [PRESS ESC TO ABORT]    |\n", 
           p->status == STATUS_ACTIVE ? "RUNNING" : "HALTED", 
           tick, cfg->duration_months);
    printf("+--------------------------------------------------+--------------------------+\n");
    
    // Flash Red if Margin Call
    if (p->status == STATUS_MARGIN_CALL) {
        printf(COLOR_RED "\n   !!! CAPITAL PROTECTION ACTIVATED - LIQUIDATING ASSETS !!! \n" COLOR_RESET);
    }
}