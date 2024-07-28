/*
 * Copyright (c) 2022, The Linux Foundation. All rights reserved.
 * Copyright (C) 2022 ZTE, Inc.
 */

#ifndef _MI_SDE_ENCODER_H_
#define _MI_SDE_ENCODER_H_

#include "../sde/sde_encoder_phys.h"
#include "../dsi/dsi_display.h"
#define MAX_VSYNC_COUNT                   200
#define SYSFS_FOLDER_NAME "disp_vsync"

struct hw_vsync_info {
	u32 config_fps;
	ktime_t timestamp;
	u64 real_vsync_period_ns;
};

struct calc_hw_vsync {
	struct hw_vsync_info vsyc_info[MAX_VSYNC_COUNT];
	int vsync_count;
	ktime_t last_timestamp;
	u64 measured_vsync_period_ns;
	u64 measured_fps_x1000;
};

void zte_sde_encoder_save_vsync_info(struct sde_encoder_phys *phys_enc);
#endif /* _MI_SDE_ENCODER_H_ */