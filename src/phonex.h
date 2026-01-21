/*
 * ======================================================================================
 * PHONEX SYSTEMS // CORE HEADER
 * CLASSIFICATION: INTERNAL ARCHITECTURE
 * LOCALIZATION:   INDIA (INR)
 * ======================================================================================
 */

#ifndef PHONEX_H
#define PHONEX_H

#include <stdint.h>
#include <stdbool.h>
#include <time.h>

/* --- SYSTEM CONSTANTS ----------------------------------------------------------- */

#define SYSTEM_NAME         "PHONEX<INDIA>"
#define SYSTEM_VERSION      "1.0.0-IN"
#define CURRENCY_SYMBOL     "INR"
#define CURRENCY_CODE       "INR"

#define CURRENCY_SCALE      1000000 
#define MAX_ASSETS          16      
#define MAX_TICKS           360
#define UI_TICK_DELAY_MS    250     

/* --- CORE TYPES ----------------------------------------------------------------- */

typedef int64_t currency_t; 
typedef int64_t quantity_t;
typedef double  rate_t;

/* --- ENUMERATIONS --------------------------------------------------------------- */

typedef enum {
    REGIME_STABLE_GROWTH,
    REGIME_STAGFLATION,
    REGIME_LIQUIDITY_CRUNCH,
    REGIME_GLOBAL_SHOCK,
    REGIME_CUSTOM
} MarketRegime;

typedef enum {
    CLASS_CASH_INR,
    CLASS_NIFTY_EQ,
    CLASS_GOVT_BOND,
    CLASS_CORP_DEBT,
    CLASS_GOLD
} AssetClass;

typedef enum {
    STATUS_ACTIVE,
    STATUS_WARNING,
    STATUS_MARGIN_CALL,
    STATUS_LIQUIDATED,
    STATUS_INSOLVENT
} AccountStatus;

/* --- DATA STRUCTURES ------------------------------------------------------------ */

typedef struct {
    char ticker[12];
    char name[32];
    AssetClass type;
    
    currency_t price;
    currency_t prev_price;
    
    rate_t volatility;
    rate_t correlation_beta;
    bool is_illiquid;
} Asset;

typedef struct {
    int asset_index;            
    quantity_t units;           
    currency_t cost_basis;
    currency_t current_val;
    currency_t pnl_unrealized;  
} Position;

typedef struct {
    currency_t cash_balance;
    currency_t total_asset_value;   
    currency_t total_liabilities;   
    currency_t nav;                 

    Position positions[MAX_ASSETS];
    int position_count;

    currency_t high_water_mark;     
    rate_t current_drawdown;        
    rate_t leverage_ratio;          
    
    AccountStatus status;
    int months_underwater;          
} Portfolio;

typedef struct {
    MarketRegime regime;
    int duration_months;
    
    rate_t max_drawdown_limit;
    rate_t max_leverage;
    rate_t min_cash_buffer;
    
    bool auto_rebalance;            
    bool allow_margin;              
} SimConfig;

/* --- MACROS --------------------------------------------------------------------- */

#define TO_MICROS(x) ((currency_t)((x) * CURRENCY_SCALE))
#define FROM_MICROS(x) ((double)(x) / CURRENCY_SCALE)
#define TO_LAKHS(x) (FROM_MICROS(x) / 100000.0)
#define TO_CRORES(x) (FROM_MICROS(x) / 10000000.0)

/* --- FUNCTION PROTOTYPES -------------------------------------------------------- */

void phonex_init(void);
void phonex_teardown(void);

void market_init_universe(Asset *universe, MarketRegime regime);
void market_tick(Asset *universe, int count, MarketRegime regime, int tick);

void portfolio_init(Portfolio *p, currency_t initial_capital);
void portfolio_update_valuation(Portfolio *p, Asset *universe);
bool portfolio_audit(Portfolio *p); 

void execution_check_constraints(Portfolio *p, SimConfig *cfg);
void execution_force_liquidate(Portfolio *p, Asset *universe);

void ui_render_login(void);
void ui_render_frame(Portfolio *p, Asset *universe, SimConfig *cfg, int tick);
void ui_get_config(SimConfig *cfg);

#endif // PHONEX_H
