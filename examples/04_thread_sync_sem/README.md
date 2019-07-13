# LED 闪烁例程

## 简介

本例程作为 SDK 的第一个例程，也是最简单的例程，类似于程序员接触的第一个程序 Hello World 一样简洁。

它的主要功能是让板载的 RGB-LED 中的蓝色 LED 不间断闪烁。

## 硬件说明

![LED 连接单片机引脚](../../docs/figures/01_basic_led_blink/led_pcb.png)

![LED 电路原理图](../../docs/figures/01_basic_led_blink/led_pcb_1.png)

如上图所示，RBG-LED 属于共阳 LED ，**阴极** 通过拨码开关后，再连接到单片机的 21，22，23 号引脚上，其中蓝色 LED 对应 **23** 号引脚。拨动拨码开关，让单片机引脚与 RBG-LED 灯引脚相连。单片机引脚输出低电平即可点亮 LED ，输出高电平则会熄灭 LED。

LED 在开发板中的位置如下图所示：

![LED 位置](../../docs/figures/01_basic_led_blink/obj.png)

开发者需要手动将拨码开关的1号位置拨到上面，芯片的 23 号引脚才能跟 RGB-LED 的蓝色灯连接上。

## 软件说明

闪灯的源代码位于 `/examples/01_basic_led_blink/applications/main.c` 中。首先定义了一个宏 `LED_PIN` ，与 LED 蓝色引脚 `23` 号相对应

```c
/* using BLUE LED in RGB */
#define LED_PIN     (23)
```

在 main 函数中，将该引脚配置为输出模式，并在下面的 while 循环中，周期性（500毫秒）开关 LED，同时输出一些日志信息。

```c
int main(void)
{
    unsigned int count = 1;
    /* set LED pin mode to output */
    rt_pin_mode(LED_PIN, PIN_MODE_OUTPUT);

    while (count > 0)
    {
        /* led on */
        rt_pin_write(LED_PIN, PIN_LOW);
        rt_kprintf("led on, count: %d\n", count);
        rt_thread_mdelay(500);

        /* led off */
        rt_pin_write(LED_PIN, PIN_HIGH);
        rt_kprintf("led off\n");
        rt_thread_mdelay(500);

        count++;
    }

    return 0;
}
```

## 运行

### 编译&下载

- **MDK**：双击 `project.uvprojx` 打开 MDK5 工程，执行编译。
- **IAR**：双击 `project.eww` 打开 IAR 工程，执行编译。

编译完成后，将固件下载至开发板。

### 运行效果

按下复位按键重启开发板，观察开发板上 RBG-LED 的实际效果。正常运行后，蓝色 LED 会周期性闪烁，如下图所示：

![RGB 蓝灯亮起](../../docs/figures/01_basic_led_blink/blue.png)

此时也可以在 PC 端使用终端工具打开开发板的 `uart0` 串口，设置 115200 8 1 N 。开发板的运行日志信息即可实时输出出来。

```shell
led on, count: 1
led off
led on, count: 2
led off
led on, count: 3
led off
led on, count: 4
led off
led on, count: 5
led off
```

## 注意事项

开发板上的拨码开关分别连接 RGB-LED 和 排针 CN7，如果 RGB-LED 灯不亮，请检测相应灯的拨码开关是否置于 ON 位置。

## 引用参考

- 《通用GPIO设备应用笔记》: docs/AN0002-RT-Thread-通用 GPIO 设备应用笔记.pdf
- 《RT-Thread 编程指南》: docs/RT-Thread 编程指南.pdf
