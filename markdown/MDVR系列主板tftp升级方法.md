# MDVR系列产品 tftp分区升级方法

## 32M 主机

MDVR N9M X5_III/X7_I/T5 系列主板tftp升级方法

拔电源重新开机任意键停在u-boot下
首先在u-boot下输入printenv，查看：
				ipaddr=192.168.50.17
				serverip=192.168.50.18	
其中ipaddr为板子的IP，serverip为自己电脑的IP，如果不是则要设置：
				setenv serverip 192.168.50.18
				setenv ipaddr 192.168.50.17

设置成功ping一下自己电脑的IP，如果is alive就可以开始升级，一般用tftpd32.exe工具。为保险起见，
先升级u-boot之后的其它软件，如kernel等，没问题再升uboot。

### uboot+Logo

```
mw.l 0x82000000 0xffffffff 0x100000;tftp 0x82000000 STM_X7_SE_UBOOT_T20112500;
sf probe 0;sf erase 0 0x60000;sf write 0x82000000 0 0x60000;
```

​	

### CVBS 

```
mw.l 0x82000000 0xffffffff 0x100000;tftp 0x82000000 logome.jpg;	
sf probe 0;	sf erase 0x60000 0x20000;	sf write 0x82000000 0x60000 0x20000;
```


​	

### Upgrade

```
mw.l 0x82000000 0xffffffff 0x100000;tftp 0x82000000 update.jpg;	
sf probe 0;	sf erase 0x80000 0x20000;	sf write 0x82000000 0x80000 0x7000;
```



### VGA

```
tftp 0x82000000 vga.jpg;	sf probe 0;sf write 0x82000000 0x87000 0x10000;
sf erase 0x80000 0x20000;sf write 0x82000000 0x87000 0x10000;
```



### kernel

```
mw.l 0x82000000 0xFFFFFFFF 0x200000;tftp 0x82000000 STM_3536_KERNEL_T20121100_01;
sf probe 0;sf erase 0xA0000 0x1C0000;sf write 0x82000000 0xA0000 0x1C0000;
```

​       

### rootfs

```
tftp 0x82000000 STM_V16HC_ROOTFS_T17122500_NFS;sf probe 0;sf erase 0x260000 0x300000;
sf write 0x82000000 0x260000 ${filesize};
```

​	

### app local

```
tftp 0x82000000 STM_LOCAL_0x14_T20041700;
sf probe 0;	sf erase 0x560000 0x400000;
sf write 0x82000000 0x560000 ${filesize}
```


==============================================
对于分区合并的版本，local指令为：

```
tftp 0x82000000 STM_LOCAL_0x14_T20083106
sf probe 0;	sf erase 0x560000 0x17C0000/0x17E0000
sf write 0x82000000 0x560000 ${filesize}
```

同时没有extend/resource/customer/plug/firmware分区

### app extend

```
tftp 0x82000000 STM_X5_AHD_EXTEND_T20082800;
sf probe 0;	sf erase 0x960000 0x600000;
sf write 0x82000000 0x960000 ${filesize};
```



### app resource

```
tftp 0x82000000 STM_RESOURCE_0x16_T20041705;
sf probe 0;	sf erase 0xF60000 0x600000;
sf write 0x82000000 0xF60000 ${filesize}
```


​	

### app customer

```
tftp 0x82000000 STM_D550_CUST_T20081700_LI;
sf probe 0;sf erase 0x1560000 0x500000;
sf write 0x82000000 0x1560000 ${filesize};
```


​	

### app plug

```
tftp 0x82000000 STM_PLUGIN_0x18_T20041705;
sf probe 0;	sf erase 0x1A60000 0x100000;
sf write 0x82000000 0x1A60000 ${filesize}
```


​	

### APP FIRMWARE

```
tftp 0x82000000 STM_FIRMWARE_0x19_T20030503;
sf probe 0;	sf erase 0x1B60000 0x1E0000
sf write 0x82000000 0x1B60000 ${filesize}
```


​	

### 参数区

erase (test purpose only)

```
sf probe 0;	sf erase 0x1D40000 0x30000;sf erase 0x1DA0000 0x30000
```


​	

	sysparameter backup
	sf probe 0
	sf erase 0x1DA0000 0x30000
	
	customer parameter
	sf probe 0;sf erase 0x1D80000 0x20000;sf erase 0x1DE0000 0x20000;
	
	customer parameter backup
	sf probe 0;sf erase 0x1DE0000 0x20000;

### sys status

(test purpose only), system status inhabit at last 1K of parameter 

	sf probe 0;sf erase 0x1D70000 0x10000;sf probe 0;sf erase 0x1DD0000 0x10000;
	
	sys status backup
	sf probe 0;sf erase 0x1DD0000 0x10000

### backup system

```
mw.l 0x82000000 0xFFFFFFFF 0x200000;	tftp 0x82000000 STM_3521D_BACKUP_T20112200;
sf probe 0;sf erase 0x1E00000 0x200000;	sf write 0x82000000 0x1E00000 0x200000;
```

​	

### 避免进备份

```
encrypt init;encrypt cnt set 0;
status set crc;reset;
```

### 手工进备份

```
encrypt init;encrypt cnt set 4;
sf probe 0;sf erase 0x560000 0x10000;reset;
```



## 16M IPC	

MDVR N9M IPC 712C6 等16M Flash 系列tftp方式

拔电源重新开机任意键停在u-boot下
首先在u-boot下输入printenv，查看：
				ipaddr=192.168.50.17
				serverip=192.168.50.18	
其中ipaddr为板子的IP，serverip为自己电脑的IP，如果不是则要设置：
				setenv serverip 192.168.50.18
				setenv ipaddr 192.168.50.17

