# EMMC升级常见问题及流程梳理

<!-- TOC -->

- [EMMC升级常见问题及流程梳理](#emmc升级常见问题及流程梳理)
  - [1. 前言](#1-前言)
  - [2. 基本流程概述](#2-基本流程概述)
  - [3. UBOOT引导备份系统](#3-uboot引导备份系统)
    - [3.1 备份系统组成](#31-备份系统组成)
    - [3.2 引导备份系统](#32-引导备份系统)
  - [4.常用调试手段整理](#4常用调试手段整理)
    - [4.1 快速进入备份系统直接升级](#41-快速进入备份系统直接升级)
    - [4.2 无法插入或识别U盘时如何进行备份升级](#42-无法插入或识别u盘时如何进行备份升级)
    - [4.3 uboot常用指令](#43-uboot常用指令)
  - [5. 常见问题整理](#5-常见问题整理)
    - [5.1 为什么tftp烧录后使用status get没有看到版本号更新](#51-为什么tftp烧录后使用status-get没有看到版本号更新)
    - [5.2 tftp烧录完成后还需要敲什么指令吗](#52-tftp烧录完成后还需要敲什么指令吗)
    - [5.3 为什么升级后看不到备份系统的版本号](#53-为什么升级后看不到备份系统的版本号)
    - [5.4 为什么在测试备份系统异常恢复这项功能时，会出现local起不来的情况](#54-为什么在测试备份系统异常恢复这项功能时会出现local起不来的情况)
    - [5.5 为什么没法进行备份升级或者直接提示找不到kernel](#55-为什么没法进行备份升级或者直接提示找不到kernel)
    - [5.6 板端信息和升级包的头信息不匹配怎么办](#56-板端信息和升级包的头信息不匹配怎么办)

<!-- /TOC -->

## 1. 前言

- 由于其他部门的成员经常会在升级过程中出现各式各样的问题，特别是新入职的同学，对整个升级系统没有达到一种直观的概念，在操作流程和方式上出现很多的错误认识，导致的结果往往是浪费多方面的时间资源

- 本文档旨在引导刚接触升级流程或者还不是很熟悉此流程的同学更好的理解升级过程及其中可能会出现的某些问题及初步分析方向

## 2. 基本流程概述

```shell

在此我们只讨论EMMC介质上的升级流程，其他的nor和nand等介质的不在此文档输出；

首先我们要对正式升级步骤有个基础的认知，如下步骤所示
1. 把升级包放在U盘根目录下的upgrade目录 U盘插入设备 CP4或者IE点启动升级
2. 业务层把升级包校验完毕后，后把会升级包放在/mnt/emmc/scare_rw/.upgrade/目录下
3. 更新状态区升级状态，设备自动重启进入备份系统升级
4. 备份系统升级完成后更新状态区版本号及升级状态等信息，再次自动重启进入正常流程
5. 升级完成

```

步骤如图所示  
![emmc](https://cdn.jsdelivr.net/gh/cairufan/image@main//picture/emmc.png)

## 3. UBOOT引导备份系统

### 3.1 备份系统组成

- 备份系统其实是由一个kernel和rootfs打包到一起的镜像分区，kernel及rootfs都属于裁剪后的 所以功能上只保留升级了所必需的项 保证大小符合分区限制

- 在备份系统分区中的rootfs项里面 也仅仅放了一些必备的驱动 比如加密芯片驱动及GPIO驱动；以及一个用于升级的进程 在rootfs启动时自动执行

### 3.2 引导备份系统

```shell

- 在我们的设备中其实会存在两份kernel，其中一份是正式的流程，另一份是备份升级流程的  
- 在UBOOT阶段会根据条件自行选择引导  

以下是UBOOT引导kernel的步骤及判断条件

1. 首先判断状态区的升级标志位，若处于升级状态，进入备份系统升级

2. 若步骤1不成立，则判断状态区内的一个ERR_COUNT计数，若ERR_COUNT >= 3，下一步则进行flash分区的CRC校验

3. CRC校验值同样也是存放状态区内，当进行CRC检验时，会从当前的flash中读取当前内容进行计算，最后的计算结果和状态区中存放的CRC值进行比较

4. 若CRC结果不一致，则进入备份系统升级；反之则进入正常流程

```

```shell

注释
1. ERR_COUNT:  
- 错误次数，存在放状态区内，初始值为0
- 当uboot启动时，会自动把该值+1，用于判断是否引导备份系统
- 当运行local中的mainbusiness时，会自动把ERR_COUNT清零
- 升级完成后也会自动把ERR_COUNT清零
- 这样至少能保证kernel或者local等分区出问题的时候 能自动进入备份系统恢复

2. CRC校验
- 一般用来检测或校验数据传输或者保存后可能出现的错误，存放在状态区内
- 除了一些可读写的分区，例如data1 data2 状态区 参数区等等外，其他的分区一般都会列入CRC的计算分区
- CRC校验值会在备份升级后重新计算，最后再更新到状态区中，其他情况均不会主动更新CRC值
- 可在UBOOT阶段手动执行status set crc更新CRC值

```

流程如下图所示  

![2110211016](https://cdn.jsdelivr.net/gh/cairufan/image@main//picture/2110211016.png)

## 4.常用调试手段整理

### 4.1 快速进入备份系统直接升级

```shell

此方式不依赖于local的运行，简单直接，但是有以下的前提条件
· 板端已烧录过备份系统
· 板端已烧录过加密芯片串号
· 升级包里的头信息要和版本保持一致（一般服务器打包出来的对应机型升级包都不会有什么问题）

步骤如下
1. 完成上列前提条件
2. 升级包放在U盘根目录下的upgrade目录中，U盘插入设备
3. 上电启动，按住CTRL+C进入UBOOT命令阶段
4. 擦除local分区的数据部分数据(只要不是uboot及之前的分区即可 不一定非要擦除local)
5. 执行status set cnt 3;reset

上述这种方式其实有个弊端，就是假如你擦除了local分区的数据，然后你进入备份系统后升级失败，在最后计算CRC的时候就会把当前的local分区数据记录进去；  
下一次你再想重新进入备份系统的时候，直接擦除local分区的方式就用了，因为CRC校验是会通过的；  
所以我推荐把上述的步骤4改成直接擦除状态分区，这样CRC就不会校验通过，每次都能顺利的进入备份系统；

```

### 4.2 无法插入或识别U盘时如何进行备份升级

```shell

除了要依赖4.1的前提条件外 还需要依赖以下前提条件：
- 网络能通
- 能进入到rootfs

如果local能正常运行，直接用IE升级即可，这种就不多做介绍了；
如果没有local或者local没有正常运行的情况下，就需要参考以下步骤进行备份升级

1. 在网络正常的情况下板端mount到你PC端能访问的某个服务器或者文件夹，例如114服务器或者虚拟机这种
2. 把升级包放到文件夹内 确认板端能访问和获取的到升级包
3. 把升级包拷贝到板端的/mnt/emmc/scare_rw/.upgrade目录下
4. 参照4.1的操作步骤进入备份系统升级

因为备份升级首先会检测是否有U盘升级包，如果没有的话，再检测板端有没有升级包，有则继续升级，只要保证有一方成立即可；

```

### 4.3 uboot常用指令

```shell

在uboot的命令阶段 我们最常用的指令就是status 用于查询状态区的数据和版本号灯

在uboot命令阶段敲status，可以看到以下的命令提示

nvt_emmc@51068# <INTERRUPT>
nvt_emmc@51068# status 
status - system status operate

Usage:
status status get
status set crc
status clr crc
status clr exception
status set cnt
status set set_cnt

nvt_emmc@51068# 

- status get <获取状态区的基本参数>
如下所示
nvt_emmc@51068# status get

MMC read: dev # 0, block # 2021376, count 2048 ... 2048 blocks read: OK
[upgrade/SysStateOperate.c, check_sys_state_valid, 39]:crc_state 1016 crc_real 1016 crc 0x9CF4FC6C

[cmd/cmd_status.c, print_status, 21]:sysstat_crc 0x9CF4FC6C

[cmd/cmd_status.c, print_status, 22]:upgrade_addr 0x0

[cmd/cmd_status.c, print_status, 23]:tv_system 0

[cmd/cmd_status.c, print_status, 24]:crc_len 1016

[cmd/cmd_status.c, print_status, 25]:back sys crc 0x539935F0

[cmd/cmd_status.c, print_status, 27]:cvbslog_version T21092602

[cmd/cmd_status.c, print_status, 28]:vgalog_version T21092602

[cmd/cmd_status.c, print_status, 29]:updatelog_version T21092602

[cmd/cmd_status.c, print_status, 30]:uboot_version T21061800

[cmd/cmd_status.c, print_status, 31]:kernel_version T21080900

[cmd/cmd_status.c, print_status, 32]:rootfs_version T21082600

[cmd/cmd_status.c, print_status, 33]:local_version T21092602

[cmd/cmd_status.c, print_status, 34]:extend_version T21092602

[cmd/cmd_status.c, print_status, 35]:resource_version 

[cmd/cmd_status.c, print_status, 36]:extra_firm_ver 

[cmd/cmd_status.c, print_status, 37]:customer_version 

[cmd/cmd_status.c, print_status, 38]:plugin_version 

[cmd/cmd_status.c, print_status, 47]:ipc err cnt 0

[cmd/cmd_status.c, print_status, 48]:nand set cnt 0

[cmd/cmd_status.c, print_status, 50]:system_main_ver X5NPRO_B342_T210926.02

[cmd/cmd_status.c, print_status, 51]:system_backup_ver T21031501

[cmd/cmd_status.c, print_status, 52]:loader_version T21050800

[cmd/cmd_status.c, print_status, 53]:fdt_version T21061800

[cmd/cmd_status.c, print_status, 54]:fdt_restore_version 

[cmd/cmd_status.c, print_status, 55]:ramdisk_version 

[cmd/cmd_status.c, print_status, 56]:mbr_version 

---------------------------------------------------

sysstat_crc：CRC校验值
upgrade_addr：升级标志位
tv_system：制式
ipc err cnt：错误计数
system_main_ver：升级包主版本号
system_backup_ver：备份系统版本号
其他基本为一些分区的版本号

---------------------------------------------------

- status set cnt 3 <手动设置错误计数 最后的3就是设置的值 可修改>

- status set crc <输入后手动更新当前crc的值到状态区中>

```

## 5. 常见问题整理

### 5.1 为什么tftp烧录后使用status get没有看到版本号更新

```shell


这是因为使用tftp或者dd指令只是单纯的把二进制烧录到某个分区内而已，并不会更新到状态区的数据，当然看不到版本号更新

```

### 5.2 tftp烧录完成后还需要敲什么指令吗

```shell

同上述原因，tftp烧录后我们其实还需要手动敲一下status set crc;status set cnt 0

crc是为了能让系统在异常（比如不启动local）时不会自动进入备份系统升级回退到之前的版本

cnt是为了清除当前的错误次数，确保下一次能直接进入正常启动流程

```

### 5.3 为什么升级后看不到备份系统的版本号

```shell

一般我们的网页出的升级包中是不会包含备份系统那部分的镜像，同时我们也不会把备份系统放在正式的升级包中，所以升级后是不会看到备份系统的版本号的

想要status get中显示备份系统版本号，请用BSP组提供的升级包升级

```

### 5.4 为什么在测试备份系统异常恢复这项功能时，会出现local起不来的情况

```shell

这种情况下一般是U盘内或者/mnt/emmc/scare_rw/.upgrade目录没有升级包，或者有升级包，但是是备份系统的升级包;
然后在测异常恢复的功能时一般步骤是擦除local分区的数据;
因为升级包是备份系统的升级包，这种包里面是只有BKSystem分区的镜像的;
这时候进入备份系统后升级的分区只有BKSystem分区，不会更新其他分区，而local已经被擦除，当然起不来;

```

### 5.5 为什么没法进行备份升级或者直接提示找不到kernel

```shell

这种情况请参考4.1的前提条件

```

### 5.6 板端信息和升级包的头信息不匹配怎么办

```shell

此情况一般的解决方式是拆包后修改包头信息，若板端的加密芯片号不匹配，则修改加密芯片串号

```
