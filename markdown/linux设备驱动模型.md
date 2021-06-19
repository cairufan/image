# linux设备驱动模型

## 1. 前言

- 在日益增多的设备管理中，为了统一实现和维护，同时简化驱动程序编写，在2.6版本中正式引入设备驱动模型
  
- 设备驱动模型旨在为设备驱动进行分层，分类，组织。降低设备多样性带来的Linux驱动开发的复杂度，以及设备热插拔处理，电源管理等
  
## 2. 相关系统模块概述

### 2.1 sysfs 文件系统

- 提供文件系统接口实现与用户空间的程序之间进行沟通和互动，这种文件形式的接口也让用户空间和内核空间的数据对象交互成为可能
  
- 它实际在内核空间创造了一颗独立的VFS树，主要用来沟通系统中总线，设备与驱动，同时向用户空间提供接口与展示系统中各种设备的拓展视图，提供实际文件系统的挂载点

### 2.2 vfs(Virtual Filesystem Switch)

- VFS是一种软件机制，也许称它为Linux的文件系统管理者更确切点，与它相关的数据结构只存在于物理内存当中。所以在每次系统初始化期间，Linux都首先要在内存当中构造一棵VFS的目录树，实际上便是在内存中建立相应的数据结构

### 2.3 devtmpfs文件系统

- 在Linux的早期，/dev目录下的设备节点需要用mknod命令手动添加，现在通过devtmpfs文件系统，就可以在device_register注册设备时自动向/dev目录添加设备节点，该节点的名字就是dev->init_name

- 关于devtmpfs文件系统，它是内核连理的另一棵独立的VFS树，最终挂载到用户课件的/dev目录之上，在内核中devtmpfs主要用来动态生成设备节点
  
## 3. Linux设备驱动模型主要构架范畴

### 3.1 kobject和kset

- 如果将Linux设备模型必要成一座大厦，那么kobject和kset就是构成这座大厦内部的钢筋以及若干钢筋构建的钢架结构，再由若干的他们构成了整个大厦内部的表现形式，设备驱动中的bus，device和driver已经是整座大厦向外界展示的那部分了，所以程序员们主要是和后三者打交道

### 3.2 总线(BUS)

- 驱动模型中的总线，即可以是物理实际总线（比如PCI总线和I2C总线等）的抽象，也可以是由于驱动模型架构需要而产生的虚拟"平台"总线，因为一个符合linux驱动模型的设备和驱动必须挂靠在一根总线上，无论它是实际存在的总线还是系统虚拟出的总线