设置成功ping一下自己电脑的IP，如果is alive就可以开始升级，一般用tftpd32.exe工具。为保险起见，
先升级u-boot之后的其它软件，如kernel等，没问题再升uboot。

1). uboot:
	mw.l 0x41000000 0xFFFFFFFF 0x100000;tftp 0x41000000 STM_742_VA_UBOOT_T19091200;
	sf probe 0;sf erase 0 0x40000;sf write 0x41000000 0 0x40000;
	
2). kernel:
	mw.l 0x41000000 0xFFFFFFFF 0x200000;tftp 0x41000000 STM_3516EV200_KERNEL_T19090300;
	sf probe 0;sf erase 0x40000 0x1C0000;sf write 0x41000000 0x40000 0x1C0000;

3). rootfs:	
	tftp 0x41000000 STM_742_VA_ROOTFS_T19090900_NFS;
	sf probe 0;	sf erase 0x200000 0x260000;sf write 0x41000000 0x200000 ${filesize};
	
4). app
	tftp 0x82000000 STM_712C6_VA_APP_T14112700
	sf probe 0;	sf erase 0x460000 0xB20000
	sf write 0x82000000 0x460000 ${filesize}

9). sys parameter and backup parameter erase (test purpose only)
	sf probe 0;	sf erase 0xF80000 0x30000;sf erase 0xFC0000 0x30000;
	
	sysparameter backup
	sf probe 0
	sf erase 0xFC0000 0x30000

10). sys status (test purpose only), system status inhabit at last 1K of parameter 

	sf probe 0;sf erase 0xFB0000 0x10000;sf probe 0;sf erase 0xFF0000 0x10000
	
	sys status backup


3. MDVR N9M IPC 916 920 等32M Flash 系列tftp方式

	拔电源重新开机任意键停在u-boot下
	首先在u-boot下输入printenv，查看：
					ipaddr=192.168.50.17
					serverip=192.168.50.18	
	其中ipaddr为板子的IP，serverip为自己电脑的IP，如果不是则要设置：
					setenv serverip 192.168.50.18
					setenv ipaddr 192.168.50.17

	设置成功ping一下自己电脑的IP，如果is alive就可以开始升级，一般用tftpd32.exe工具。为保险起见，
	先升级u-boot之后的其它软件，如kernel等，没问题再升uboot。

1). uboot:
	mw.l 0x82000000 0xFFFFFFFF 0x100000
	tftp 0x82000000 STM_916_VA_UBOOT_T15092401
	sf probe 0
	sf erase 0 0x40000
	sf write 0x82000000 0 0x40000
	
2). kernel:
	mw.l 0x82000000 0xFFFFFFFF 0x200000
	tftp 0x82000000 STM_916_VA_KERNEL_T15070200
	sf probe 0;sf erase 0x40000 0x1C0000
	sf write 0x82000000 0x40000 0x1C0000

3). rootfs:	
	tftp 0x82000000 STM_916_VA_ROOTFS_T15092401
	sf probe 0;	sf erase 0x200000 0x260000
	sf write 0x82000000 0x200000 ${filesize}
	
4). app
	tftp 0x82000000 STM_916_VA_APP_T15092401
	sf probe 0;	sf erase 0x460000 0xB20000
	sf write 0x82000000 0x460000 ${filesize}

5). app extern
	tftp 0x82000000 MDVR_EXTEND(0x00000015)(T15092401)
	sf probe 0;	sf erase 0xF80000 0x1000000
	sf write 0x82000000 0xF80000 ${filesize}

9). sys parameter and backup parameter erase (test purpose only)
	sf probe 0;	sf erase 0x1F80000 0x30000
	
	sysparameter backup
	sf probe 0
	sf erase 0x1FC0000 0x30000

10). sys status (test purpose only), system status inhabit at last 1K of parameter 

	sf probe 0;sf erase 0x1FB0000 0x10000
	
	sys status backup
	sf probe 0;sf erase 0x1FF0000 0x10000

4. 滴滴主机 IPC C6D C6E
setenv ipaddr 192.168.50.17;setenv serverip 192.168.50.18;

1). uboot

	mw.l 	0x82000000 0xffffffff 0x100000
	tftp 0x82000000 STM_923_VC_UBOOT_T16111001
	
	sf probe 0;sf erase 0 0x40000;sf write 0x82000000 0 0x40000

2). kernel:
	mw.l 	0x82000000 0xffffffff 0x200000
	tftp 0x82000000 STM_923_VC_KERNEL_T16112600
	
	sf probe 0;sf erase 0x40000 0x1C0000;sf write 0x82000000 0x40000 0x1c0000;

3). rootfs:	

	tftp 0x82000000 STM_923_VC_ROOTFS_T16112900_NFS
	
	sf probe 0;sf erase 0x200000 0x260000;sf write 0x82000000 0x200000 ${filesize};

4). local:
	tftp 0x82000000 STM_LOCAL_0x14_T19092499.C6D_III
	
	sf probe 0;sf erase 0x460000 0xB20000;sf write 0x82000000 0x460000 ${filesize};

4). extend:
	tftp 0x82000000 STM_923_VC_EXTEND_T16113000
	
	sf probe 0;sf erase 0xF80000 0x100000;sf write 0x82000000 0xF80000 ${filesize};

5). 擦除参数
	sf probe 0;sf erase 0x1DB0000 0x10000; sf erase 0x1DF0000 0x10000;
	
	
5. 3515 3520 平台烧录方法
setenv ipaddr 192.168.50.225;setenv serverip 192.168.50.18;

1). uboot+logo:
	protect off 0x80000000 +0x60000
	erase 0x80000000 +0x60000
	tftp 0x80000000 STM_3520_UBOOT_T17070800
	protect on 0x80000000 +0x60000
	
	erase 0x80060000 +0x20000
	tftp 0x80060000 logome.JPG （开机图片）
	
	erase 0x80080000 +0x20000
	tftp 0x80080000 update.JPG（升级图片）
	tftp 0x80087000 vga.jpg   （VGA开机Logo）

