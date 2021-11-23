SATA速度确认方法



cat /sys/class/ata_link/link1/sata_spd_limit

~ # cat /sys/class/ata_link/link1/sata_spd
<unknown>
~ # cat /sys/class/ata_link/link1/sata_spd_limit 
1.5 Gbps