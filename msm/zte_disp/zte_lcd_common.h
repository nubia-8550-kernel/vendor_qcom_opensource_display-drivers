#ifndef _ZTE_LCD_COMMON_H_
#define _ZTE_LCD_COMMON_H_
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/of_gpio.h>
#include <linux/gpio.h>
#include <linux/delay.h>
#include <linux/time.h>
#include <linux/ctype.h>
#include <linux/debugfs.h>
#include <linux/sysfs.h>
#include <linux/proc_fs.h>
#include <linux/kobject.h>
#include <linux/mutex.h>

#include "dsi_panel.h"
#include "zte_disp_panel.h"

#define SKIP_CREATE_NODE "no need to create node"
/* ACL Status */
enum {
   LCD_ACL_OFF = 0,
   LCD_ACL_LOW,
   LCD_ACL_MID,
   LCD_ACL_HIGH,
   LCD_ACL_MAX
};

/* AOD Status */
enum {
    AOD_BL_LEVEL_LOW = 0,
    AOD_BL_LEVEL_MID,
    AOD_BL_LEVEL_HIGH,
    AOD_BL_LEVEL_OFF,
    AOD_BL_MAX
};

enum {	/* lcd curve mode */
	CURVE_MATRIX_MAX_350_LUX = 1,
	CURVE_MATRIX_MAX_400_LUX,
	CURVE_MATRIX_MAX_450_LUX,
	CURVE_MATRIX_MAX_RM692C9_LUX,
};

#define LOCAL_HBM_INDEX 2
enum {
	LCD_STATUS_HBM_OFF_EVENT = 0,
	LCD_STATUS_HBM_ON_EVENT,
	LCD_STATUS_HBM_FG_RELEASE_EVENT,
	LCD_STATUS_HBM_FG_PRESS_EVENT,
	LCD_STATUS_HBM_MAX
};

/* FPS Define */
#define ZTE_60FPS      60
#define ZTE_90FPS      90
#define ZTE_120FPS     120
#define ZTE_144FPS     144

enum {
	ZTE_LCD_INFO_CTRL = 0,
	ZTE_LCD_HBM_CTRL,
	ZTE_LCD_HDR_CTRL,
	ZTE_LCD_COLOR_GAMUT_CTRL,
	ZTE_LCD_FPS_CTRL,
	ZTE_LCD_ACL_CTRL,
	ZTE_LCD_AOD_BL,
	ZTE_LCD_LSPOT,
	ZTE_LCD_OD,
	ZTE_LCD_RECOVERY_BL,
	ZTE_LCD_AOD_OFF_HBM_ON,
	ZTE_DIM_ON,
	ZTE_LCD_STATE_CTRL,
	ZTE_LCD_BL_LIMIT,
	ZTE_LCD_SECOND_IRQ_FPS,
	ZTE_LCD_SPR_CTRL,
	ZTE_LCD_MAX_CTRL
};

static const char *zte_node_string[ZTE_LCD_MAX_CTRL] = {
	"driver/lcd_id",
	"driver/lcd_hbm",
	"driver/lcd_hdr",
	"driver/lcd_color_gamut",
	"driver/lcd_fps",
	"driver/lcd_acl",
	"driver/lcd_aod_bl",
	"driver/lcd_lspot",
	"driver/lcd_od",
	SKIP_CREATE_NODE,
	SKIP_CREATE_NODE,
	SKIP_CREATE_NODE,
	"driver/lcd_state",
	"driver/lcd_bl_limit",
	"driver/lcd_second_irq_fps",
	"driver/lcd_spr"
};

static const char *feature_name[ZTE_LCD_MAX_CTRL] = {
	"lcd_id",
	"lcd_hbm",
	"lcd_hdr",
	"lcd_color_gamut",
	"lcd_fps",
	"lcd_acl",
	"lcd_aod_bl",
	"lcd_lspot",
	"lcd_od",
	"bl_restore",
	"aodoff_hbmon",
	"dim_on",
	"lcd_state",
	"lcd_bl_limit",
	"lcd_second_irq_fps",
	"lcd_spr",
};

int zte_set_disp_parameter(u32 feature, u32 feature_mode, bool from_node);
void zte_panel_hbm_send_uevent(int mode, int ret);
void zte_disp_common_func(struct dsi_panel *panel);
void zte_panel_fps_send_uevent(int fps);

#define LCD_PROC_FILE_DEFINE(name, nodeid) \
static ssize_t name##_proc_write(struct file *file, \
		const char __user *buffer, size_t count, loff_t *ppos) \
{ \
	char *tmp = kzalloc((count+1), GFP_KERNEL); \
	u32 mode; \
	if (!tmp) \
		return -ENOMEM; \
	if (copy_from_user(tmp, buffer, count)) { \
		kfree(tmp); \
		return -EFAULT; \
	} \
    if (nodeid == ZTE_LCD_BL_LIMIT) { \
		sscanf(tmp, "%d", &mode); \
	} else { \
		mode = *tmp - '0'; \
	} \
	zte_set_disp_parameter(nodeid, mode, true); \
	kfree(tmp); \
	return count; \
} \
static int name##_show(struct seq_file *m, void *v) \
{ \
    if (nodeid == ZTE_LCD_INFO_CTRL) { \
		seq_printf(m, "panel_name=%s\n", zte_panel->disp_feature->name); \
		pr_info("MSM_LCD node name: %s\n", zte_panel->disp_feature->name); \
	} else { \
		seq_printf(m, "%d\n", zte_panel->disp_feature->name); \
		pr_info("MSM_LCD node value: %d\n", zte_panel->disp_feature->name); \
	} \
	return 0; \
} \
static int name##_proc_open(struct inode *inode, struct file *file) \
{ \
	return single_open(file, name##_show, NULL); \
} \
static const struct proc_ops name##_proc_fops = { \
  	.proc_open	= name##_proc_open, \
  	.proc_read	= seq_read, \
  	.proc_lseek	= seq_lseek, \
 	.proc_release	= single_release, \
	.proc_write	= name##_proc_write, \
}; \
static void name##_init() \
{ \
	proc_create(zte_node_string[nodeid], 0664, NULL, & name##_proc_fops); \
	return; \
}
#endif /* _ZTE_LCD_COMMON_H_ */