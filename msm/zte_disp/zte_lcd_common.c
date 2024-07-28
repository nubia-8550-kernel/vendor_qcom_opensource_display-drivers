#include "zte_lcd_common.h"
#include "../sde/sde_encoder.h"
#include "../dsi/dsi_display.h"

struct dsi_panel *zte_panel;
struct device *dsi_uevent_device;

LCD_PROC_FILE_DEFINE(zte_lcd_info, ZTE_LCD_INFO_CTRL)
LCD_PROC_FILE_DEFINE(zte_lcd_hbm, ZTE_LCD_HBM_CTRL)
LCD_PROC_FILE_DEFINE(zte_lcd_hdr, ZTE_LCD_HDR_CTRL)
LCD_PROC_FILE_DEFINE(zte_lcd_aod_bl, ZTE_LCD_AOD_BL)
LCD_PROC_FILE_DEFINE(zte_lcd_color_gamut, ZTE_LCD_COLOR_GAMUT_CTRL)
LCD_PROC_FILE_DEFINE(zte_lcd_acl, ZTE_LCD_ACL_CTRL)
LCD_PROC_FILE_DEFINE(zte_lcd_cur_fps, ZTE_LCD_FPS_CTRL)
LCD_PROC_FILE_DEFINE(zte_panel_state, ZTE_LCD_STATE_CTRL)
LCD_PROC_FILE_DEFINE(zte_lcd_od, ZTE_LCD_OD)
LCD_PROC_FILE_DEFINE(zte_lcd_lspot, ZTE_LCD_LSPOT)
LCD_PROC_FILE_DEFINE(zte_lcd_bl_limit, ZTE_LCD_BL_LIMIT)
LCD_PROC_FILE_DEFINE(zte_lcd_second_irq_fps, ZTE_LCD_SECOND_IRQ_FPS)
LCD_PROC_FILE_DEFINE(zte_lcd_spr, ZTE_LCD_SPR_CTRL)

void zte_hdr_ctrl_display(struct dsi_panel *panel, u32 setHdr)
{
	u32 bl_lvl_bak = panel->bl_config.bl_level;
	char cal_value = panel->disp_feature->zte_lcd_hdr;

	if (panel->disp_feature->hbm_config->hdr_sunlight_bl_ctrl) {
		switch (setHdr) {
		case 0:
			cal_value &= 0xf0;/* clear low4 bit(0) */
			panel->disp_feature->hbm_config->hdr_video_enable = false;
			break;
		case 1:
			cal_value |= 0x01;/* set low4 bit */
			panel->disp_feature->hbm_config->hdr_video_enable = true;
			break;
		case 2:
		case 4:
		case 6: //enter sunlight mode
			cal_value &= 0x0f;
			cal_value |= setHdr << 4;/* set high4 bit */
			panel->disp_feature->hbm_config->hdr_sunlight_lvl = setHdr;
			break;
		case 3: //exit sunlight mode
			cal_value &= 0x0f;/* clear high4 bit*/
			panel->disp_feature->hbm_config->hdr_sunlight_lvl = 0;
			break;
		default:
			pr_err("[MSM_LCD] HDR invalid flags: %d\n", setHdr);
		break;
		}
	} else {
		switch (setHdr) {
		case 0:
			cal_value &= 0xfe;/* clear bit(0) */
			break;
		case 1:
			cal_value |= 0x01;/* set bit(0) */
			break;
		case 2:
		case 4:
		case 6: //enter sunlight mode
			cal_value |= 0x02;/* set bit(1) */
			break;
		case 3:
			cal_value &= 0xfd;/* clear bit(1) */
			break;
		default:
			pr_err("[MSM_LCD] HDR invalid flag: %d\n", setHdr);
		break;
		}
	}
	if (panel->disp_feature->zte_lcd_hdr != cal_value) {
		panel->disp_feature->zte_lcd_hdr = cal_value; //0x00,0x01,0x20,0x21,0x30,0x31,0x40,0x41,0x60,0x61
		if (bl_lvl_bak != 0)
			dsi_panel_set_backlight(panel, bl_lvl_bak);
		pr_info("[MSM_LCD] HDR flag: new value:0x%x\n", panel->disp_feature->zte_lcd_hdr);
	} else {
		pr_info("[MSM_LCD] HDR flag is same as old: %d, do nothing\n", panel->disp_feature->zte_lcd_hdr);
	}
}