2). kernel:
	erase 0x800A0000 +0x1C0000
	tftp 0x800A0000 STM_3520_KERNEL_T17070800

3.rootfs:	
	erase 0x80260000 +0x300000
	tftp 0x80260000 STM_S28_ROOTFS_T17070800
	
4.local:
	erase 0x80560000 +0x400000
	tftp 0x80560000 STM_A5_II_APP_LOCAL_T16012001

5). app extern
	erase 0x80960000 +0x600000
	tftp 0x80960000 STM_A5_II_APP_EXT_T16012001

6). app resource
	erase 0x80F60000 +0x600000
	tftp 0x80F60000 STM_A5_II_RESOURCE_T16012001
	
7). app customer
	erase 0x81560000 +0x500000
	tftp 0x81560000 STM_A5_II_CUSTOMER_T16012001
	
8). app plug
	erase 0x81A60000 +0x100000
	tftp 0x81A60000 STM_A5_II_PLUGIN_T16012001


9). APP FIRMWARE
	erase 0x81B60000 +0x1E0000
	tftp 0x81B60000 STM_X5_III_EXTRA_BJGJ_T14110700


​	
10). sys parameter and backup parameter erase (test purpose only)
​	erase 0x81D40000 +0x30000
​	
​	sysparameter backup
​	erase 0x81DA0000 +0x30000
​	
​	customer parameter
​	erase 0x81D80000 +0x20000;
​	
​	customer parameter backup
​	erase 0x81DE0000 +0x20000;

12). backup system
	erase 0x81E00000 +0x200000
	tftp 0x81E00000 RMMDVR_A5_II_T15072100_backup


4. MDVR N9M ES4206 ES8412 系列主板tftp升级方法

	拔电源重新开机任意键停在u-boot下
	首先在u-boot下输入printenv，查看：
					ipaddr=192.168.50.17
					serverip=192.168.50.18	
	其中ipaddr为板子的IP，serverip为自己电脑的IP，如果不是则要设置：
					setenv serverip 192.168.50.18
					setenv ipaddr 192.168.50.17

	设置成功ping一下自己电脑的IP，如果is alive就可以开始升级，一般用tftpd32.exe工具。为保险起见，
	先升级u-boot之后的其它软件，如kernel等，没问题再升uboot。

1). uboot+Logo:
	sf probe 0;sf lock 0;	//关闭flash写保护
	tftp 0x82000000 STM_ES4206_UBOOT_T16103101
	sf probe 0
	sf erase 0 0x60000
	sf write 0x82000000 0 0x60000
	
CVBS 
	tftp 0x82000000 logome.jpg
	sf probe 0
	sf erase 0x60000 0x20000
	sf write 0x82000000 0x60000 0x20000
	
Upgrade
	tftp 0x82000000 update.jpg
	sf probe 0
	sf erase 0x80000 0x20000
	sf write 0x82000000 0x80000 0x7000

VGA
	tftp 0x82000000 vga.jpg
	sf probe 0

sf erase 0x80000 0x20000

	sf write 0x82000000 0x87000 0x10000

2). kernel:
	sf probe 0;sf lock 0;	//关闭flash写保护
	mw.l 0x82000000 0xFFFFFFFF 0x200000
	tftp 0x82000000 STM_ES4206_KERNEL_T16091000
	sf probe 0;sf erase 0xA0000 0x1C0000
	sf write 0x82000000 0xA0000 0x1C0000

3). rootfs:
	sf probe 0;sf lock 0;	//关闭flash写保护
	tftp 0x82000000 STM_ES4206_ROOTFS_T16102901
	sf probe 0;	sf erase 0x260000 0x300000
	sf write 0x82000000 0x260000 ${filesize}
	
4). backup system
	sf probe 0;sf lock 0;	//关闭flash写保护
	mw.l 0x82000000 0xFFFFFFFF 0x200000;
	tftp 0x82000000 STM_ES4206_BACKUP_T16102901
	sf probe 0;sf erase 0x560000 0x1E0000;
	sf write 0x82000000 0x560000 0x1E0000;

	清除自动进备份系统方式:
	encrypt init;encrypt cnt set 0;
	status set crc;reset;

5). app local
	sf probe 0;sf lock 0;	//关闭flash写保护
	tftp 0x82000000 STM_ES4206_APP_LOCAL_T16102890
	sf probe 0;	sf erase 0x740000 0x400000
	sf write 0x82000000 0x740000 ${filesize}
	
6). app extern
	sf probe 0;sf lock 0;	//关闭flash写保护
	tftp 0x82000000 STM_ES4206_APP_EXT_T16102890
	sf probe 0;	sf erase 0xB40000 0x600000
	sf write 0x82000000 0xB40000 ${filesize}

7). app resource
	tftp 0x82000000 STM_ES4206_RESOURCE_T16102890
	sf probe 0;	sf erase 0x1140000 0x600000
	sf write 0x82000000 0x1140000 ${filesize}
	
8). app customer
	tftp 0x82000000 STM_ES4206_CUSTOMER_T16102890
	sf probe 0;sf erase 0x1740000 0x500000
	sf write 0x82000000 0x1740000 ${filesize}
	
9). app plug
	tftp 0x82000000 STM_ES4206_PLUGIN_T16102890
	sf probe 0;	sf erase 0x1C40000 0x100000
	sf write 0x82000000 0x1C40000 ${filesize}
	
