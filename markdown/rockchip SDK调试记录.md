# rockchip SDK调试记录

<!-- TOC -->

- [rockchip SDK调试记录](#rockchip-sdk调试记录)
  - [编译环境配置](#编译环境配置)
    - [SDK版本](#sdk版本)
  - [编译](#编译)
    - [U-Boot编译](#u-boot编译)
      - [U-Boot配置说明](#u-boot配置说明)
    - [Kernel编译](#kernel编译)
      - [Kernel配置说明](#kernel配置说明)
    - [Rootfs编译](#rootfs编译)
      - [Rootfs配置说明](#rootfs配置说明)
    - [全自动编译](#全自动编译)
    - [固件打包](#固件打包)
  - [U-Boot终端下tftp使用说明](#u-boot终端下tftp使用说明)
    - [U-Boot配置以太网](#u-boot配置以太网)
    - [U-Boot的tftp下载说明](#u-boot的tftp下载说明)
    - [eMMC通过tftp烧录loader的方法](#emmc通过tftp烧录loader的方法)
    - [tftp直接烧录到指定分区命令](#tftp直接烧录到指定分区命令)
  - [Windows下USB uart调试及烧录](#windows下usb-uart调试及烧录)
    - [安装USB驱动](#安装usb驱动)
    - [Maskrom烧写模式](#maskrom烧写模式)
    - [串口调试](#串口调试)
  - [Stacktrace](#stacktrace)
    - [使用方式](#使用方式)
  - [uboot调试记录](#uboot调试记录)
    - [dts](#dts)
  - [kernel调试记录](#kernel调试记录)
    - [内核配置修改记录](#内核配置修改记录)
    - [dts](#dts-1)
  - [rootfs调试记录](#rootfs调试记录)
    - [CPU](#cpu)
    - [DDR](#ddr)
    - [USB](#usb)
    - [PWM](#pwm)
  - [问题记录](#问题记录)

<!-- /TOC -->

## 编译环境配置

```text

切换板级配置命令：
在根目录执行命令： ./build.sh lunch

重点介绍下lunch
lunch可选择一个mk配置文件，文件目录在device\rockchip\rv1126_rv1109下;
目录下还包含了parameter配置文件，用于指定分区分配;
mk文件有RK_KERNEL_DTS RK_KERNEL_DEFCONFIG RK_PARAMETER RK_CFG_BUILDROOT等重要配置参数

查看编译命令
在根目录执行命令：./build.sh -h|help

查看部分模块详细编译命令
./build.sh -h kernel

cd kernel
make ARCH=arm rv1126_defconfig
make ARCH=arm rv1126-evb-ddr3-v10.img -j12

```

### SDK版本

SDK版本号记录:
rv1126_rv1109_linux_v2.2.2_20210923

## 编译

### U-Boot编译

```text

查看U-Boot详细编译命令
./build.sh -h uboot

U-Boot编译命令
./build.sh uboot

```

#### U-Boot配置说明

```text

使用menuconfig配置U-Boot，选择需要的模块，最后保存退出。
rv1126_defconfig 文件在目录 u-boot/configs
命令格式：make "RK_UBOOT_DEFCONFIG"_defconfig
RK_UBOOT_DEFCONFIG 定义在./build.sh选择的BoardConfig*.mk

cd u-boot
make rv1126_defconfig
make menuconfig

保存配置到对应的文件rv1126_defconfig
make savedefconfig
cp defconfig configs/rv1126_defconfig

```

### Kernel编译

```text

查看Kernel详细编译命令
./build.sh -h kernel

Kernel编译命令
./build.sh kernel

```

#### Kernel配置说明

```text

例如 device/rockchip/rv1126_rv1109/BoardConfig.mk
./build.sh device/rockchip/rv1126_rv1109/BoardConfig.mk
cd kernel

命令格式：make ARCH=arm "RK_KERNEL_DEFCONFIG" "RK_KERNEL_DEFCONFIG_FRAGMENT"
RK_KERNEL_DEFCONFIG 和RK_KERNEL_DEFCONFIG_FRAGMENT 都定义在./build.sh选择的BoardConfig*.mk
RK_KERNEL_DEFCONFIG_FRAGMENT 是可选项，具体看BoardConfig*.mk配置  

make ARCH=arm rv1126_defconfig
make ARCH=arm menuconfig

make ARCH=arm savedefconfig
cp defconfig arch/arm/configs/rv1126_defconfig

```

### Rootfs编译

```text

查看Rootfs详细编译命令
./build.sh -h rootfs

Rootfs编译和打包命令
./build.sh rootfs

```

#### Rootfs配置说明

```text

1. 先在SDK根目录查看Board Config对应的rootfs是哪个配置
./build.sh -h rootfs

Current SDK Default [ rootfs ] Build Command#
source envsetup.sh rockchip_rv1126_rv1109
make

2. source buildroot对应的defconfig
source envsetup.sh rockchip_rv1126_rv1109

3. 使用menuconfig配置文件系统，选择需要的模块，最后保存退出。

例如：要配置app/ipc-daemon这个工程

a. 找到app/ipc-daemon对应的配置文件
grep -lr "app/ipc-daemon" buildroot/package
buildroot/package/rockchip/ipc-daemon/ipc-daemon.mk
对应的配置文件：buildroot/package/rockchip/ipc-daemon/Config.in

b. 获取配置名称BR2_PACKAGE_IPC_DAEMON
查看buildroot/package/rockchip/ipc-daemon/Config.in


进入menuconfig后，按/"进入查找模式，输入BR2_PACKAGE_IPC_DAEMON
make menuconfig

4. 保存到rootfs配置文件
./buildroot/configs/rockchip_rv1126_rv1109_defconfig
make savedefconfig

```

### 全自动编译

```text

./build.sh all
1. 只编译模块代码（u-Boot，kernel，Rootfs，Recovery）
2. 需要再执行./mkfirmware.sh 进行固件打包

./build.sh 
在./build.sh all基础上
1. 增加固件打包 ./mkfirmware.sh
2. update.img打包
3. 复制rockdev目录下的固件到IMAGE/***_RELEASE_TEST/IMAGES目录
4. 保存各个模块的补丁到IMAGE/***_RELEASE_TEST/PATCHES目录
注：./build.sh 和 ./build.sh allsave 命令一样

```

### 固件打包

固件打包命令： ./mkfirmware.sh  
固件目录：rockdev

## U-Boot终端下tftp使用说明

### U-Boot配置以太网

```text

默认U-Boot代码支持的以太网phy是EVB板RTL8211F。U-Boot初始化以太网时，会先读取内核dtb的gmac
节点，如果没有获取到内核dtb，会使用U-Boot下的dtb初始化以太网phy。 所以如果板子上的phy不是
RTL8211F，则需要同步修改U-Boot的dtb的gmac节点

```

### U-Boot的tftp下载说明

使用`sysmem_search`获取一块指定大小的内存地址，然后设置IP地址，最用使用tftp命令下载文件

```text

Hit key to stop autoboot('CTRL+C'): 0
=> <INTERRUPT>
=> <INTERRUPT>
=>
=> sysmem_search
sysmem_search - Search a available sysmem region
Usage:
sysmem_search <size in hex>
=> sysmem_search 0x6400000
Sysmem: Available region at address: 0x356f6cc0
=> setenv ipaddr 172.16.21.47
=> setenv serverip 172.16.21.199
=> tftp 0x356f6cc0 uboot.img
ethernet@ffc40000 Waiting for PHY auto negotiation to complete. done
Using ethernet@ffc40000 device
TFTP from server 172.16.21.199; our IP address is 172.16.21.47
Filename 'uboot.img'.
Load address: 0x356f6cc0
Loading: 
> #################################################################
> #################################################################
> ######################
139.6 KiB/s
done
Bytes transferred = 2228224 (220000 hex)
=>

```

### eMMC通过tftp烧录loader的方法

![20210928142213](https://cdn.jsdelivr.net/gh/cairufan/image@main//picture/20210928142213.png)

### tftp直接烧录到指定分区命令

```text

=> tftpflash 0x20000000 uboot.img uboot
=> tftpflash 0x20000000 misc.img misc
=> tftpflash 0x20000000 rootfs.img rootfs
=> tftpflash 0x20000000 boot.img boot
=> tftpflash 0x20000000 recovery.img recovery
=> tftpflash 0x20000000 oem.img oem
=> tftpflash 0x20000000 userdata.img userdata

Usage:
tftpflash [loadAddress] [[hostIPaddr:]bootfilename] [partition]

每个分区升级成功会打印如下信息

=> tftpflash 0x20000000 uboot.img uboot
ethernet@ffc40000 Waiting for PHY auto negotiation to complete. done
Using ethernet@ffc40000 device
TFTP from server 192.168.11.26; our IP address is 192.168.11.254
Filename 'uboot.img'.
Load address: 0x20000000
Loading: 
> #################################################################
> #################################################################
> #################################################################
> #################################################################
> ##########################
479.5 KiB/s
done
Bytes transferred = 4194304 (400000 hex)
> ## TFTP flash uboot.img to partititon 'uboot' size 0x400000 ... OK

```

## Windows下USB uart调试及烧录

### 安装USB驱动

```text

烧录和调试前都需要安装USB驱动 驱动详见:

<SDK>/tools/windows/DriverAssitant_v4.91.zip

```

### Maskrom烧写模式

```text

SDK 提供 Windows 烧写工具(工具版本需要 V2.78 或以上)，工具位于工程根目录：

tools/
├── windows/RKDevTool

注意： Windows PC需要在管理员权限运行工具才可执行

基本原理是在系统上电前将FLASH_D0对地短路，使Flash引导失败，从而进入Maskrom状态。
适用于烧写了错误的bootloader文件，无法正常引导系统开机的情况下。

具体步骤如下：
1、 连接USB到电脑PC端，并按住开发板的update按键不放；
2、 给EVB供电12V，并打开船型开关；要是已经处于上电情况下，请按下复位按键。
3、 等待会儿开发工具将显示“发现一个Maskrom设备”，需要注意的是在Maskrom状态下需要
同时选择对应的Loader才能升级。
4、 开发工具选择对应的image文件。
5、 点击执行，即进入升级状态，在工具的右侧有进度显示栏，显示下载与校验情况


```

![20210928154142](https://cdn.jsdelivr.net/gh/cairufan/image@main//picture/20210928154142.png)

### 串口调试

- 连接EVB板的USB Debug到电脑PC端，在PC端设备管理器中得到当前断开的COM号

- 配置串口，波特率选择1500000(不是115200)，流控RTS/CTS不需勾选

## Stacktrace

- 原生的U-Boot不支持调用栈回溯机制，RK平台增加了该功能。目前一共有3种方式触发调用栈打印

- 系统崩溃时自动触发

- 用户主动调用 dump_stack()

- 使能 CONFIG_ROCKCHIP_DEBUGGER

### 使用方式

例如系统abort:

```text

"Synchronous Abort" handler, esr 0x96000010
// abort的原因、pc、lr、sp
* Reason: Exception from a Data abort, from current exception level
* PC = 000000000028f430
* LR = 00000000002608d0
* SP = 00000000f3dceb30
...
// 重点突出PC和LR
Call trace:
PC: [< 0028f430 >]
LR: [< 002608d0 >]
// 函数调用关系
Stack:
[< 0028f430 >]
[< 0028da24 >]
[< 00211600 >]
[< 002117b0 >]
[< 00202910 >]
[< 00202aa8 >]
[< 0027698c >]
[< 002151ec >]
[< 00201b2c >]
// 指导用户转换上述调用栈信息
Copy info from "Call trace..." to a file(eg. dump.txt), and run
command in your U-Boot project: ./scripts/stacktrace.sh dump.txt

```

用户根据上述说明，把调用栈信息复制到任意txt文件（比如dump.txt）后执行如下命令

```text

cjh@Ubuntu:~/u-boot$ ./scripts/stacktrace.sh dump.txt
// 符号表来源
SYMBOL File: ./u-boot.sym
// 重点列出PC和LR对应的代码位置
Call trace:
PC: [< 0028f430 >] strncpy+0xc/0x20 ./lib/string.c:98
LR: [< 002608d0 >] on_serialno+0x10/0x1c ./drivers/usb/gadget/g_dnl.c:217
// 转换后得到真实函数名
Stack:
[< 0028f430 >] strncpy+0xc/0x20
[< 0028da24 >] hdelete_r+0xcc/0xf0
[< 00211600 >] _do_env_set.isra.0+0x70/0x1b8
[< 002117b0 >] env_set+0x3c/0x58
[< 00202910 >] rockchip_set_serialno+0x54/0x140
[< 00202aa8 >] board_late_init+0x5c/0xa0
[< 0027698c >] initcall_run_list+0x58/0x94
[< 002151ec >] board_init_r+0x20/0x24
[< 00201b2c >] relocation_return+0x4/0x0

```

注意事项:

- 执行该命令时，当前机器上的固件必须和当前代码环境匹配才有意义！否则会得到错误
的转换

- 转换命令有三种，具体用哪种请根据调用栈打印之后的指导说明

```text

./scripts/stacktrace.sh ./dump.txt // 解析来自U-Boot的调用栈信息
./scripts/stacktrace.sh ./dump.txt tpl // 解析来自tpl的调用栈信息
./scripts/stacktrace.sh ./dump.txt spl // 解析来自spl的调用栈信息

```

## uboot调试记录

### dts

- 现有dts文件为u-boot/arch/arm/dts/rv1126-evb.dts 里面有对应链接的dtsi文件
  
- 原生的U-Boot只支持使用U-Boot自己的DTB，RK平台增加了kernel DTB机制的支持，即使用kernel DTB去初始化外设。主要目的是为了兼容外设板级差异，如：power、clock、display 等

## kernel调试记录

### 内核配置修改记录

- 打开CONFIG_IKCONFIG及CONFIG_IKCONFIG_PROC 在/proc下自动生成.config压缩文件

- 打开CONFIG_NFS_FS 其他nfs选项默认会打开 启用nfs挂载功能

### dts

- ./bulid.sh info 可查看现有kernel下的dts的配置文件

- 现有dts文件为kernel/arch/arm/boot/dts/rv1126-evb-ddr3-v13.dts 里面有对应链接的dtsi文件

## rootfs调试记录

### CPU

- 设置CPU频率  
`echo 1512000 > /sys/devices/system/cpu/cpufreq/policy0/scaling_setspeed`

- 关闭CPU变频功能  
`echo userspace > /sys/devices/system/cpu/cpufreq/policy0/scaling_governor`

- 查看当前CPU频率  
`cat /sys/devices/system/cpu/cpufreq/policy0/scaling_cur_freq`

### DDR

- 查看带宽利用率  
`cat load`

### USB

- 查看 USB 总线上的所有 USB 设备信息  
`/sys/kernel/debug/usb/devices`

- /sys/bus/usb/drivers/usb 路径下的每个 'usb*' 节点，分别对应一个 USB Host 控制器，比如要重新初始化 USB Host 控制器 'usb2'，则执行如下命令：  
`echo usb2 > /sys/bus/usb/drivers/usb/unbind`  
`echo usb2 > /sys/bus/usb/drivers/usb/bind`

### PWM

- 以下是 pwmchip0 的例子，设置 pwm0 输出频率 100K，占空比 50%, 极性为正极性
  
```txt

cd /sys/class/pwm/pwmchip0/
echo 0 > export
cd pwm0
echo 10000 > period
echo 5000 > duty_cycle
echo normal > polarity
echo 1 > enable

```

## 问题记录

1. 如何构建一个通用的kernel 后续的机型通过区分FDT或者UBOOT的方式实现不同外设的变化需求 包括外设上电时序和pinmux等主要需求

2. 只保留fdt uboot kernel rootfs这几个主要分区 其他分区全部干掉，或者把pre-uboot等分区合并到uboot分区里去

3. rootfs的构建，能否使用busybox生成一个squashfs的rootfs，如何操作

4. kernel DTB的目录和使用方式