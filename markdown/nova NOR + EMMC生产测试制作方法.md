# nova NOR + EMMC生产测试制作方法

## 设备准备

生产测试功能正常测试完

备份系统升级进去

设备版本号之类的显示正常

参数该擦除的擦除

该设置的设置。

可以使用的剩余容量不小于1G的 FAT32格式u盘

## SPI NOR上镜像制作

设备正常启动后，插上u盘，执行如下指令

进入u盘挂载目录

cd /var/mnt/usbstate/front/

## 导出SPI NOR 分区内容

dd if=/dev/mtdblock0 of=Firmware_D5X_XRV4_SPI_NOR_Txxxxxxxx bs=1M count=1;  

---

备注：  
Firmware_D5X_XRV4_SPI_NOR_Txxxxxxxx为输出的文件名，根据实际机型来填写，替换其中的DX5_XRV4位实际机型名，Txxxxxxxx替换为实际日期

---

## 导出emmc 分区内容

dd if=/dev/mmcblk0 of=Firmware_D5X_XRV4_EMMC_Txxxxxx bs=1M count=988;

---

备注：  
Firmware_D5X_XRV4_EMMC_Txxxxxx为输出的文件名，根据实际机型来填写，替换其中的DX5_XRV4位实际机型名，Txxxxxxxx替换为实际日期

---

## 退出u盘目录，同步u盘信息

cd /

sync

## 拔出U盘

u盘里的Firmware俩文件即分别为SPI NOR 和EMMC烧录文件