10). sys parameter and backup parameter erase (test purpose only)
	sf probe 0;	sf erase 0x1D40000 0x30000
	
	sysparameter backup
	sf probe 0
	sf erase 0x1DA0000 0x30000
	
	customer parameter
	sf probe 0;sf erase 0x1D80000 0x20000;
	
	customer parameter backup
	sf probe 0;sf erase 0x1DE0000 0x20000;

11). sys status (test purpose only), system status inhabit at last 1K of parameter 

	sf probe 0;sf erase 0x1D70000 0x10000;sf probe 0;sf erase 0x1DD0000 0x10000;
	
	sys status backup
	sf probe 0;sf erase 0x1DD0000 0x10000

12). APP FIRMWARE
	tftp 0x82000000 STM_ES4206_EXTRA_T00000001
	sf probe 0;	sf erase 0x1E00000 0x1E0000
	sf write 0x82000000 0x1E00000 ${filesize}
	
13). 清除Log分区 
	sf probe 0;	sf erase 0x1FE0000 0x20000


5. 滴滴主机 IPC M4N0200
setenv ipaddr 192.168.52.110;setenv serverip 192.168.52.108;

1).uboot
	mw.l 	0x82000000 0xffffffff 0x100000
	tftp 0x82000000 STM_928_VB_UBOOT_T16111001

	sf probe 0;sf erase 0 0x40000;sf write 0x82000000 0 0x40000

2).kernel:
	mw.l 	0x82000000 0xffffffff 0x200000
	tftp 0x82000000 STM_928_VB_KERNEL_T16112600
	
	sf probe 0;sf erase 0x40000 0x1C0000;sf write 0x82000000 0x40000 0x1c0000;

3).rootfs:	

	tftp 0x82000000 STM_928_VB_ROOTFS_T16112900_NFS
	
	sf probe 0;sf erase 0x200000 0x260000;sf write 0x82000000 0x200000 ${filesize};

4).local:
	tftp 0x82000000 STM_928_VB_LOCAL_T16113000
	
	sf probe 0;sf erase 0x460000 0xB20000;sf write 0x82000000 0x460000 ${filesize};

5).extend:
	tftp 0x82000000 STM_928_VB_EXTEND_T16113000
	
	sf probe 0;sf erase 0xF80000 0xAE0000;sf write 0x82000000 0xF80000 ${filesize};

6).擦除参数
	系统参数 
	sf probe 0;sf erase 0x1F40000 0x30000; 
	客户参数
	sf probe 0;sf erase 0x1F80000 0x20000;
	
	系统参数备份
	sf probe 0;sf erase 0x1FA0000 0x30000;
	客户参数
	sf probe 0;sf erase 0x1FE0000 0x20000;

## 256M NAND

setenv ipaddr 192.168.52.110;setenv serverip 192.168.52.13;

### uboot

```
mw.l 0x82000000 0xffffffff 0x100000;tftp 0x82000000 STM_940_VA_UBOOT_T20112600;

nand erase 0 0x60000;nand write 0x82000000 0 0x60000;
```

【注意上述写入大小，write的最后一个参数必须是实际文件大小的向上取 0x20000的整数倍大小】

### kernel

​	mw.l 	0x82000000 0xffffffff 0x400000;
​	tftp 0x82000000 STM_KERNEL_0x12_T19072900;
​	

	nand erase 0x100000 0x400000;nand write 0x82000000 0x100000 0x400000;
	
	【注意上述写入大小，write的最后一个参数必须是实际文件大小的向上取 0x20000的整数倍大小】

### rootfs

	tftp 0x82000000 STM_940_VA_ROOTFS_T20121400;
	
	nand erase 0x500000 0x1000000;nand write.yaffs 0x82000000 0x500000 ${filesize};
	tftpput 0x82000000 filename_ ${filesize}

### local

​	tftp 0x82000000 STM_940C28_LOCAL_T20112600
​	

	nand erase 0x1500000 0x4000000;nand write.yaffs 0x82000000 0x1500000 ${filesize};

### extend

```
tftp 0x85000000 STM_EXTEND_0x15_T20091202;

nand erase 0x5500000 0x9400000;nand write.yaffs 0x85000000 0x5500000 ${filesize};
```



### 擦除参数

	系统参数 
	nand erase 0xe900000 0x400000;