void zte_panel_fps_send_uevent(int fps)
{
	char *envp[3];
	if (fps == 60)
		envp[0] = "LCD_FPS=60";
	else if (fps == 90)
		envp[0] = "LCD_FPS=90";
	else if (fps == 120)
		envp[0] = "LCD_FPS=120";
	else if (fps == 144)
		envp[0] = "LCD_FPS=144";

	envp[1] = NULL;
	envp[2] = NULL;

	if (dsi_uevent_device){
		kobject_uevent_env(&dsi_uevent_device->kobj, KOBJ_CHANGE, envp);
		pr_info("[MSM_LCD]FPS: send fps %d\n",fps);
	} else
		pr_info("[MSM_LCD]FPS: uevent_device is NULL, LCD state send faild\n");
}

void zte_panel_irq_fps_send_ltpo_uevent(int fps)
{
	char *envp[3];
	if (fps == 60)
		envp[0] = "LCD_LTPO_FPS=60";
	else if (fps == 90)
		envp[0] = "LCD_LTPO_FPS=90";
	else if (fps == 120)
		envp[0] = "LCD_LTPO_FPS=120";
	else if (fps == 144)
		envp[0] = "LCD_LTPO_FPS=144";
	else if (fps == 1)
		envp[0] = "LCD_LTPO_FPS=1";
	else if (fps == 10)
		envp[0] = "LCD_LTPO_FPS=10";
	else if (fps == 20)
		envp[0] = "LCD_LTPO_FPS=20";
	else if (fps == 30)
		envp[0] = "LCD_LTPO_FPS=30";
	else if (fps == 40)
		envp[0] = "LCD_LTPO_FPS=40";

	envp[1] = NULL;
	envp[2] = NULL;

	if (dsi_uevent_device){
		kobject_uevent_env(&dsi_uevent_device->kobj, KOBJ_CHANGE, envp);
		pr_info("[MSM_LCD]irq_fps: send ltpo fps %d\n",fps);
	} else
		pr_info("[MSM_LCD]irq_fps: ltpo uevent_device is NULL, LCD state send faild\n");
}

void zte_panel_hbm_send_uevent(int mode, int ret)
{
	char *envp[3];

	if (mode == LCD_STATUS_HBM_OFF_EVENT)
		envp[0] = "HBM_STATUS=OFF";
	else if (mode == LCD_STATUS_HBM_ON_EVENT)
		envp[0] = "HBM_STATUS=ON";
	else if (mode == LCD_STATUS_HBM_FG_RELEASE_EVENT)
		envp[0] = "HBM_STATUS=FG_RELEASE";
	else if (mode == LCD_STATUS_HBM_FG_PRESS_EVENT)
		envp[0] = "HBM_STATUS=FG_PRESS";
	else
		envp[0] = "HBM_STATUS=ERR";

	if (ret == 0)
		envp[1] = "HBM_SET_RESULT=SUCCESSFUL";
	else
		envp[1] = "HBM_SET_RESULT=FAILED";

	envp[2] = NULL;

	if (dsi_uevent_device) {
            kobject_uevent_env(&dsi_uevent_device->kobj, KOBJ_CHANGE, envp);
            pr_info("[MSM_LCD]HBM: uevent = %s , %s\n", envp[0], envp[1]);
        }
	else
		pr_info("[MSM_LCD]HBM: dsi_uevent_device is NULL\n");
}

