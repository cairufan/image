# rootfs杂项记录

## 前言

- 记录常见的rootfs知识点及疑难问题

### linux启动顺序

- 内核 -> init -> /etc/inittab -> /etc/init.d/rcS -> /etc/profile

### /etc/inittab解读

- ::sysinit:/etc/init.d/rcS