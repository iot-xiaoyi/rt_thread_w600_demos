/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2019-02-13     tyx          first implementation
 */
#include <rthw.h>
#include <rtthread.h>
#include <rtdevice.h>
#include <dfs_fs.h>
#include <fal.h>

#define LOG_TAG               "main"
#include <ulog.h>

#define FS_PARTITION_NAME ("fs_part")

int main(void)
{
    struct rt_device *flash_dev;

    /* 配置 wifi 工作模式 */
    rt_wlan_set_mode(RT_WLAN_DEVICE_STA_NAME, RT_WLAN_STATION);
    rt_wlan_set_mode(RT_WLAN_DEVICE_AP_NAME, RT_WLAN_AP);

    /* 初始化分区表 */
    fal_init();

    /* 在 fs_part 分区上创建一个 MTD 设备 */
    flash_dev = fal_mtd_nor_device_create(FS_PARTITION_NAME);
    if (flash_dev == NULL)
    {
        LOG_E("Can't create a mtd device on '%s' partition.\n", FS_PARTITION_NAME);
    }

    /* 挂载 LittleFS 文件系统 */
    if (dfs_mount(FS_PARTITION_NAME, "/", "lfs", 0, 0) == 0)
    {
        LOG_I("Filesystem initialized!\n");
    }
    else
    {
        /* 创建 LittleFS 文件系统 */
        dfs_mkfs("lfs", FS_PARTITION_NAME);
        /* 再次挂载 LittleFS 文件系统 */
        if (dfs_mount(FS_PARTITION_NAME, "/", "lfs", 0, 0) != 0)
        {
            LOG_E("Failed to initialize filesystem!");
        }
    }

    /* 打开 MicroPython 命令交互界面 */
    extern void mpy_main(const char *filename);
    mpy_main(NULL);

    LOG_D("MicroPython will reset by user");
    rt_hw_cpu_reset();
    return 0;
}