int zte_hbm_ctrl_display(struct dsi_panel *panel, u32 setHbm, bool from_node){
    int err = 0;
    struct dsi_display_mode_priv_info *priv_info;
	struct dsi_cmd_desc *cmds = NULL;
    struct dsi_cmd_desc *aod_cmds = NULL;
	enum dsi_cmd_set_type type, aod_type;
	u8 *tx_buf;
    u8 *aod_tx_buf;
	u32 count, aod_count;

    if (panel->cur_mode)
		priv_info = panel->cur_mode->priv_info;
	else
		priv_info = NULL;

    if (setHbm == 0) {
        if (panel->disp_feature->hbm_config->global_hbm) {
            if (panel->in_aod)
               type = DSI_CMD_SET_ZTE_HBM_OFF_AOD_ON;
            else
               type = DSI_CMD_SET_ZTE_HBM_OFF;
        } else {
            if (panel->in_aod)
               type = DSI_CMD_SET_ZTE_FOD_OFF_AOD_ON;
            else
               type = DSI_CMD_SET_ZTE_FOD_OFF;
        }

        if (type == DSI_CMD_SET_ZTE_HBM_OFF_AOD_ON)
            atomic_set(&panel->skip_nolp, 0);

        if (type == DSI_CMD_SET_ZTE_HBM_OFF) {
            if (panel->saved_backlight != 0x0 && priv_info){
                pr_info("[MSM_LCD] set HBM Brightness : %d\n",panel->saved_backlight);
                count = priv_info->cmd_sets[type].count;
				cmds = priv_info->cmd_sets[type].cmds;
                if (cmds && count >= 1) {
                    tx_buf = (u8 *)cmds[count-1].msg.tx_buf;
                    if (tx_buf && tx_buf[0] == 0x51) {
                        tx_buf[1] = panel->saved_backlight >> 8;
                        tx_buf[2] = panel->saved_backlight & 0xff;
                        pr_info("[MSM_LCD] HBM DSI_CMD_SET_ZTE_HBM_OFF 0x%02X = 0x%02X 0x%02X\n",tx_buf[0], tx_buf[1], tx_buf[2]);
                    }
                }
            }
        } else if(type == DSI_CMD_SET_ZTE_HBM_OFF_AOD_ON){
            count = priv_info->cmd_sets[type].count;
			cmds = priv_info->cmd_sets[type].cmds;
            if (cmds && count >= 1) {
                tx_buf = (u8 *)cmds[count-1].msg.tx_buf;
                if (tx_buf && tx_buf[0] == 0x51) {
                    aod_type = DSI_CMD_SET_ZTE_AOD_LOW;
                    if (panel->disp_feature->zte_lcd_aod_bl == AOD_BL_LEVEL_MID) {
                       aod_type = DSI_CMD_SET_ZTE_AOD_MID;
                    } else if (panel->disp_feature->zte_lcd_aod_bl == AOD_BL_LEVEL_HIGH) {
                       aod_type = DSI_CMD_SET_ZTE_AOD_HIGH;
                    }
                    aod_count = priv_info->cmd_sets[aod_type].count;
                    aod_cmds = priv_info->cmd_sets[aod_type].cmds;
                    aod_tx_buf = (u8 *)aod_cmds[aod_count-1].msg.tx_buf;
                    tx_buf[1] = aod_tx_buf[1];
                    tx_buf[2] = aod_tx_buf[2];
                }
            }
        }
        err = zte_dsi_panel_tx_cmd_set(panel, type);
        if (err < 0) {
            pr_err("[MSM_LCD] HBM OFF: failed to send %d cmd, rc=%d\n", type, err);
            return err;
        } else {
            pr_info("[MSM_LCD] HBM OFF: success to send %d cmd\n ", type);
        }

    } else {
        if (panel->disp_feature->hbm_config->global_hbm) {
            if (panel->in_aod)
               type = DSI_CMD_SET_ZTE_AOD_OFF_HBM_ON;
            else
               type = DSI_CMD_SET_ZTE_HBM_ON;
        } else {
            if (panel->in_aod)
               type = DSI_CMD_SET_ZTE_AOD_OFF_FOD_ON;
            else
               type = DSI_CMD_SET_ZTE_FOD_ON;
        }

        if (type == DSI_CMD_SET_ZTE_AOD_OFF_HBM_ON)
            atomic_set(&panel->skip_nolp, 1);

        err = zte_dsi_panel_tx_cmd_set(panel, type);
        if (err < 0) {
			pr_err(" MSM_LCD HBM ON: failed to send %d cmd, rc=%d\n", type, err);
            return err;
		} else {
			pr_info("MSM_LCD HBM ON: success to send %d cmd.", type);
		}
    }
    return err;
}

