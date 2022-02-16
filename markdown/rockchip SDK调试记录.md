# rockchip SDK调试记录

<!-- TOC -->

- [rockchip SDK调试记录](#rockchip-sdk调试记录)
  - [编译环境配置](#编译环境配置)
    - [SDK版本](#sdk版本)
    - [交叉工具链说明](#交叉工具链说明)
  - [编译](#编译)
    - [miniloader编译](#miniloader编译)
    - [U-Boot编译](#u-boot编译)
      - [U-Boot配置说明](#u-boot配置说明)
    - [Kernel编译](#kernel编译)
      - [Kernel配置说明](#kernel配置说明)
    - [备份系统编译](#备份系统编译)
    - [Rootfs编译](#rootfs编译)
      - [Rootfs配置说明](#rootfs配置说明)
    - [全自动编译](#全自动编译)
    - [固件打包](#固件打包)
    - [update.img拆包](#updateimg拆包)
  - [U-Boot终端下tftp使用说明](#u-boot终端下tftp使用说明)
    - [U-Boot配置以太网](#u-boot配置以太网)
    - [U-Boot的tftp下载说明](#u-boot的tftp下载说明)
    - [eMMC通过tftp烧录loader的方法](#emmc通过tftp烧录loader的方法)
    - [tftp直接烧录到指定分区命令](#tftp直接烧录到指定分区命令)
  - [Windows下USB uart调试及烧录](#windows下usb-uart调试及烧录)
    - [安装USB驱动](#安装usb驱动)
    - [关于瑞芯微开发工具的使用备注](#关于瑞芯微开发工具的使用备注)
    - [Maskrom烧写模式](#maskrom烧写模式)
    - [loader烧录模式](#loader烧录模式)
    - [串口调试](#串口调试)
  - [Stacktrace](#stacktrace)
    - [使用方式](#使用方式)
  - [uboot调试记录](#uboot调试记录)
    - [dts](#dts)
    - [启动流程](#启动流程)
  - [kernel调试记录](#kernel调试记录)
    - [内核配置修改记录](#内核配置修改记录)
    - [dts](#dts-1)
  - [rootfs调试记录](#rootfs调试记录)
    - [CPU](#cpu)
    - [DDR](#ddr)
    - [USB](#usb)
    - [PWM](#pwm)
    - [查看分区信息](#查看分区信息)
    - [分区写保护设置](#分区写保护设置)
  - [问题记录](#问题记录)

<!-- /TOC -->

## 编译环境配置

```text

切换板级配置命令：
在根目录执行命令： ./build.sh lunch 
默认选0

重点介绍下lunch
lunch可选择一个mk配置文件，文件目录在device\rockchip\rv1126_rv1109下;
目录下还包含了parameter配置文件，用于指定分区分配;
mk文件有RK_KERNEL_DTS RK_KERNEL_DEFCONFIG RK_PARAMETER RK_CFG_BUILDROOT等重要配置参数

paramter目录：device/rockchip/rv1126_rv1109 （parameter-buildroot-fit.txt）
uboot_config目录：u-boot/configs (rv1126_defconfig)
kernel_config目录：kernel/arch/arm/configs (rv1126_defconfig)
kernel_dts目录：kernel/arch/arm/boot/dts （rv1126-evb-ddr3-v13.dts）
uboot_dts目录：u-boot/arch/arm/dts （rv1126-evb.dts）

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

### 交叉工具链说明

SDK中需要用到2个交叉工具链

用于文件系统和上层应用:  
prebuilts/gcc/linux-x86/arm/gcc-arm-8.3-2019.03-x86_64-arm-linuxgnueabihf

用于UBOOT和内核以及驱动编译:  
prebuilts/gcc/linux-x86/arm/gcc-linaro-6.3.1-2017.05-x86_64_arm-linuxgnueabihf

## 编译

### miniloader编译

```text

1. 修改rkbin\tools\ddrbin_param.txt相关信息
2. cd rkbin/tools;./ddrbin_tool ddrbin_param.txt ../bin/rv11/rv1126_ddr_924MHz_v1.08.bin
3. 在根目录下执行./build.sh uboot 生成miniloader文件
4. ./build.sh firmware 连接miniloader文件到rockdev目录下

```

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

### 备份系统编译

```text

1. 选择备份系统的内核配置 (arch/arm/configs/rv1126_bksystem_defconfig)

执行 ./bulid.sh lunch
选择 BoardConfig-bksystem.mk

2. 编译内核 (即我们所需要的备份系统)

./bulid.sh kernel

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

2. source buildroot对应的defconfig
source envsetup.sh rockchip_rv1126_rv1109

3. cd到buildroot目录
cd buildroot

4. 使用menuconfig配置文件系统，选择需要的模块，最后保存退出。
make menuconfig

5. 保存到rootfs配置文件(buildroot/configs/rockchip_rv1126_rv1109_defconfig)
make savedefconfig

```

### 全自动编译

```text

./build.sh all
1. 只编译模块代码（u-Boot，kernel，Rootfs，Recovery）
2. 需要再执行./bulid.sh firmware进行固件打包

```

### 固件打包

```text

固件目录：rockdev
./bulid.sh firmware指令用来把生成镜像进行软链接到rockdev目录下
./bulid.sh updateimg用来生成总的大包（update.img）

```

### update.img拆包

```text

1. 可通过Linux环境下脚本工具解包update.img为分立的镜像文件

- 工具：tools/linux/Linux_Pack_Firmware/rockdev/unpack.sh
- 使用方法：将update.img放置于unpack.sh脚本同级目录，执行以下命令解包
./unpack.sh

2. 可使用Windows环境下瑞芯微开发工具解包update.img为分立的镜像文件

- 工具：tools/windows/RKDevTool/RKDevTool_Release/RKDevTool.exe
- 使用方法：选择 高级功能 选项卡，固件栏 ... 选择对应update.img，点击 解包 按钮，即可解包固件

```

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

### 关于瑞芯微开发工具的使用备注

- 选择固件或者镜像烧录后，若想替换芯的固件或者镜像，要重新打开工具，因为工具会缓存上次烧录过的文件，若位置不换的情况还是会烧录之前的固件镜像

- 高级功能中有些功能也需要进入loader模式，比如镜像拷贝，且镜像拷贝的偏移地址不能超过32M，基本没啥用，建议用tftp或者dd指令

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

如果是我们的板子，步骤1，2要改为在uboot下敲rbrom可以实现

```

![20210928154142](https://cdn.jsdelivr.net/gh/cairufan/image@main//picture/20210928154142.png)

### loader烧录模式

```text

SDK 提供 Windows 烧写工具(工具版本需要 V2.78 或以上)，工具位于工程根目录：

tools/
├── windows/RKDevTool

注意： Windows PC需要在管理员权限运行工具才可执行

1. uboot下输入download指令，进入loader烧录模式
2. PC工具上会显示发现一个loader设备，选择想对应的分区镜像
3. 点击执行进行烧录

```

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

### 启动流程

```text

start.s
// 汇编环境
=> IRQ/FIQ/lowlevel/vbar/errata/cp15/gic // ARM架构相关的lowlevel初始化
=> _main
    => stack // 准备好C环境需要的栈
    // 【第一阶段】C环境初始化，发起一系列的函数调用
    => board_init_f: init_sequence_f[]
        initf_malloc
        arch_cpu_init // 【SoC的lowlevel初始化】
        serial_init // 串口初始化
        dram_init // 【获取ddr容量信息】
        reserve_mmu // 从ddr末尾开始往低地址reserve内存
        reserve_video
        reserve_uboot
        reserve_malloc
        reserve_global_data
        reserve_fdt
        reserve_stacks
        dram_init_banksize
        sysmem_init
        setup_reloc // 确定U-Boot自身要reloc的地址
    // 汇编环境
    => relocate_code // 汇编实现U-Boot代码的relocation
    // 【第二阶段】C环境初始化，发起一系列的函数调用
    => board_init_r: init_sequence_r[]
        initr_caches // 使能MMU和I/Dcache
        initr_malloc
        bidram_initr
        sysmem_initr
        initr_of_live // 初始化of_live
        initr_dm // 初始化dm框架
        board_init // 【平台初始化，最核心部分】
            board_debug_uart_init // 串口iomux、clk配置
            init_kernel_dtb // 【切到kernel dtb】！
            clks_probe // 初始化系统频率
            regulators_enable_boot_on // 初始化系统电源
            io_domain_init // io-domain初始化
            set_armclk_rate // __weak，ARM提频(平台有需求才实现)

```

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
`rk-msch-probe -c rv1126`

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

### 查看分区信息

- fdisk -l

### 分区写保护设置

```txt

Linux Kernel下EMMC和SD CARD是块设备，NAND FLASH使用rknand 或者rkflash驱动时也是块设备，
可以通过下面命令配置分区的读写属性。

示例1. 设置system分区为只读

./busybox blockdev --setro /dev/block/by-name/system

示例2. 设置system分区为可读写

./busybox blockdev --setrw /dev/block/by-name/system

注意：分区配置最好在分区mount之前，不然分区mount为可写，在配置分区属性为只读，文件系统会报错。

```

## 问题记录

```text

1. 如何构建一个通用的kernel 后续的机型通过区分FDT或者UBOOT的方式实现不同外设的变化需求，包括外设上电时序和pinmux等主要需求

原生的kernel是把fdt和kernel的镜像打成一个img文件，不能满足通用kernel的需求；
解决方法是在打包的时候不把fdt的内容追加在kernel镜像内，kernel在运行的时候若在自身镜像分区内找不到fdt文件，会自动在resource分区去寻找（uboot也类似）；
所以可以要新增一个resource分区，并修改打包步骤；

2. 原生瑞芯微的分区理解

原生瑞芯微分区分为uboot misc recovery boot rootfs oem userdate media等分区；
瑞芯微的emmc会格式化为GPT的分区格式，即头部有MBR和GPT的隐藏分区，其中GPT分区会放置parameter分区表文件，uboot和kernel会解析分区内容；
misc属于安全启动分区，rocovery分区属于备份升级分区，这些我们都不需要，直接干掉；
oem，userdate，media等分区主要是存放media或者一些库文件的镜像文件，也可以干掉，其中一些必须的库文件可以扔到rootfs中；
至此分区只剩下uboot boot rootfs resource和gpt隐藏分区

3. 关于无法像其他平台一样连接好kernel路径就可以编译驱动问题

需要添加ARCH=arm的编译选项

4. 如何修改debug串口波特率的问题

debug口的串口波特率原生SDK中为1.5M 我们的232芯片不支持那么大的波特率，只能设置为115200的比特率;
debug口的串口修改主要分为3个阶段 ddr，uboot，kernel
ddr： 修改rkbin\tools\ddrbin_param.txt文件中的uart baudrate这一项为115200，其他不动，然后用指令./ddrbin_tool ddrbin_param.txt ../bin/rv11/rv1126_ddr_924MHz_v1.08.bin即可；具体方法在rkbin\tools\ddrbin_tool_user_guide.txt中有介绍
uboot: 修改下config配置即可
kernel: 修改下dts配置即可

```
