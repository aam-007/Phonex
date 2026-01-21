#define _POSIX_C_SOURCE 199309L // For nanosleep/usleep
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <termios.h>
#include <fcntl.h>
#include <string.h>
#include <ctype.h>

#include "../phonex.h"

// --- TERMINAL CONTROL ---

struct termios orig_termios;

void reset_terminal_mode() {
    tcsetattr(0, TCSANOW, &orig_termios);
    printf("\033[?25h"); // Show cursor
}

void set_conio_terminal_mode() {
    struct termios new_termios;
    tcgetattr(0, &orig_termios);
    memcpy(&new_termios, &orig_termios, sizeof(new_termios));
    atexit(reset_terminal_mode);
    
    // Disable canonical mode (buffered i/o) and echo
    new_termios.c_lflag &= ~(ICANON | ECHO);
    new_termios.c_cc[VMIN] = 0; // Non-blocking read
    new_termios.c_cc[VTIME] = 0;
    
    tcsetattr(0, TCSANOW, &new_termios);
    printf("\033[?25l"); // Hide cursor
}

// Check if ESC (27) was pressed
bool kbhit_esc() {
    char ch;
    int nread = read(0, &ch, 1);
    if (nread > 0 && ch == 27) return true;
    return false;
}

// --- CONFIG WIZARD ---

void run_wizard(SimConfig *cfg) {
    char input[64];
    
    printf("\n\n   [PHONEX SETUP WIZARD]\n");
    printf("   ---------------------\n");
    
    // 1. DURATION
    printf("   > DURATION (MONTHS) [12-360]: ");
    if (scanf("%d", &cfg->duration_months) != 1) cfg->duration_months = 120;
    if (cfg->duration_months < 12) cfg->duration_months = 12;
    if (cfg->duration_months > 360) cfg->duration_months = 360;

    // 2. REGIME
    printf("   > MARKET REGIME (1=GROWTH, 2=STAGFLATION, 3=CRASH): ");
    int reg_in;
    if (scanf("%d", &reg_in) != 1) reg_in = 1;
    switch(reg_in) {
        case 2: cfg->regime = REGIME_STAGFLATION; break;
        case 3: cfg->regime = REGIME_LIQUIDITY_CRUNCH; break;
        default: cfg->regime = REGIME_STABLE_GROWTH;
    }

    // 3. RISK
    printf("   > MAX DRAWDOWN LIMIT (%%) [e.g. 15]: ");
    double dd;
    if (scanf("%lf", &dd) != 1) dd = 20.0;
    cfg->max_drawdown_limit = dd / 100.0;

    // 4. LEVERAGE
    printf("   > ALLOW MARGIN? (1=YES, 0=NO): ");
    int lev;
    if (scanf("%d", &lev) != 1) lev = 0;
    cfg->allow_margin = (lev == 1);
    cfg->max_leverage = cfg->allow_margin ? 1.5 : 1.0;

    printf("\n   [ SYSTEM LOCKED. INITIALIZING... ]\n");
    sleep(1);
    
    // Flush input buffer before raw mode
    int c; while ((c = getchar()) != '\n' && c != EOF);
}

// --- MAIN RUNTIME ---

int main(void) {
    // 1. INIT
    srand(time(NULL)); // Only for UI jitter, not engine
    ui_render_login();
    
    printf("\n   PRESS [ENTER] TO AUTHENTICATE.");
    getchar();

    // 2. CONFIG
    SimConfig config = {0};
    run_wizard(&config);

    // 3. SETUP ENGINE
    Portfolio port;
    Asset universe[MAX_ASSETS];
    
    // Initial Capital: ₹ 10 Crores
    portfolio_init(&port, TO_MICROS(100000000.00)); 
    market_init_universe(universe, config.regime);

    // Initial Allocation (Simple 60/40 for demo)
    // Buy NIFTY
    port.positions[0].asset_index = 0;
    port.positions[0].units = 2500; // 2500 Units of NIFTY
    port.positions[0].cost_basis = universe[0].price;
    port.positions[0].current_val = universe[0].price * 2500;
    
    // Buy BONDS
    port.positions[1].asset_index = 1;
    port.positions[1].units = 400000; 
    port.positions[1].cost_basis = universe[1].price;
    port.positions[1].current_val = universe[1].price * 400000;

    port.position_count = 2;
    
    // Adjust cash
    currency_t invested = port.positions[0].current_val + port.positions[1].current_val;
    port.cash_balance -= invested;

    // 4. RUN LOOP
    set_conio_terminal_mode(); // ENTER RAW MODE
    
    for (int t = 1; t <= config.duration_months; t++) {
        // A. Tick Market
        market_tick(universe, 3, config.regime, t);
        
        // B. Tick Portfolio
        portfolio_update_valuation(&port, universe);
        
        // C. Audit
        if (!portfolio_audit(&port)) {
            reset_terminal_mode();
            printf("\nFATAL: LEDGER CORRUPTION AT TICK %d\n", t);
            return 1;
        }

        // D. Check Constraints
        execution_check_constraints(&port, &config);

        // E. Render
        ui_render_frame(&port, universe, &config, t);

        // F. Input Check (Non-blocking)
        if (kbhit_esc()) {
            reset_terminal_mode();
            printf("\n\n   >> SIMULATION ABORTED BY USER.\n");
            return 0;
        }

        // G. Game Over Check
        if (port.status == STATUS_INSOLVENT) {
            reset_terminal_mode();
            printf("\n\n   >> TERMINAL FAILURE: INSOLVENCY.\n");
            return 0;
        }
        
        // Wait
        usleep(UI_TICK_DELAY_MS * 1000);
    }

    reset_terminal_mode();
    printf("\n\n   >> SIMULATION COMPLETE.\n");
    return 0;
}