void zte_lcd_gamespace_bl_limit(struct dsi_panel *panel, u32 bl_limit)
{
	u32 bl_bak_store;
	//u32 bl_bak = panel->bl_config.bl_level;
	u32 bl_bak;
	struct sde_connector *sde_conn;
	struct dsi_display *nubia_display = get_main_display();

    if (!nubia_display || !nubia_display->panel) {
		pr_err("msm_lcd Invalid display panel\n");
		return;
	}

	sde_conn = to_sde_connector(nubia_display->drm_conn);
	if(sde_conn && sde_conn->bl_device) {
		//backlight_update_status(sde_conn->bl_device);
		bl_bak = sde_conn->bl_device->props.brightness;
	} else {
		pr_err("msm_lcd Invalid sde_connector sde_conn\n");
		return;
    }

	if (bl_limit <= 0 || bl_limit > panel->bl_config.brightness_max_level) {
		panel->disp_feature->zte_lcd_bl_limit = panel->bl_config.brightness_max_level;
		pr_info("[MSM_LCD] gamespace_bl_limit value=%d error, do nothing\n", bl_limit);
		return;
	} else if (panel->zte_lcd_thermal_max_brightness > 0 && bl_limit > panel->zte_lcd_thermal_max_brightness) {
		panel->disp_feature->zte_lcd_bl_limit = bl_limit;
		pr_info("[MSM_LCD] gamespace_bl_limit=%d bigger thermal_max_brightness=%d do nothing\n", bl_limit, panel->zte_lcd_thermal_max_brightness);
		return;
	}

	if (panel->disp_feature->zte_lcd_bl_limit != bl_limit) {
		panel->disp_feature->zte_lcd_bl_limit = bl_limit;
		if (bl_bak > 0) {
			if (bl_bak > bl_limit) {
				//pr_info("[MSM_LCD] scale=%d,%d bl_bak %d to gamespace_bl_limit:%d\n", panel->bl_config.bl_scale, panel->bl_config.bl_scale_sv,
                //     bl_bak, panel->disp_feature->zte_lcd_bl_limit);
				bl_bak = bl_limit;
				bl_bak_store = mult_frac(bl_bak, panel->bl_config.bl_max_level, panel->bl_config.brightness_max_level);
                //scale backlight begin
                bl_bak_store = bl_bak_store * panel->bl_config.bl_scale / MAX_BL_SCALE_LEVEL;
                bl_bak_store = (u32)bl_bak_store * panel->bl_config.bl_scale_sv / MAX_SV_BL_SCALE_LEVEL;
                if (bl_bak_store > panel->bl_config.bl_max_level)
                    bl_bak_store = panel->bl_config.bl_max_level;
                if (bl_bak_store && (bl_bak_store < panel->bl_config.bl_min_level))
                    bl_bak_store = panel->bl_config.bl_min_level;
                //scale backlight end

				dsi_panel_set_backlight(panel, bl_bak_store);
                //dsi_display_set_backlight  //mutex_lock error
			} else {
				pr_info("[MSM_LCD] gamespace_bl_limit bl_scale_sv=%d,bl_bak:%d<=bl_limit:%d, do nothing\n", panel->bl_config.bl_scale_sv, bl_bak, bl_limit);
			}
		}
	} else {
		pr_info("[MSM_LCD] gamespace_bl_limit is same as old: %d, do nothing\n", bl_limit);
	}

	return;
}

extern void zte_dsi_display_change_fps_irq_status(struct dsi_panel *panel, bool enable);
void zte_panel_sencond_irq_fps_ctrl(struct dsi_panel *panel, u32 irq_fps_enable)
{
	u16 zte_get_fps;

	if (panel->disp_feature->zte_lcd_second_irq_fps == irq_fps_enable) {
		return;
	}

	panel->disp_feature->zte_lcd_second_irq_fps = irq_fps_enable;
	if (irq_fps_enable == 1){
		zte_dsi_display_change_fps_irq_status(panel, irq_fps_enable);
		pr_info("[MSM_LCD]irq_fps: enable\n");
	} else {
		zte_dsi_display_change_fps_irq_status(panel, irq_fps_enable);
		zte_get_fps = panel->disp_feature->zte_get_irq_fps / 10;
		if (zte_get_fps <= 5)
			zte_get_fps = 1;
		else if (zte_get_fps <= 15)
			zte_get_fps = 10;
		else if (zte_get_fps <= 25)
			zte_get_fps = 20;
		else if (zte_get_fps <= 35)
			zte_get_fps = 30;
		else if (zte_get_fps <= 45)
			zte_get_fps = 40;
		else if (zte_get_fps <= 65)
			zte_get_fps = 60;
		else if (zte_get_fps <= 95)
			zte_get_fps = 90;
		else if (zte_get_fps <= 125)
			zte_get_fps = 120;
		pr_info("[MSM_LCD]irq_fps: disable read value=%d,fps=%d\n", panel->disp_feature->zte_get_irq_fps, zte_get_fps);
		zte_panel_irq_fps_send_ltpo_uevent(zte_get_fps);
	}
}

