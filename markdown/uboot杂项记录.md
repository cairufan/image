# uboot杂项记录

## 前言

- 本文档用于记录平时看uboot时的一些疑难记录点

## 新版uboot启动第一阶段

## 新版uboot启动第二阶段

- Autoboot command->**CONFIG_BOOTCOMMAND** UBOOT启动后自动执行的命令 联咏机型定义为nvt_boot；

- bootargs=**CONFIG_BOOTARGS** UBOOT设置的环境变量；

- 现uboot阶段加密芯片i2c的引脚控制交互由bksystem/u-boot/common/drivers/encrypt里面提前编译好的.o文件链接，源码属于绝密文件，在大师手上；