- 内核为总线对象定义的数据结构是bug_type，其中struct subsys_private *p表示一个用来管理其上设备和驱动的数据结构，其完整定义如下(drivers\base\base.h)
![20210529160421](https://cdn.jsdelivr.net/gh/cairufan/image@main//picture/20210529160421.png)

#### 3.2.1 结构体struct subsys_private主要成员介绍

----

struct kset subsys：
表示该bus所在的子系统，在内核中通过bus_register注册进系统的bus所在的kset都将指向bus_kset，换句话说bus_kset是系统中所有bus内核对象的容器，而新注册的bus本身也是一个kset型对象

struct kset *drivers_kset:
表示该bus上所有驱动的一个集合

struct kset *devices_kset:
表示该bus上所有设备的一个集合

struct klist klist_devices:
表示该bus上所有设备的链表

unsigned int drivers_autoprobe:1:
表示当向系统中某一总线中注册某一设备或者驱动的时候，是否进行设备与驱动的绑定操作

下图展示了个总线对象所衍生出来的拓扑关系，这种拓扑关系主要通过bus_type中的struct subsys_private *p成员来体现；在这个成员中，struct kset subsys标识了系统中当前总线对象与bus_kset间的隶属关系，而struct kset *drivers_kset和struct kset *devices_kset则是在想系统注册当前新总线时动态生成的容纳该总线上的所有驱动与设备的kset，与此对应，两个klist成员则以链表的形式将该总线上的所有驱动与设备链接到了一起

bus，dev，drv的层次关系图

![20210529164824](https://cdn.jsdelivr.net/gh/cairufan/image@main//picture/20210529164824.png)

----

### 3.2.2 Linux内核中针对总线的一些主要操作

- buses_init: 实现如下图所示(drivers\base\bus.c)
![20210529163318](https://cdn.jsdelivr.net/gh/cairufan/image@main//picture/20210529163318.png)

----

1. 函数将创建一个名称为"bus"的kset并将其加入到sysfs文件系统树种，函数中的bus_uevent_ops定义了当"bus"这个kset中有状态变化时，用来通知用户空间uevent消息的操作集。这个操作集只定义了一个filter函数，意味着当"bus"kset中发生状态变化时，会通过bus_uevent_ops中的filter函数先行处理，以决定是否通知用户态空间。同时filter会判断kobj对象类型是不是总线类型，如果不是uevent消息将不会发送到用户空间，所以bus_uevent_ops使得bus_kset只用来发送bus类型的内核对象产生的uevent消息

2. buses_init将在sysfs文件系统的根目录下建立一个"bus"目录，在用户空间看来，就是/sys/bus。buses_init函数创建的"bus"总线将是系统中所有后续注册总线的祖先

----

- bus_register：实现如下图所示(drivers\base\bus.c)

![20210529164023](https://cdn.jsdelivr.net/gh/cairufan/image@main//picture/20210529164023.png)

----

1. 例如总线为bus1，函数首先会将一个kset对象将被产生出来并且加入到sysfs文件系统中，该kset的parent内核对象为buses_init函数中所产生的bus_kset

2. 其次bus_regiset通过调用kset_create_and_add函数产生连接到bus1上的device_kset和driver_kset两个集合，对应到sysfs文件系统，将会在bus1目录下产生两个新的目录"devices"和"drivers"

3. 最后为了让用户空间看到或者重新配置bus1上的某些属性值，bus_register调用bus_create_file函数为bus1产生一些属性文件，这些属性文件也将位于/sys/bus/bus1目录之下，属性文件实际上向用户空间提供了一种接口，使得用户程序可以通过文件的方式来显示某一内核对象的属性或者重新配置这一属性

图为通过bus_regiseter向系统注册一根总线bus1
![20210529164542](https://cdn.jsdelivr.net/gh/cairufan/image@main//picture/20210529164542.png)

----

#### 3.2.3 总线的属性

- 总线属性代表着该总线特有的信息和配置，如果通过sysfs文件系统为总线生成属性文件，那么用户空间的程序可以通过该文件接口的方式很容易地显示或者更改该总线的属性

- 根据实际需要，可以为总线创建不止一个属性文件，每个文件代表该总线的一个或一组属性信息

### 3.3 设备(device)

- 设备代表真实的、具体的物理器件

- 系统中的每个设备都是一个struct device对象，内核为容纳所有这些设备定义了一个kset ---- deveices_kset，作为系统中所有struct device类型内核对象的容器

- 同时，内核将系统中的设备分为两大类，block和char。每类对应一个内核对象，分别为sysfs_dev_block_kobj和sysfs_dev_char_kobj，自然地这些内核对象也在sysfs文件树中占有对应的入口点

- 在linux系统初始化期间由devices_init来完成，这个函数反映到/sys文件目录下，就是生成了/sys/device、/sys/dev、/sys/dev/block和/sys/dev/char
  
- 设备在内核中的数据结构为struct device，该类型的示例是对具体设备的一个抽象，结构体如下图所示(include\linux\device.h)
  
![20210529170415](https://cdn.jsdelivr.net/gh/cairufan/image@main//picture/20210529170415.png)

#### 3.3.1 结构体struct device主要成员介绍

----

struct device *parent:
当前设备的父设备

struct device_private *p:
指向该设备的驱动相关的数据

struct kobject kobj:
代表struct device的内核对象

const char *init_name:
设备对象的名称，在将该设备对象加入到系统中时，内核会把init_name设置成kobj成员的名称，后者在sysfs中表现为一个目录

struct bus_type	*bus:
设备所在的总线对象指针

struct device_driver *driver:
用以表示当前设备是否已经与它的driver进行了绑定，如果该值为NULL，说明当前设备还没有找到它的driver

----

### 3.4 驱动(driver)

- 驱动代表着操作设备的方式和流程

- Linux设备驱动按加载时序分类可分为两种类型：内核模块驱动文件和KO模块驱动文件  
内核模块驱动文件：是内核文件(uImage)不可分割的一部分，它们之间是一个整体。在内核启动是（开机时），直接执行内核模块驱动文件，此情况，适应于开机时必须启动的设备  
KO模块驱动文件：是独立的一部分，可单独的加载与卸掉。一般用于开机之后，后续加载运行的设备

- Linux设备驱动按操作单位分类可分为三种类型:字符设备驱动、块设备驱动、网络设备驱动  
字符设备：以字节为单位进行的，典型的如LCD、串口、鼠标等  
块设备：块设备被软件操作时是以块为单位的，设备的块大小是设备本身设计时定义好的，软件是不能去更改的，不同设备的块大小可以不一样。常见的块设备都是存储设备，如：硬盘、NandFlash、SD等  
网络设备：专为网卡设计的驱动模型，linux中网络设备驱动主要目的是为了支持API中socket相关的那些函数工作

- 内核为驱动对象定义的数据结构是struct device_driver，结构体如下图所示(include\linux\device.h)

![20210529172832](https://cdn.jsdelivr.net/gh/cairufan/image@main//picture/20210529172832.png)

#### 3.4.1 结构体struct device_driver主要成员介绍

----

const char *name：驱动的名称

struct bus_type *bus：驱动所属的总线

struct module *owner：驱动所在的内核模块

int (*probe) (struct device *dev)：驱动程序所定义的探测函数，当在总线bus中将该驱动与对应的设备进行绑定时，内核会首先调用bus中的probe函数（如果该bus实现了probe函数），如果bus没有实现自己的probe函数，那么内核会调用驱动程序中实现的probe函数

int (*remove) (struct device *dev)：驱动程序所定义的卸载函数，当调用driver_unregister从系统中卸载一个驱动对象时，内核会首先调用bus中的remove函数（如果该bus实现了remove函数），如果bus没有实现自己的remove函数，那么内核会调用驱动程序中实现的remove函数

----

### 3.5 类(class)

- 相对于设备device，class是一种更高层次的抽象，用于对设备进行功能上的划分，有时候也被称为设备类。Linux的设备模型引入类，是将其用来作为具有同类型功能设备的一个容器

- 几乎所有的类都在sysfs中的/sys/class下出现，例如所有的网络接口可在/sys/class/net下发现，不管接口类型，输入设备科在/sys/class/input下，以及串行设备在/sys/class/tty，块设备在/sys/class/block下

- 内核为类定义的数据结构是struct class，结构体如下图所示(include\linux\device.h)

![20210529174404](https://cdn.jsdelivr.net/gh/cairufan/image@main//picture/20210529174404.png)

#### 3.5.1 结构体struct class主要成员介绍

----

const char *name：类的名称

struct module *onwer：拥有该类的模块的指针

struct class_attribute *class_attrs：类的属性

struct device_attribute *dev_attrs：设备的属性

struct kobject *dev_kobj：代表当前类中设备的内核对象

struct subsys_private *p：类的私有数据区，用于处理类的子系统及其所包含的设备链表

----

### 3.6 设备与驱动的绑定

- 这里的绑定，简单地说就是将一个设备与能控制它的驱动程序结合到一起的行为。两个内核对象间的结合自然是靠各自背后的数据结构中的某些成员来完成

- 总线在设备与驱动绑定的过程中发挥着核心作用：总线相关的代码屏蔽了大量底层琐碎的技术细节，为驱动程序员们提供了一组使用友好的外在接口，从而简化了驱动程序开发工作

- 在总线上发送的两类事件将导致设备与驱动绑定行为的发送，一是通过device_register函数向某一bus上注册一设备，这种情况下内核除了将该设备加入到bus上的设备链表的尾端，同时会试图将此设备与总线上的所有驱动对象进行绑定操作；二是通过driver_register将某一驱动注册到其所属的bus上，内核此时除了将驱动对象加入到bus的所有驱动对象构成的链表的尾部，也会试图将该驱动与其上的所有设备进行绑定操作

- 例如一个设备对象还没有和它的驱动程序绑定，此时需要遍历dev所在总线dev->bus上挂载的所有驱动程序对象，若设备对象dev已经和它的驱动程序进行了绑定，这种情况下只需调用device_bind_driver在sysfs文件树种建立dev与其驱动程序之间的互联关系；如果当前drv对象所在的总线定义了match方式，那么就调用它来进行是否匹配的判断，否则匹配时直接返回成功；匹配成功后，将调用driver_probe_device将drv和dev进行绑定，核心函数为really_probe，函数首先将当前驱动程序对象drv赋值给dev->driver，然后，如果dev->bus->probe不为空，即dev所在的总线定义了probe方法，则调用之，否则调用dev对象本身的dev->probe(dev)，这种设计机制给驱动程序提供了一个探测硬件的机会，即在其probe函数中作出判断；当前的设备是不是自己所支持的，以及当前设备是否处于工作状态等


### 3.7 总线，设备，驱动三者之间的联系

- 设备要让系统感知自己的存在，设备需要向总线注册自己；同样地，驱动要让系统感知自己的存在，也需要向总线注册自己

- 设备会向总线提出自己对驱动的条件（最简单的也是最精确的就是指定对方的名字了），而驱动也会向总线告知自己能够支持的设备的条件（一般是型号ID等，最简单的也可以是设备的名字）

- 设备在注册的时候，总线就会遍历注册在它上面的驱动，找到最适合这个设备的驱动，然后填入设备的结构成员中；驱动注册的时候，总线也会遍历注册在其之上的设备，找到其支持的设备（可以是多个，驱动和设备的关系是1:N），并将设备填入驱动的支持列表中。我们称总线这个牵线的行为是match；总线在匹配设备和驱动之后，真实的物理设备此时是否正常还不一定，因此驱动需要探测这个设备是否正常。我们称这个行为为probe

- 下图为bus、device、driver的整体框架关系图

![20210529183905](https://cdn.jsdelivr.net/gh/cairufan/image@main//picture/20210529183905.png)

## 4 内核中的gpiolib模块

### 4.1 概述

- 很多硬件都会用到GPIO及其复用，如果同一个GPIO被两个区的同时控制了，就会出现BUG和混乱；所以内核中提供gpiolib来统一管理系统中所有GPIO，gpiolib本身属于驱动框架的一部分

### 4.2 gpiolib主要文件及其主要接口

----
driver/gpio/gpiolib  
gpiochip_add_data: 是框架开出来的接口，用于向内核注册我们的gpiolib, 添加一个新的gpio描述符  
gpiod_request：是框架开出来的接口，驱动中想要使用某一个gpio，就必须先调用此接口来想内核的gpiolib申请资源，得到允许后才可以去使用这个gpio  
gpiod_free: 对应gpiod_request，用来释放申请后用完了的gpio  
gpiod_set_value/gpiod_get_value: 用来设置/获取IO口值  
gpiod_direction_input/gpiod_direction_output: 接口用来设置gpio的输入/输出模式

----

### 4.3 使用gpiolib完成gpio驱动

- 使用gpio_request申请要使用的一个GPIO

- gpio_direction_input/gpio_direction_output 设置输入/输出模式

- 设置输出值—gpio_set_value；获取IO口值—gpio_get_value

示例代码截图如下

![20210529175613](https://cdn.jsdelivr.net/gh/cairufan/image@main//picture/20210529175613.png)