extern int zte_wait_for_spr(void);
int zte_set_disp_parameter(u32 feature, u32 feature_mode, bool from_node){
    int err = 0;
	struct dsi_panel *panel;
	struct dsi_display_mode_priv_info *priv_info;
	struct dsi_cmd_desc *cmds = NULL;
	u8 *tx_buf;
	u32 count;

    if (!zte_panel) {
		pr_info("[MSM_LCD] No panel device\n");
		return -EINVAL;
	}

    panel = zte_panel;

    if (panel->cur_mode)
		priv_info = panel->cur_mode->priv_info;
	else
		priv_info = NULL;

    mutex_lock(&panel->panel_lock);

    if (!panel->panel_initialized) {
		err = -EPERM;
		pr_err("[MSM_LCD] panel is off or not initialized feature=%d feature_mode=%d\n", feature, feature_mode);
		if (feature == ZTE_LCD_AOD_BL && feature_mode < AOD_BL_LEVEL_OFF) {
			panel->disp_feature->zte_lcd_aod_bl = feature_mode;
			pr_info("[MSM_LCD] save aod bl =%d\n", panel->disp_feature->zte_lcd_aod_bl);
		}
		goto error;
	}

    pr_info("[MSM_LCD] Send cmds [feature,feature_mode] = [%s,%d]\n", feature_name[feature], feature_mode);

    switch (feature) {
        case ZTE_LCD_HBM_CTRL:
			err = zte_hbm_ctrl_display(panel,feature_mode,from_node);
			if(from_node) {
				panel->disp_feature->zte_lcd_hbm = feature_mode;
            	pr_info("[MSM_LCD] HBM MODE is %d\n", panel->disp_feature->zte_lcd_hbm);
			}
            break;
        case ZTE_LCD_HDR_CTRL:
            zte_hdr_ctrl_display(panel,feature_mode);
            break;
        case ZTE_LCD_COLOR_GAMUT_CTRL:
            if (panel->disp_feature->zte_lcd_color_gamut != feature_mode) {
                switch(feature_mode) {
                    case COLOR_GAMUT_ORIGINAL:
                        err = zte_dsi_panel_tx_cmd_set(panel, DSI_CMD_SET_ZTE_COLOR_ORIGINAL);
                        break;
                    case COLOR_GAMUT_SRGB:
                        err = zte_dsi_panel_tx_cmd_set(panel, DSI_CMD_SET_ZTE_COLOR_SRGB);
                        break;
                    case COLOR_GAMUT_P3:
                        err = zte_dsi_panel_tx_cmd_set(panel, DSI_CMD_SET_ZTE_COLOR_P3);
                        break;
                    default:
                        pr_err("[MSM_LCD] Color gamut index %d not supported\n", feature_mode);
                        break;
                }
                panel->disp_feature->zte_lcd_color_gamut = feature_mode;
                pr_info("[MSM_LCD] COLOR GAMUT is %d\n", panel->disp_feature->zte_lcd_color_gamut);
            } else {
                pr_err("[MSM_LCD] COLOR GAMUT Repeat settings!");
            }
            break;
		case ZTE_LCD_ACL_CTRL:
            if (panel->disp_feature->zte_lcd_acl != feature_mode) {
                switch(feature_mode){
                    case LCD_ACL_OFF:
                        err = zte_dsi_panel_tx_cmd_set(panel, DSI_CMD_SET_ZTE_ACL_OFF);
                        break;
                    case LCD_ACL_LOW:
                        err = zte_dsi_panel_tx_cmd_set(panel, DSI_CMD_SET_ZTE_ACL_LOW);
                        break;
                    case LCD_ACL_MID:
                        err = zte_dsi_panel_tx_cmd_set(panel, DSI_CMD_SET_ZTE_ACL_MID);
                        break;
                    case LCD_ACL_HIGH:
                        err = zte_dsi_panel_tx_cmd_set(panel, DSI_CMD_SET_ZTE_ACL_HIGH);
                        break;
                    default:
                        pr_err("[MSM_LCD] ACL %d not supported\n", feature_mode);
                        break;
                }
                panel->disp_feature->zte_lcd_acl = feature_mode;
                pr_info("[MSM_LCD] ACL level is %d\n", panel->disp_feature->zte_lcd_acl);
            } else {
                pr_err("[MSM_LCD] ACL Repeat settings!");
            }
            break;
        case ZTE_LCD_AOD_BL:
            if (feature_mode < AOD_BL_LEVEL_OFF)
            	panel->disp_feature->zte_lcd_aod_bl = feature_mode;

            if (panel->is_hbm_enabled || panel->disp_feature->zte_lcd_hbm) {
               pr_info("[MSM_LCD] skip setting aod bl when panel in hbm!");
               break;
            }

            if (!panel->in_aod) {
               pr_info("[MSM_LCD] skip setting aod bl when panel not in aod!");
               break;
            }

            switch(feature_mode) {
                case AOD_BL_LEVEL_LOW:
                    err = zte_dsi_panel_tx_cmd_set(panel, DSI_CMD_SET_ZTE_AOD_LOW);
                    break;
                case AOD_BL_LEVEL_MID:
                    err = zte_dsi_panel_tx_cmd_set(panel, DSI_CMD_SET_ZTE_AOD_MID);
                    break;
                case AOD_BL_LEVEL_HIGH:
                    err = zte_dsi_panel_tx_cmd_set(panel, DSI_CMD_SET_ZTE_AOD_HIGH);
                    break;
                default:
                    pr_info("[MSM_LCD] AOD: please check AOD Brightness level\n");
                    break;
            }
            pr_info("[MSM_LCD] AOD Brightness is %d\n", panel->disp_feature->zte_lcd_aod_bl);
            break;
		case ZTE_LCD_RECOVERY_BL:
			if (priv_info && feature_mode != 0) {
				if (panel->disp_feature->zte_lcd_hbm == 1 || panel->is_hbm_enabled){
					if (panel->disp_feature->hbm_config->global_hbm == 1){
						panel->saved_backlight = feature_mode;
                        pr_info("[MSM_LCD] skip restore Brightness when hbm enabled!");
                        break;
					}
				}
				pr_info("[MSM_LCD] restore Brightness : %d\n",feature_mode);

                count = priv_info->cmd_sets[DSI_CMD_SET_ZTE_RESTORE_BL].count;
				cmds = priv_info->cmd_sets[DSI_CMD_SET_ZTE_RESTORE_BL].cmds;
                if (cmds && count >= 1) {
                    tx_buf = (u8 *)cmds[count-1].msg.tx_buf;
                    if (tx_buf && tx_buf[0] == 0x51) {
                        tx_buf[1] = feature_mode >> 8;
                        tx_buf[2] = feature_mode & 0xff;
                        pr_info("[MSM_LCD] DSI_CMD_SET_ZTE_RESTORE_BL 0x%02X = 0x%02X 0x%02X\n",tx_buf[0], tx_buf[1], tx_buf[2]);
                    }
                }
                err = zte_dsi_panel_tx_cmd_set(panel, DSI_CMD_SET_ZTE_RESTORE_BL);
            }
			break;
		case ZTE_LCD_AOD_OFF_HBM_ON:
			err = zte_dsi_panel_tx_cmd_set(panel, DSI_CMD_SET_ZTE_AOD_OFF_HBM_ON);
			pr_info("[MSM_LCD] set DSI_CMD_SET_ZTE_AOD_OFF_HBM_ON\n");
			break;
        case ZTE_LCD_LSPOT:
            if (feature_mode != panel->disp_feature->zte_lcd_lspot) {
                panel->disp_feature->zte_lcd_lspot = feature_mode;
                if (feature_mode)
                    err = zte_dsi_panel_tx_cmd_set(panel, DSI_CMD_SET_ZTE_FOD_ON);
                else
                    err = zte_dsi_panel_tx_cmd_set(panel, DSI_CMD_SET_ZTE_FOD_OFF);
            } else {
                pr_info("[MSM_LCD] lspot setting is Repeated and don't work\n");
            }
			break;
        case ZTE_LCD_OD:
            if (feature_mode != panel->disp_feature->zte_lcd_od) { 
                panel->disp_feature->zte_lcd_od = feature_mode;
                if (feature_mode)
                   err = zte_dsi_panel_tx_cmd_set(panel, DSI_CMD_SET_ZTE_OD_ON);
                else
                   err = zte_dsi_panel_tx_cmd_set(panel, DSI_CMD_SET_ZTE_OD_OFF);
            } else {
                pr_info("[MSM_LCD] od setting is Repeated and don't work\n");
            }
			break;
        case ZTE_DIM_ON:
            err = zte_dsi_panel_tx_cmd_set(panel, DSI_CMD_SET_ZTE_DIM_ON);
            break;
        case ZTE_LCD_BL_LIMIT:
            zte_lcd_gamespace_bl_limit(panel,feature_mode);
            break;
		case ZTE_LCD_SECOND_IRQ_FPS:
			zte_panel_sencond_irq_fps_ctrl(panel,feature_mode);
			break;
        case ZTE_LCD_SPR_CTRL:
            if (from_node) {
                if (feature_mode != panel->disp_feature->zte_lcd_spr) { 
                    panel->disp_feature->zte_lcd_spr = feature_mode;
                } else {
                    pr_info("[MSM_LCD] SPR setting is Repeated and don't work\n");
                }
            } else {
                if (feature_mode)
                   err = zte_dsi_panel_tx_cmd_set(panel, DSI_CMD_SET_ZTE_SPR_ON);
                else
                   err = zte_dsi_panel_tx_cmd_set(panel, DSI_CMD_SET_ZTE_SPR_OFF);
            }
            break;
        default:
            pr_info("[MSM_LCD] Unsupported index=%d\n", feature_mode);
            break;
	}
	mutex_unlock(&panel->panel_lock);

    if(feature == ZTE_LCD_HBM_CTRL){
		zte_panel_hbm_send_uevent(feature_mode, err);
        if(!panel->disp_feature->hbm_config->global_hbm)
				zte_panel_hbm_send_uevent(feature_mode + LOCAL_HBM_INDEX, err);
    }

    if (err < 0) {
        pr_err("MSM_LCD Feature %d mode %d tx send fail\n", feature,feature_mode);
    }
	return err;
error:
	mutex_unlock(&panel->panel_lock);
	return err;
}

