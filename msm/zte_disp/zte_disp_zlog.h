#include "../dsi/dsi_panel.h"

#ifdef CONFIG_ZTE_LCD_ZLOG
#include <../../../../drivers/vendor/common/zlog/zlog_common/zlog_common.h>
static struct zlog_mod_info zlog_lcd_dev = {
	.module_no = ZLOG_MODULE_LCD,
	.name = "LCD",
	.device_name = "qcom_lcd",
	.ic_name = "dummy_lcd",
	.module_name = "QCOM",
	.fops = NULL,
};
#endif