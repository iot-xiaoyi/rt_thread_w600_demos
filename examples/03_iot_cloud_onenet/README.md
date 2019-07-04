# 中国移动 OneNET 云平台接入例程

本例程演示如何使用 RT-Thread 提供的 onenet 软件包接入中国移动物联网开放平台，介绍如何通过 MQTT 协议接入 OneNET 平台，初次使用 OneNET 平台的用户请先阅读[《OneNET 用户手册》](https://www.rt-thread.org/document/site/submodules/onenet/docs/introduction/)。

## 简介

OneNET 平台是中国移动基于物联网产业打造的生态平台，具有高并发可用、多协议接入、丰富 API 支持、数据安全存储、快速应用孵化等特点。OneNET 平台可以适配各种网络环境和协议类型，现在支持的协议有LWM2M（NB-IOT）、EDP、MQTT、HTTP、MODBUS、JT\T808、TCP透传、RGMP等。用户可以根据不同的应用场景选择不同的接入协议。

onenet 软件包是 RT-Thread 针对 OneNET 平台做的适配，通过这个软件包可以让设备在 RT-Thread 上非常方便的连接 OneNet 平台，完成数据的发送、接收、设备的注册和控制等功能。

## 硬件说明

## 准备工作

### 创建设备

在使用本例程前需要在 OneNET 平台注册账号，并在帐号里创建产品，具体的流程参考[《OneNET 示例说明》](https://www.rt-thread.org/document/site/submodules/onenet/docs/samples/)。产品创建完成后，记录下产品概况页面的产品 ID 和 APIKey。

![设备信息](../../docs/figures/03_iot_cloud_onenet/device_info1.png)

切换到设备管理界面，记录下设备注册码。

![环境注册码](../../docs/figures/03_iot_cloud_onenet/device_info2.png)

打开 `/examples/03_iot_cloud_onenet/rtconfig.h`，找到 `ONENET_REGISTRATION_CODE`，`ONENET_INFO_PROID`，`ONENET_MASTER_APIKEY` 这三个宏定义，将原来的内容替换成刚刚记录下来的环境注册码，产品 ID 和 APIKey，然后保存文件。

### 代码移植

设备注册，设备上线，需要用到一些需要移植的接口函数，下面将具体介绍需要用到的四个接口函数。IoT Board关于 OneNET 的移植代码位于 `/examples/03_iot_cloud_onenet/applications/main.c` 文件中。

#### 保存设备信息

注册设备成功后，OneNET 平台会返回设备 ID 和 api key，用户需要将这两个信息保存起来，以便在下次开机时能读取这两个信息用于登录 OneNET 平台。除了保存 2 个设备信息外，用户还应该保存个已经注册成功的标志位，用来在开机时判断设备是否已经注册。

```c
rt_err_t onenet_port_save_device_info(char *dev_id, char *api_key)
{
    EfErrCode err=EF_NO_ERR;

    /* 保存设备 ID */
    err = ef_set_and_save_env("dev_id",dev_id);
    if(err != EF_NO_ERR)
    {
        rt_kprintf("save device info(dev_id : %s) failed!\n", dev_id);
        return -RT_ERROR;
    }

    /* 保存设备 api_key */
    err = ef_set_and_save_env("api_key", api_key);
    if(err != EF_NO_ERR)
    {
        rt_kprintf("save device info(api_key : %s) failed!\n", api_key);
        return -RT_ERROR;
    }

    /* 保存环境变量：已经注册 */
    err = ef_set_and_save_env("already_register","1");
    if(err != EF_NO_ERR)
    {
        rt_kprintf("save already_register failed!\n");
        return -RT_ERROR;
    }

    return RT_EOK;
}
```

#### 获取注册设备信息

设备注册需要提供设备名字和鉴权信息，这里取 设备的 的 mac 地址用作设备名字和鉴权信息，除了将 mac 地址赋值给 2 个入参外，还应该作为鉴权信息保存起来，用于下次开机登录 OneNET 平台。

```c
rt_err_t onenet_port_get_register_info(char *dev_name, char *auth_info)
{
    rt_uint32_t mac_addr[2] = {0};
    EfErrCode err = EF_NO_ERR;

    /* get mac addr */
    if (rt_wlan_get_mac((rt_uint8_t *)mac_addr) != RT_EOK)
    {
        rt_kprintf("get mac addr err!! exit\n");
        return -RT_ERROR;
    }

    /* set device name and auth_info */
    rt_snprintf(dev_name, ONENET_INFO_AUTH_LEN, "%d%d", mac_addr[0], mac_addr[1]);
    rt_snprintf(auth_info, ONENET_INFO_AUTH_LEN, "%d%d", mac_addr[0], mac_addr[1]);

    /* save device auth_info */
    err = ef_set_and_save_env("auth_info", auth_info);
    if (err != EF_NO_ERR)
    {
        rt_kprintf("save auth_info failed!\n");
        return -RT_ERROR;
    }

    return RT_EOK;
}
```

#### 获取设备信息

获取用于登录的设备信息功能的代码如下所示：

```c
rt_err_t onenet_port_get_device_info(char *dev_id, char *api_key, char *auth_info)
{
    char *info =RT_NULL;

    /* 获取设备 ID */
    info = ef_get_env("dev_id");
    if( info == RT_NULL)
    {
        rt_kprintf("read dev_id failed!\n");
        return -RT_ERROR;
    }
    else
    {
        rt_snprintf(dev_id,ONENET_INFO_AUTH_LEN,"%s",info);
    }

    /* 获取 api_key */
    info = ef_get_env("api_key");
    if( info == RT_NULL)
    {
        rt_kprintf("read api_key failed!\n");
        return -RT_ERROR;
    }else
    {
        rt_snprintf(api_key,ONENET_INFO_AUTH_LEN,"%s",info);
    }

    /* 获取设备鉴权信息 */
    info = ef_get_env("auth_info");
    if( info == RT_NULL)
    {
        rt_kprintf("read auth_info failed!\n");
        return -RT_ERROR;
    }else
    {
        rt_snprintf(auth_info,ONENET_INFO_AUTH_LEN,"%s",info);
    }

    return RT_EOK;
}
```

#### 查询设备注册状态

检查设备是否已经注册的代码如下所示：

```c
rt_bool_t onenet_port_is_registed(void)
{
    char *already_register = RT_NULL;

    /* 检查设备是否已经注册 */
    already_register = ef_get_env("already_register");
    if (already_register == RT_NULL)
    {
        return RT_FALSE;
    }

    return already_register[0] == '1' ? RT_TRUE : RT_FALSE;
}
```

## 软件说明

本例程主要实现了每隔 5s 往 OneNET 平台上传一次环境光强度，并可以执行 OneNET 下发命令的功能，程序代码位于 `/examples/03_iot_cloud_onenet/applications/main.c` 文件中。

在 main 函数中，主要完成了以下几个任务：

- 初始化 LED 管脚
- 注册 OneNET 启动函数为 WiFi 连接成功的回调函数

当 WiFi 连接成功后，会调用 `/examples/03_iot_cloud_onenet/packages/onenet-1.0.0/src/onenet_mqtt.c` 文件中的 onenet_mqtt_init() 函数，该函数会获取设备信息完成设备上线的任务。如果设备未注册，会自动调用注册函数完成注册。

OneNET 设备上线后，会自动执行 `/examples/03_iot_cloud_onenet/applications/main.c`中的onenet_upload_cycle() 函数，函数会设置命令响应的回调函数，并启动一个 onenet_send 的线程，线程会每隔 5 秒随机得到一个环境光强度数据，并将这个数据上传到 OneNET 的 light 数据流中，发送 100 次后线程自动结束。上传数据的代码如下所示：

```c
static void onenet_upload_entry(void *parameter)
{
    int value = 0;
    int i = 0;

    srand((int)rt_tick_get());
    /* upload ambient light value to topic light*/
    for (i = 0; i < NUMBER_OF_UPLOADS; i++)
    {
        value = rand() % 100;

        if (onenet_mqtt_upload_digit("light", value) < 0)
        {
            rt_kprintf("upload has an error, stop uploading\n");
            break;
        }
        else
        {
            rt_kprintf("buffer : {\"light\":%d}\n", value);
        }

        rt_thread_mdelay(5 * 1000);
    }
}
```

命令响应回调函数里主要完成的是命令打印，命令匹配的任务。当匹配到 ledon 和 ledoff 这两个命令时，会执行打开 led 和关闭 led 的动作，并向云端发送响应。具体代码如下所示：

```c
static void onenet_cmd_rsp_cb(uint8_t *recv_data, size_t recv_size, uint8_t **resp_data, size_t *resp_size)
{
    char res_buf[20] = { 0 };

    rt_kprintf("recv data is %.s\n", recv_size, recv_data);

    /* 命令匹配 */
    if (rt_strncmp(recv_data, "ledon", 5) == 0)
    {
        /* led on */
        rt_pin_write(LED_PIN, PIN_LOW);

        rt_snprintf(res_buf, sizeof(res_buf), "led is on");

        rt_kprintf("led is on\n");
    }
    else if (rt_strncmp(recv_data, "ledoff", 6) == 0)
    {
        /* led off */
        rt_pin_write(LED_PIN, PIN_HIGH);

        rt_snprintf(res_buf, sizeof(res_buf), "led is off");

        rt_kprintf("led is off\n");
    }

    /* 开发者必须使用 ONENET_MALLOC 为响应数据申请内存 */
    *resp_data = (uint8_t *) ONENET_MALLOC(strlen(res_buf) + 1);

    strncpy(*resp_data, res_buf, strlen(res_buf));

    *resp_size = strlen(res_buf);
}
```

## 运行

### 编译&下载

- **MDK**：双击 `project.uvprojx` 打开 MDK5 工程，执行编译。
- **IAR**：双击 `project.eww` 打开 IAR 工程，执行编译。

编译完成后，将开发板的 ST-Link USB 口与 PC 机连接，然后将固件下载至开发板。

程序运行日志如下所示：

```text
  \ | /
- RT -     Thread Operating System
 / | \     4.0.0 build Feb 13 2019
 2006 - 2018 Copyright by rt-thread team
lwIP-2.0.2 initialized!
[5] I/SAL_SOC: Socket Abstraction Layer initialize success.
[75] I/WLAN.dev: wlan init success
[113] I/WLAN.lwip: eth device init ok name:w0
[118] I/WLAN.dev: wlan init success
[154] I/WLAN.lwip: eth device init ok name:w1
[D/FAL] (fal_flash_init:63) Flash device | nor_flash | addr: 0x00000000 | len: 0x00100000 | blk_size: 0x00001000 |initialized finish.
[I/FAL] ==================== FAL partition table ====================
[I/FAL] | name      | flash_dev |   offset   |    length  |
[I/FAL] -------------------------------------------------------------
[I/FAL] | app       | nor_flash | 0x00010000 | 0x00080000 |
[I/FAL] | download  | nor_flash | 0x00090000 | 0x00060000 |
[I/FAL] | fs_part   | nor_flash | 0x000f0000 | 0x0000b000 |
[I/FAL] | easyflash | nor_flash | 0x000fb000 | 0x00001000 |
[I/FAL] =============================================================
[I/FAL] RT-Thread Flash Abstraction Layer (V0.3.0) initialize success.
[Flash] EasyFlash V3.3.0 is initialize success.
[Flash] You can get the latest version on https://github.com/armink/EasyFlash .
msh />  
```

### 连接无线网络

程序运行后会进行 MSH 命令行，等待用户配置设备接入网络。使用 MSH 命令 `wifi join ssid password` 配置网络，如下所示：

```shell
msh />wifi join ssid password
join ssid:ssid
[I/WLAN.mgnt] wifi connect success ssid:ssid_test
.......
msh />[I/WLAN.lwip] Got IP address : 152.10.200.224
```

### 数据上传

在 WiFi 连接成功后，开发板会自动连接 OneNET 平台，如果是第一次连接云平台，会自动注册设备，如下所示：

```shell
msh />[D/ONENET] (response_register_handlers:266) response is {"errno":0,"data":{"device_id":"42940432","key":"GZz=nP9xGMENrsBFFPMDUUe66Q8="},"error":"succ"}    #注册返回的设备信息
[D/ONENET] (mqtt_connect_callback:87) Enter mqtt_connect_callback!
[D/MQTT] ipv4 address port: 6002
[D/MQTT] HOST = '183.230.40.39'
[I/ONENET] RT-Thread OneNET package(V1.0.0) initialize success.
[I/WLAN.lwip] Got IP address : 152.10.200.224
[I/MQTT] MQTT server connect success
[D/ONENET] (mqtt_online_callback:92) Enter mqtt_online_callback!
[D/ONENET] (onenet_upload_entry:82) buffer : {"light":20}    #开始上传数据
[D/ONENET] (onenet_upload_entry:82) buffer : {"light":28}
```

打开 OneNET 平台，在**设备列表**页面，选择**数据流展示**，点击展开 **light** 数据流，可以看到刚刚上传的数据信息。

 ![数据流](../../docs/figures/03_iot_cloud_onenet/data_strem.png)

### 命令控制

在**设备列表**界面，选择**下发命令**，点击蓝色的**下发命令**按纽，会弹出一个命令窗口，我们可以下发命令给开发板。例程支持 ledon 和 ledoff 两个命令，板子收到这两个命令后会打开和关闭开发板上的红色 led。示例效果如下所示：

![设备控制界面](../../docs/figures/03_iot_cloud_onenet/send_cmd.png)

![发送命令](../../docs/figures/03_iot_cloud_onenet/ledon.png)

```shell
[D/ONENET] (mqtt_callback:62) topic $creq/1798e855-c334-5f03-a81a-5d8f587a0bc9 receive a message
[D/ONENET] (mqtt_callback:64) message length is 5
[D/ONENET] (onenet_cmd_rsp_cb:212) recv data is ledon
[D/ONENET] (onenet_cmd_rsp_cb:248) led is on
```

## 注意事项

- 使用本例程前请先阅读[《OneNET 用户手册》](https://www.rt-thread.org/document/site/submodules/onenet/docs/introduction/)。
- 使用本例程前请先修改 `rtconfig.h` 里的三个 OneNET 平台宏定义(`ONENET_REGISTRATION_CODE`,`ONENET_INFO_PROID`,`ONENET_MASTER_APIKEY`)。

## 引用参考

- 《GPIO 设备应用笔记》: docs/AN0002-RT-Thread-通用 GPIO 设备应用笔记.pdf
- 《OneNET 用户手册》: docs/UM1003-RT-Thread-OneNET 用户手册.pdf
- 《RT-Thread 编程指南 》: docs/RT-Thread 编程指南.pdf