static int zte_disp_backlight_config(struct dsi_panel *panel)
{
	int rc = 0;
    const char *curve_type = NULL;
	struct dsi_parser_utils *utils = &panel->utils;

    panel->bl_config.curve_enabled = utils->read_bool(utils->data, "zte,curve_enabled");
    if (panel->bl_config.curve_enabled) {
        curve_type = utils->get_property(utils->data, "zte,lcm_backlight_curve_mode", NULL);
        if (!strcmp(curve_type, "lcd_brightness_max_350_lux")) {
           panel->bl_config.curve_mode = CURVE_MATRIX_MAX_350_LUX;
        } else if (!strcmp(curve_type, "lcd_brightness_max_400_lux")) {
           panel->bl_config.curve_mode = CURVE_MATRIX_MAX_400_LUX;
        } else if (!strcmp(curve_type, "lcd_brightness_max_450_lux")) {
           panel->bl_config.curve_mode = CURVE_MATRIX_MAX_450_LUX;
        } else {
           panel->bl_config.curve_mode = CURVE_MATRIX_MAX_450_LUX;
        }
    }

    panel->bl_config.zte_convert_brightness = zte_convert_backlevel_function;
    pr_info("MSM_LCD finish backlight config! curve = %d\n",
                                    panel->bl_config.curve_mode);
	return rc;
}