​	或直接系统起来后删除 /var/SysPara/ 和 /var/CustPara/下所有文件
​	rm -rvf /var/SysPara/*
​	rm -rvf /var/CustPara/*

### 擦除状态区

​	nand erase 0xed00000 0x100000;
​	

## 8G EMMC 海思

setenv ipaddr 192.168.52.110;setenv serverip 192.168.52.13;

【**注意写入大小，write的最后一个参数必须是实际文件大小的向上取 512的整数倍大小**】

### uboot

```
mw.l 	0x82000000 0x0 0x100000;tftp 0x82000000 STM_ADkit_UBOOT_T20121500;
mw.l 	0x82000000 0x0 0x100000;tftp 0x82000000 STM_940_VB_UBOOT_T20121700;
mw.l 	0x82000000 0x0 0x100000;tftp 0x82000000 STM_D43_UBOOT_T20112500;
```



	mmc write 0 0x82000000 0 0x500;

​	

### logo cvbs

​	mw.l 	0x82000000 0x0 0x100000;tftp 0x82000000 logome_720p.jpg
​	mmc write 0 0x82000000 0x600 0x20;
​	

### logo upgrade

mw.l 	0x82000000 0x0 0x100000;tftp 0x82000000 update.jpg;
tftp 0x82007000 logome_720p.jpg
mmc write 0 0x82000000 0x700 0x100;

### kernel

```
mw.l 	0x82000000 0x0 0x500000;tftp 0x82000000 STM_3516EV200_KERNEL_T19081000;
mw.l 	0x82000000 0x0 0x500000;tftp 0x82000000 STM_3519AV100_KERNEL_T19092700;
mw.l 	0x82000000 0x0 0x500000;tftp 0x82000000 STM_3516CV500_KERNEL_T20090500;

mmc write 0 0x82000000 0x800 0x1D80;
```



### rootfs

```
mw.l 	0x82000000 0x0 0x1000000;tftp 0x82000000 STM_D43_ROOTFS_T20111800_NFS;
mw.l 	0x82000000 0x0 0x1000000;tftp 0x82000000 STM_ADkit_ROOTFS_T20120800_NFS;

mw.l 	0x82000000 0x0 0x1000000;tftp 0x82000000 STM_AX4_ROOTFS_T20080400_NFS;	
```



	mmc write 0 0x82000000 0x2800 0x1D80;
	#mmc write 0 ${fileaddr} ${Root_offset_blk} ${blkcnt};

### local

​	tftp 0x82000000 STM_ADkit_LOCAL_T20111800;

tftp 0x82000000 STM_D43_LOCAL_T20112500;	

	mmc write 0 0x82000000 0xC800 0x9CA0;
	#mmc write 0 ${fileaddr} ${AppLocal_offset_blk} ${blkcnt};

### AI/EXTEND

​	tftp 0x82000000 STM_EXTEND_0x15_T20042195;
​	

	mmc write 0 0x82000000 0x4C800 0x2F1A0;
	#mmc write 0 ${fileaddr} ${AppExt_offset_blk} ${blkcnt};

### backup system

​	mw.l 	0x82000000 0x0 0x500000;tftp 0x82000000 STM_3516DV300_BACKUP_T20111800_01_SHA;

mw.l 	0x82000000 0x0 0x500000;tftp 0x82000000 STM_3516DV300_BACKUP_T20092100;	

	mmc write 0 0x82000000 0xA800 0x2000;
	#mmc write 0 ${fileaddr} ${BKSystem_offset_blk} ${blkcnt};

​	

### 擦除参数

	mw.l 0x82000000 0x0  0x1000000;	mmc write 0 0x82000000 0x170800 0x8000;
	
	或直接系统起来后清空对应分区
	dd if=/dev/zero of=/dev/mmcblk0p6 bs=1M count=16

​	

### 擦除状态区

```
mw.l 0x82000000 0x0 0x100000;mmc write 0 0x82000000 0x178800 0x800;
```


​	
​	或直接系统起来后清空对应分区
​	dd if=/dev/zero of=/dev/mmcblk0p7 bs=1M count=1

encrypt init;encrypt cnt set 0;
status clr cnt;reset;

## 16G EMMC海思

env set ipaddr 192.168.55.12;env set serverip 192.168.55.234;

【注意写入大小，write的最后一个参数必须是实际文件大小的向上取 512的整数倍大小】

### uboot

```
mw.l 0x42000000 0x0 0x100000;tftp 0x42000000 STM_A8PRO_UBOOT_T20121100;
mmc write 0 0x42000000 0 0x500;
```

​	

### logo cvbs

```
#mw.l 	0x42000000 0x0 0x100000;tftp 0x42000000 logome_720p.jpg
mw.l 	0x42000000 0x0 0x100000;tftp 0x42000000 720x576_color_bar.jpg
mw.l 	0x42000000 0x0 0x100000;tftp 0x42000000 logome.jpg
mmc write 0 0x42000000 0x600 0x20;
```


​	

### logo upgrade

```
mw.l 0x42000000 0x0 0x100000;tftp 0x42000000 update.jpg;
mw.l 0x42000000 0x0 0x100000;tftp 0x42000000 logome_720p.jpg
mw.l 0x42000000 0x0 0x100000;tftp 0x42000000 logome_720p.jpg
mmc write 0 0x42000000 0x700 0x30;
```



### logo vga

```
mw.l 0x42000000 0x0 0x100000;tftp 0x42000000 vga.jpg;
mw.l 0x42000000 0x0 0x100000;tftp 0x42000000 vga_win7.jpg
mmc write 0 0x42000000 0x738 0x50;
```



### kernel

```
mw.l 0x42000000 0x0 0x1000000;tftp 0x42000000 STM_3531DV200_KERNEL_T20092900_4096;
mmc write 0 0x42000000 0x800 0x8000;
```



### rootfs:	

​	

```
mw.l 0x42000000 0x0 0x1000000;tftp 0x42000000 STM_A8PRO_ROOTFS_T20120400_NFS;
mmc write 0 0x42000000 0x8800 0x2AA0;
#mmc write 0 ${fileaddr} ${Root_offset_blk} ${blkcnt};
```

### local:

```
tftp 0x42000000 STM_A8PRO_LOCAL_T20120400;
```

​	

	mmc write 0 0x42000000 0x15800 0xB158;
	#mmc write 0 ${fileaddr} ${AppLocal_offset_blk} ${blkcnt};

### AI/EXTEND

​	tftp 0x42000000 STM_EXTEND_0x15_T20042195;
​	

	mmc write 0 0x42000000 0x55800 0x2F1A0;
	#mmc write 0 ${fileaddr} ${AppExt_offset_blk} ${blkcnt};

### backup system

​	mw.l 	0x42000000 0x0 0xA00000;tftp 0x42000000 STM_3531DV200_BACKUP_T20092200;
​	

	mmc write 0 0x42000000 0x10800 0x5000;
	#mmc write 0 ${fileaddr} ${BKSystem_offset_blk} ${blkcnt};

​	

### 擦除参数

	mw.l 0x42000000 0x0 0x1000000;	mmc write 0 0x42000000 0x115800 0x8000;
	
	或直接系统起来后清空对应分区
	dd if=/dev/zero of=/dev/mmcblk0p6 bs=1M count=16

​	

### 擦除状态区

​	mw.l 0x42000000 0xffffffff 0x100000;mmc write 0 0x42000000 0x11D800 0x800;
​	
​	或直接系统起来后清空对应分区
​	dd if=/dev/zero of=/dev/mmcblk0p7 bs=1M count=1

encrypt init;encrypt cnt set 0;
status clr cnt;reset;

nova nand 128M

1).evb:
setenv ipaddr 192.168.55.111;setenv serverip 192.168.55.8;
mw.l 0x0 0xffffffff 0x50000;tftp 0x0 nvt-na51068-evb.bin;nand erase 0x40000 0x40000;nand write 0x0 0x40000 0x40000;

2).uboot:
setenv ipaddr 192.168.55.111;setenv serverip 192.168.55.8;
mw.l 0x0 0xffffffff 0x210000;tftp 0x0 u-boot.bin;nand erase 0xC0000 0x200000;nand write 0x0 0xC0000 0x200000;

3).kernel:
setenv ipaddr 192.168.55.111;setenv serverip 192.168.55.8;
mw.l 0x0 0xffffffff 0x500000;tftp 0x0 uImage.bin;nand erase 0x300000 0x460000;nand write 0x0 0x300000 0x460000;

4).rootfs:
setenv ipaddr 192.168.55.111;setenv serverip 192.168.55.8;
mw.l 0x0 0xffffffff 0x4000000;tftp 0x0 rootfs.ramdisk.bin;nand erase 0x760000 0x3200000;nand write.trimffs 0x0 0x760000 ${filesize};

5).rootfs1:
mw.l 0x0 0xffffffff 0x3000000;tftp 0x0 rootfs_1.rw.ubifs.bin;nand erase 0x3960000 0x2500000;nand write.trimffs 0x0 0x3960000 ${filesize};
	
6).app:
mw.l 0x0 0xffffffff 0x3000000;tftp 0x0 NVR_8CH.nand.ubifs.bin;nand erase 0x5E60000 0x21A0000;nand write.trimffs 0x0 0x5E60000 ${filesize};

## nova 2M+8G

主要涉及产品 nt98321/nt98323

Flash操作方法

### loader

```
setenv ipaddr 192.168.55.111;setenv serverip 192.168.55.8;
mw.l 0x1000000 0xffffffff 0x50000;tftpboot 0x1000000 LD98321A.bin;sf probe 0;sf erase 0 0x10000;sf write 0x1000000 0x0 0x10000;

mw.l 0x1000000 0xffffffff 0x50000;tftpboot 0x1000000 LD98321A_emmc.bin;sf probe 0;sf erase 0 0x10000;sf write 0x1000000 0x0 0x10000;

mw.l 0x1000000 0xffffffff 0x50000;tftpboot 0x1000000 LD98321A_emmc.bin;sf probe 0;sf erase 0 0x10000;sf write 0x1000000 0x0 0x10000;
```



### evb/FDT

```
setenv ipaddr 192.168.55.111;setenv serverip 192.168.55.8;
mw.l 0x1000000 0xffffffff 0x50000;tftpboot 0x1000000 nvt-na51068-evb.bin;sf probe 0;sf erase 0x10000 0x20000;sf write 0x1000000 0x10000 0x20000;

mw.l 0x1000000 0xffffffff 0x50000;tftpboot 0x1000000 nvt-na51068-evb_atsha_i2c.bin;sf probe 0;sf erase 0x10000 0x20000;sf write 0x1000000 0x10000 0x20000;

mw.l 0x1000000 0xffffffff 0x50000;tftpboot 0x1000000 STM_D5X_XRV4_FDT_T20110300;sf probe 0;sf erase 0x10000 0x20000;sf write 0x1000000 0x10000 0x20000;



mw.l 0x1000000 0xffffffff 0x100000;tftpboot 0x1000000 nvt-na51068-evb_emmc.bin;

mw.l 0x1000000 0xffffffff 0x100000;tftpboot 0x1000000 STM_D5X_XRV4_FDT_T111500_I2C;

mw.l 0x1000000 0xffffffff 0x100000;tftpboot 0x1000000 STM_D5X_XRV4_FDT_T20112400_EMMC;

mmc write 0x1000000 0x200 0x200;

```



### uboot

```
mw.l 0x1000000 0xffffffff 0x100000;tftpboot 0x1000000 STM_D5X_XRV4_UBOOT_T20102400;sf probe 0;sf erase 0x50000 0x60000;sf write 0x1000000 0x50000 0x60000;

mw.l 0x1000000 0xffffffff 0x100000;tftpboot 0x1000000 STM_D5X_XRV4_UBOOT_T20102400_ATSHA;sf probe 0;sf erase 0x50000 0x60000;sf write 0x1000000 0x50000 0x60000;



mw.l 0x1000000 0xffffffff 0x100000;tftpboot 0x1000000 STM_D5X_XRV4_UBOOT_T20110301;sf probe 0;sf erase 0x50000 0x60000;sf write 0x1000000 0x50000 0x60000;



mw.l 0x1000000 0xffffffff 0x100000;tftpboot 0x1000000 STM_D5X_XRV4_UBOOT_T20111200_EMMC;sf probe 0;sf erase 0x50000 0x60000;sf write 0x1000000 0x50000 0x60000;

mw.l 0x1000000 0xffffffff 0x100000;tftpboot 0x1000000 STM_D5X_XRV4_UBOOT_T20112700;

mw.l 0x1000000 0xffffffff 0x100000;tftpboot 0x1000000 STM_D5X_XRV4_UBOOT_T20112800;
mmc write 0x1000000 0x600 0x600;
```



### cvbs logo

```
setenv ipaddr 192.168.55.111;setenv serverip 192.168.55.8;
mw.l 0x1000000 0xffffffff 0x100000;tftpboot 0x1000000 logome.jpg;sf probe 0;sf erase 0xB0000 0x20000;sf write 0x1000000 0xB0000 0x20000;

mw.l 0x1000000 0xffffffff 0x100000;tftpboot 0x1000000 logome.jpg;
mmc write 0x1000000 0xc00 0x100;
```



### upgrade logo

```
mw.l 0x1000000 0xffffffff 0x100000;tftpboot 0x1000000 update.jpg;sf probe 0;sf erase 0xD0000 0x10000;sf write 0x1000000 0xD0000 0x7000;

mw.l 0x1000000 0xffffffff 0x100000;tftpboot 0x1000000 update.jpg;
mmc write 0x1000000 0xd00 0x100;
```



### vga logo

```
#mw.l 0x1000000 0xffffffff 0x100000;tftpboot 0x1000000 vga.jpg;sf probe 0;sf erase 0xD7000 0x10000;sf write 0x1000000 0xD7000 0x19000;
mw.l 0x1000000 0xffffffff 0x100000;tftpboot 0x1000000 vga.jpg;sf probe 0;sf write 0x1000000 0xD7000 0x19000;
```



### kernel

```
setenv ipaddr 192.168.11.9;setenv serverip 192.168.11.8;
mw.l 0x1000000 0xffffffff 0x500000;tftpboot 0x1000000 STM_D5X_XRV4_KERNEL_T20110300;sf probe 0;sf erase 0xF0000 0x300000;sf write 0x1000000 0xF0000 ${filesize};

mw.l 0x1000000 0xffffffff 0x500000;tftpboot 0x1000000 STM_D5X_XRV4_KERNEL_T20121600_I2C_RETRY128;

mw.l 0x1000000 0xffffffff 0x500000;tftpboot 0x1000000 STM_D5X_XRV4_KERNEL_T20121200_I2C;

mmc write 0x1000000 0xE00 0x2000;
```



### rootfs

```
setenv ipaddr 192.168.55.111;setenv serverip 192.168.55.8;
mw.l 0x1000000 0xffffffff 0x500000;tftpboot 0x1000000 STM_D5X_XRV4_ROOTFS_T20110900_004_NFS;sf probe 0;sf erase 0x3F0000 0x300000;sf write 0x1000000 0x3F0000 ${filesize};

mw.l 0x1000000 0xffffffff 0x500000;tftpboot 0x1000000 STM_D5X_XRV4_ROOTFS_T2011097;sf probe 0;sf erase 0x3F0000 0x300000;sf write 0x1000000 0x3F0000 ${filesize};

mw.l 0x1000000 0xffffffff 0x500000;tftpboot 0x1000000 STM_D5X_XRV4_ROOTFS_T20120300_NFS;

mw.l 0x1000000 0xffffffff 0x500000;tftpboot 0x1000000 STM_D5X_XRV4_ROOTFS_T20120500_NFS;

mmc write 0x1000000 0xA800 0x8000;
```



### app

```
setenv ipaddr 192.168.55.111;setenv serverip 192.168.55.8;
mw.l 0x1000000 0xffffffff 0x2000000;tftpboot 0x1000000 STM_LOCAL_0x14_T20110995;sf probe 0;sf erase 0x6F0000 0x1650000;sf write 0x1000000 0x6F0000 ${filesize};
```



### 清除参数区

擦除参数区、状态区
sf probe 0;sf erase 0x1d40000 0x40000;sf erase 0x1da0000 0x40000

### 清除状态区

sf probe 0;sf erase 0x1d70000 0x10000;sf erase 0x1dd0000 0x10000

导出参数区
setenv ipaddr 192.168.55.111;setenv serverip 192.168.55.8;
sf probe 0;sf read 0x1000000 0x1d40000 0x40000;tftpput 0x1000000 0x40000 flash_state;
sf probe 0;sf read 0x1000000 0x1da0000 0x40000;tftpput 0x1000000 0x40000 flash_state_bak;

导入参数分区
mw.l 0x1000000 0xffffffff 0x50000;tftpboot 0x1000000 flash_state;sf probe 0;sf erase 0x1d40000 0x40000;sf write 0x1000000 0x1d40000 0x40000;
mw.l 0x1000000 0xffffffff 0x50000;tftpboot 0x1000000 flash_state_bak;sf probe 0;sf erase 0x1da0000 0x40000;sf write 0x1000000 0x1da0000 0x40000;

### backup

setenv ipaddr 192.168.55.111;setenv serverip 192.168.55.8;
mw.l 0x1000000 0xffffffff 0x200000;tftpboot 0x1000000 STM_D5X_XRV4_BACKUP_T20102400;sf probe 0;sf erase 0x1E00000 0x200000;sf write 0x1000000 0x1e00000 ${filesize};

setenv ipaddr 192.168.55.111;setenv serverip 192.168.55.8;
mw.l 0x1d600000 0xffffffff 0x500000;tftpboot 0x1d600000 uImage.bin;bootm 0x1d600000

mw.l 0x1000000 0xffffffff 0x500000;tftpboot 0x1000000 STM_D5X_XRV4_BACKUP_T20112800_EMMC;

mw.l 0x1000000 0xffffffff 0x500000;tftpboot 0x1000000 STM_D5X_XRV4_BACKUP_T20112400_EMMC;

mmc write 0x1000000 0x12800 0x2000;

擦除备份系统
sf probe 0;sf erase 0x1e00000 0x200000

### 不进备份系统

encrypt init;encrypt cnt set 0;
status set crc;reset;

//导入生产测试包
sf probe 0;tftpboot 0x1000000 Firmware_X5N_V320_T200820.03_C0010_TEST
sf probe 0;sf erase 0 0x2000000;sf write 0x1000000 0 0x2000000;


镜像拷贝
dd if=/dev/mtdblock0 of=/var/run/mnt/nova_backup/loader bs=1M count=1
dd if=/dev/mtdblock1 of=/var/run/mnt/nova_backup/fdt bs=1M count=1
dd if=/dev/mtdblock2 of=/var/run/mnt/nova_backup/fdt_restore bs=1M count=1
dd if=/dev/mtdblock3 of=/var/run/mnt/nova_backup/UBOOT bs=2M count=1
dd if=/dev/mtdblock4 of=/var/run/mnt/nova_backup/env bs=2M count=1
dd if=/dev/mtdblock5 of=/var/run/mnt/nova_backup/kernel bs=2M count=3
dd if=/dev/mtdblock6 of=/var/run/mnt/nova_backup/rootfs bs=20M count=3
dd if=/dev/mtdblock7 of=/var/run/mnt/nova_backup/rootfs1 bs=20M count=3
dd if=/dev/mtdblock8 of=/var/run/mnt/nova_backup/app bs=20M count=3
dd if=/dev/mtdblock9 of=/var/run/mnt/nova_backup/all bs=20M count=10

dd if=/dev/mtd0 of=system_back/loader bs=1M count=1
dd if=/dev/mtd1 of=system_back/fdt bs=1M count=1
dd if=/dev/mtd2 of=system_back/fdt_restore bs=1M count=1
dd if=/dev/mtd3 of=system_back/UBOOT bs=2M count=1
dd if=/dev/mtd4 of=system_back/env bs=2M count=1
dd if=/dev/mtd5 of=system_back/kernel bs=2M count=3
dd if=/dev/mtd6 of=system_back/rootfs bs=20M count=3
dd if=/dev/mtd7 of=system_back/rootfs1 bs=20M count=3
dd if=/dev/mtd8 of=system_back/app bs=20M count=3
dd if=/dev/mtdblock9 of=system_back/all bs=20M count=10



a. loady 0x0 115200
b. secucrt: option->传输Ymodem->选择需要烧入的文件。
c.  nand erase 0xC0000 0x200000;nand write 0x0 0xC0000 0x200000
reset。
再进入uboot ping其他IP OK.

## nova 8G EMMC

主要涉及NT98528产品：D50_VA/951_VA/AIBOX5.0/C6D5.0/ADPRO2.0)tftp烧录方法

	拔电源重新开机按键ctrl+c停在u-boot下
	首先在u-boot下输入printenv，查看：
					ipaddr=192.168.50.17
					serverip=192.168.50.18	
	其中ipaddr为板子的IP，serverip为自己电脑的IP，如果不是则要设置：
					setenv serverip 192.168.50.18
					setenv ipaddr 192.168.50.17
	
	设置成功ping一下自己电脑的IP，如果is alive就可以开始升级，一般用tftpd32.exe工具。为保险起见，
	先升级u-boot之后的其它软件，如kernel等，没问题再升uboot。
	
	【注意写入大小，mmc write的最后一个参数必须是实际文件大小的向上取 512的整数倍大小】

### loader

​	mmc dev 2;mmc partconf 2 1 1 1;
​	mw.l 0x1000000 0xffffffff 0x100000;tftpboot 0x1000000 LD98528A.bin;
​	mmc write 0x1000000 0x0 0x200;
​	mmc dev 2;mmc partconf 2 1 1 7;
​	

### fdt

​	mw.l 0x1000000 0xffffffff 0x100000;tftpboot 0x1000000 STM_AIBOX50_FDT_T20110700;
​	mmc write 0x1000000 0x200 0x200;

### uboot

​	mw.l 0x1000000 0xffffffff 0x100000;tftpboot 0x1000000 STM_AIBOX50_UBOOT_T20110700;

​	mw.l 0x1000000 0xffffffff 0x100000;tftpboot 0x1000000 STM_A5N60_UBOOT_T20110400;

​	mmc write 0x1000000 0x600 0x800;
​	

### kernel

​	mw.l 0x1000000 0xffffffff 0x500000;tftpboot 0x1000000 uImage.bin;
​	mmc write 0x1000000 0xE00 0x2000;
​	

### rootfs

​	mw.l 0x1000000 0xffffffff 0x500000;tftpboot 0x1000000 STM_AIBOX5.0_ROOTFS_T20111400;
​	mmc write 0x1000000 0xA800 0x8000;
​	

### AppLocal

​	tftpboot 0x1000000 STM_LOCAL;
​	mmc write 0x1000000 0x14800 0x40000;
​	

### AppExt

​	tftpboot 0x1000000 STM_EXTEND;
​	mmc write 0x1000000 0x55000 0x190000
​	

### backup system

​	mw.l 0x1000000 0xffffffff 0x500000;tftpboot 0x1000000 BKSystem.bin
​	mmc write 0x1000000 0x12800 0x2000

### 擦除参数

​	mw.l 0x1000000 0xffffffff 0x1000000;mmc write 0x1000000 0x1E5800 0x8000

### 擦除状态区

​	mw.l 0x1000000 0xffffffff 0x100000;mmc write 0x1000000 0x1EE000 0x800

MBR(注：拆包软件的MBR是所有MBR_N合并的，需要拆分后一个个单独烧录)
	mw.l 0x1000000 0xffffffff 0x100000;tftpboot 0x1000000 MBR_N.bin;
	mmc write 0x1000000 0x0 0x200;		//MBR_0.bin
	mmc write 0x1000000 0x54800 0x200;	//MBR_1.bin
	mmc write 0x1000000 0x1E5000 0x200;	//MBR_2.bin
	mmc write 0x1000000 0x1ED800 0x200;	//MBR_3.bin
	mmc write 0x1000000 0x1EE800 0x200;	//MBR_4.bin
	mmc write 0x1000000 0x5EF000 0x200;	//MBR_5.bin
	mmc write 0x1000000 0x65E000 0x200;	//MBR_6.bin