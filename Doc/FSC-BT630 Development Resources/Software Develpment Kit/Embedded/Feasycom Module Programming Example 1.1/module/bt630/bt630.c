
#include <stdint.h>

#include "config.h"
#include "debug.h"
#include "private.h"

#include "module.h"


#define PLAIN_PATTERN				""

static const bt_pattern_t bt_pattern_table[] = 
{
	{ FSC_BT_AT,		PLAIN_PATTERN},
	{ FSC_BT_NAME,		"+NAME"},
	{ FSC_BT_PIN, 		"+PIN"},
	{ FSC_BT_BAUD,		"+BAUD"},
	{ FSC_BT_LPM,		"+LPM"},
	{ FSC_BT_VER,		"+VER"},
	{ FSC_BT_ADDR,		"+ADDR"},

	{ FSC_BT_FLOWCTL,	"+FLOWCTL"},
	{ FSC_BT_REBOOT,	"+REBOOT"},
	{ FSC_BT_RESTORE,	"+RESTORE"},
	{ END_OF_COMMANDS,	PLAIN_PATTERN},
	
	{ FSC_RESP_OK,		"OK"},
	{ FSC_RESP_ERROR,	"ERROR"},
	{ END_OF_RESPONSES,	PLAIN_PATTERN},

	{ FSC_TP_INCOMING,	PLAIN_PATTERN},
	{ FSC_TP_OUTGOING,	PLAIN_PATTERN},
	{ END_OF_TP_PATTERNS,	PLAIN_PATTERN},

	{ END_OF_ALL_PATTERNS,	PLAIN_PATTERN},
};

static const module_t bt826_module =
{
	.init_time = 200,
	.dedicated_at_response_timeout = 100,
	.bt_pattern_table = bt_pattern_table,
};

const module_t* get_module(void)
{
	return &bt826_module;
}