static int zte_disp_hbm_config(struct dsi_panel *panel)
{
	int rc = 0;
    u32 val = 0;
	struct dsi_parser_utils *utils = &panel->utils;

    panel->disp_feature->hbm_config = kzalloc(sizeof(struct disp_hbm_config), GFP_KERNEL);

    panel->disp_feature->hbm_config->global_hbm = utils->read_bool(utils->data, "zte,hbm_enabled");

	panel->disp_feature->hbm_config->hdr_sunlight_bl_ctrl = utils->read_bool(utils->data, "zte,hdr_sunlight_bl_ctrl");

    rc = utils->read_u32(utils->data, "zte,hdr_brightness", &val);
	if (!rc) {
		panel->disp_feature->hbm_config->hdr_brightness = val;
	}

    rc = utils->read_u32(utils->data, "zte,hdr_threshold", &val);
	if (!rc) {
		panel->disp_feature->hbm_config->hdr_threshold = val;
	}

    pr_info("MSM_LCD finish hbm config! global_hbm = %d, hdr_brightness = %d, hdr_threshold = %d\n",
                                    panel->disp_feature->hbm_config->global_hbm,
                                    panel->disp_feature->hbm_config->hdr_brightness,
                                    panel->disp_feature->hbm_config->hdr_threshold);
	return rc;
}

void zte_disp_feature_check(struct dsi_panel *panel) {
    bool feature_enabled;
	struct dsi_parser_utils *utils = &panel->utils;

    zte_lcd_info_init();
    panel->disp_feature->zte_lcd_info = panel->name;
    pr_info("MSM_LCD panel name = %s\n", panel->disp_feature->zte_lcd_info);

    zte_panel_state_init();

    feature_enabled = utils->read_bool(utils->data, "zte,hdr_enabled");
    if (feature_enabled) {
        zte_lcd_hdr_init();
        panel->disp_feature->zte_lcd_hdr = 0;
        pr_info("MSM_LCD create HDR node\n");
    }

    feature_enabled = utils->read_bool(utils->data, "zte,color_space_enabled");
    if (feature_enabled) {
        zte_lcd_color_gamut_init();
        panel->disp_feature->zte_lcd_color_gamut = COLOR_GAMUT_P3;
        pr_info("MSM_LCD create COLOR node\n");
    }

    feature_enabled = utils->read_bool(utils->data, "zte,fps_enabled");
    if (feature_enabled) {
        zte_lcd_cur_fps_init();
        panel->disp_feature->zte_lcd_cur_fps = 60;
        pr_info("MSM_LCD create FPS node\n");
    }

    feature_enabled = utils->read_bool(utils->data, "zte,acl_enabled");
    if (feature_enabled) {
        zte_lcd_acl_init();
        panel->disp_feature->zte_lcd_acl = 0;
        pr_info("MSM_LCD create ACL node\n");
    }

    feature_enabled = utils->read_bool(utils->data, "zte,aod_enabled");
    if (feature_enabled) {
        zte_lcd_aod_bl_init();
        panel->disp_feature->zte_lcd_aod_bl = AOD_BL_LEVEL_MID;
        pr_info("MSM_LCD create AOD node\n");
    }

    feature_enabled = utils->read_bool(utils->data, "zte,hbm_enabled");
    if (feature_enabled) {
        zte_lcd_hbm_init();
        panel->disp_feature->zte_lcd_hbm = 0;
        pr_info("MSM_LCD create HBM node\n");
        zte_disp_hbm_config(panel);
    }

    feature_enabled = utils->read_bool(utils->data, "zte,od_enabled");
    if (feature_enabled) {
        zte_lcd_od_init();
        panel->disp_feature->zte_lcd_od = 0;
        pr_info("MSM_LCD create OD node\n");
    }

    feature_enabled = utils->read_bool(utils->data, "zte,lspot_enabled");
    if (feature_enabled) {
        zte_lcd_lspot_init();
        panel->disp_feature->zte_lcd_lspot = 0;
        pr_info("MSM_LCD create LSPOT node\n");
    }

    feature_enabled = utils->read_bool(utils->data, "zte,zlog_enabled");
    if (feature_enabled) {
        zte_lcd_lspot_init();
        panel->disp_feature->zte_lcd_lspot = 0;
        pr_info("MSM_LCD create LSPOT node\n");
    }

    feature_enabled = utils->read_bool(utils->data, "zte,fps_need_blank");
    if (feature_enabled) {
        panel->disp_feature->zte_lcd_fps_need_blank = true;
        pr_info("MSM_LCD fps change in blank\n");
    }

    feature_enabled = utils->read_bool(utils->data, "zte,bl_limit_enabled");
    if (feature_enabled) {
        zte_lcd_bl_limit_init();
		panel->disp_feature->zte_lcd_bl_limit = panel->bl_config.brightness_max_level;
		panel->disp_feature->zte_lcd_bl_limit_feature_enable = true;
		pr_info("MSM_LCD create bl_limit node\n");
    }

    feature_enabled = utils->read_bool(utils->data, "zte,second_irq_fps_enabled");
    if (feature_enabled) {
        zte_lcd_second_irq_fps_init();
		panel->disp_feature->zte_lcd_fps_irq_gpio_feature_enable = true;
		pr_info("MSM_LCD create irq_fps gpio_irq node\n");
		zte_dsi_display_register_fps_irq(panel);
    }
    feature_enabled = utils->read_bool(utils->data, "zte,spr_enabled");
    if (feature_enabled) {
        zte_lcd_spr_init();
		panel->disp_feature->zte_lcd_spr = 0;
		pr_info("MSM_LCD create lcd_spr node\n");
    }
}

