#include "../dsi/dsi_panel.h"

#define MAX_SLEEP_TIME 200

void zte_disp_handle_lp_event(struct dsi_panel *panel, int power_mode);

int zte_convert_backlevel_function(int level, u32 bl_max, u32 mode);