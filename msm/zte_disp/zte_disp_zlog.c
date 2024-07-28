#include "zte_disp_zlog.h"
#ifdef CONFIG_ZTE_LCD_ZLOG
void zte_zlog_lcd_client_init(struct dsi_panel *panel)
{
	struct zlog_client *zlog_lcd_client = NULL;

	if (panel->name) {
		zlog_lcd_dev.ic_name = panel->name;
	}

	pr_info("%s: MSM_LCD ic_name=%s\n", __func__, zlog_lcd_dev.ic_name);
	zlog_lcd_client = zlog_register_client(&zlog_lcd_dev);
	if (!zlog_lcd_client) {
		pr_err("%s: MSM_LCD zlog register client zlog_lcd_dev fail\n", __func__);
		return;
	}
    panel->disp_feature->zlog_lcd_client = zlog_lcd_client;
	pr_info("%s: MSM_LCD zlog register client zlog_lcd_dev successfully\n", __func__);
}
#endif