void zte_disp_config(struct dsi_panel *panel) {
    int rc = 0;
    bool feature_enabled;
    struct dsi_parser_utils *utils = &panel->utils;

    rc = utils->read_u32(utils->data, "zte,aod-exit-delay-time", &panel->aod_exit_delay_time);
	if (rc)
		panel->aod_exit_delay_time = 0;

	pr_info("MSM_LCD aod exit delay %d\n", panel->aod_exit_delay_time);

    feature_enabled = utils->read_bool(utils->data, "zte,hbm_need_delay");
    if (feature_enabled) {
        panel->hbm_need_delay = true;
    } else {
        panel->hbm_need_delay = false;
    }
    pr_info("MSM_LCD hbm need delay = %d\n", panel->hbm_need_delay);
}

static void dimming_enable_delayed_work(struct work_struct *work)
{
	zte_set_disp_parameter(ZTE_DIM_ON, 1, false);
}

static void report_fod_in_aod_work(struct work_struct *work)
{
	zte_panel->fod_layer = true;
	zte_panel_hbm_send_uevent(LCD_STATUS_HBM_FG_PRESS_EVENT,0);
}

extern void zte_lcd_reg_debug_func(void);
#ifdef CONFIG_ZTE_LCD_ZLOG
extern void zte_zlog_lcd_client_init(struct dsi_panel *panel);
#endif
extern int zte_disp_vsync_create(void);
void zte_disp_common_func(struct dsi_panel *panel) {

    if(!panel) {
        pr_info("MSM_LCD No panel device\n");
		return;
    }

    zte_panel = panel;

    zte_panel->disp_feature = kzalloc(sizeof(struct zte_disp_feature), GFP_KERNEL);

    if (!zte_panel->disp_feature) {
  		pr_err("%s: %d MSM_LCD kzalloc memory failed\n", __func__, __LINE__);
  		return;
  	}

    zte_disp_feature_check(zte_panel);

    zte_disp_backlight_config(zte_panel);

    zte_disp_config(zte_panel);

    zte_lcd_reg_debug_func();

    zte_disp_vsync_create();

#ifdef CONFIG_ZTE_LCD_ZLOG
    zte_zlog_lcd_client_init(panel);
#endif

    atomic_set(&panel->aod_entering, 0);
    atomic_set(&panel->aod_exiting, 0);
    atomic_set(&panel->skip_nolp, 0);

    INIT_DELAYED_WORK(&panel->dim_work, dimming_enable_delayed_work);
    INIT_DELAYED_WORK(&panel->report_fod_in_aod_work, report_fod_in_aod_work);
}