#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <linux/mii.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <linux/sockios.h>
#include <linux/types.h>
#include <netinet/in.h>

#define CFW_SET_BIT(val, n) ((val) |= (1 << n))
#define CFW_CLR_BIT(val, n) ((val) &= ~(1 << n))
#define CFW_IS_BIT_SET(val, n) ((val) & (1 << n))
#define CFW_IS_BIT_CLR(val, n) (~((val) & (1 << n)))
#define CFW_SWITCH_BIT(val, n) ((val) ^= (1 << n))
#define CFW_GET_BIT(val, n) (((val) >> n) & 1)

#define PRINT(fmt, args...) do{printf("[%s, %d]: " fmt "\n", __func__, __LINE__, ##args);}while(0);

static void Usage(char * argv)
{
    PRINT("Usage: \n");
    PRINT("%s Read phy reg [eth0/eth1]\n", argv);
    PRINT("%s Write phy reg value [eth0/eth1]\n", argv);
    PRINT("%s phy status [eth0/eth1]\n", argv);

    exit(-1);
}

static int rm_read_mii_register(int sockfd, struct ifreq *ifr, struct mii_ioctl_data *mii)
{
    if (!ifr || !mii || sockfd < 0)
        return -1;

    if (ioctl(sockfd, SIOCGMIIREG, ifr))
    {
        PRINT("SIOCGMIIREG fail %s\n", strerror(errno));
        return -1;
    }
    else
    {
        if (mii->reg_num != 0x15 && mii->reg_num != 0x17)
            PRINT("Read Success %s: phy 0x%x reg 0x%x value 0x%x\n", ifr->ifr_name, mii->phy_id, mii->reg_num, mii->val_out);

        return 0;
    }
}

static int rm_write_mii_register(int sockfd, struct ifreq *ifr, struct mii_ioctl_data *mii)
{
    if (!ifr || !mii || sockfd < 0)
        return -1;

    if (ioctl(sockfd, SIOCSMIIREG, ifr))
    {
        PRINT("SIOCSMIIREG fail %s\n", strerror(errno));
        return -1;
    }
    else
    {
        if (mii->reg_num != 0x15 && mii->reg_num != 0x17)
            PRINT("Write Success %s: phy 0x%x reg 0x%x value 0x%x\n", ifr->ifr_name, mii->phy_id, mii->reg_num, mii->val_in);

        return 0;
    }
} 

static int rm_open_mii_device(int *old_socket, struct ifreq *ifr)
{
    if (!old_socket || !ifr)

    if (*old_socket > 0)
    {
        close(*old_socket);
        *old_socket = -1;
    }

    memset(ifr, 0, sizeof(struct ifreq));
    strncpy(ifr->ifr_name, "eth0", IFNAMSIZ - 1);

    int new_socket = -1;
    new_socket = socket(PF_LOCAL, SOCK_DGRAM, 0);
    if (new_socket < 0)
    {
        PRINT("rm_open_mii_device socket fail %s\n", strerror(errno));
        return -1;
    }

    *old_socket = new_socket;
    PRINT("rm_open_mii_device socket success %d\n", new_socket);
    return 0;
}

int main(int argc, char *argv[])
{
    int sockfd = -1;

    if (argc < 3)
    {
        Usage(argv[0]);
        return -1;
    }

    struct mii_ioctl_data *mii = NULL;
    struct ifreq ifr;

    sockfd = socket(PF_LOCAL, SOCK_DGRAM, 0);
    if (sockfd < 0)
    {
        PRINT("socket fail %s\n", strerror(errno));
        return -1;
    }

#if 0
    //get phy address in smi bus
    if (ioctl(sockfd, SIOCGMIIPHY, &ifr))
    {
        PRINT("SIOCGMIIPHY fail %s\n", strerror(errno));
        close(sockfd);
        return -1;
    }
#endif

	memset(&ifr, 0, sizeof(ifr));
    strncpy(ifr.ifr_name, "eth0", IFNAMSIZ - 1);

    mii = (struct mii_ioctl_data*)&ifr.ifr_data;

    if (0 == strcasecmp(argv[1], "read"))
    {
    	if (argc > 4)
		{
			strncpy(ifr.ifr_name, argv[4], IFNAMSIZ - 1);
		}

        mii->phy_id = (uint16_t)strtoul(argv[2], NULL, 16);
        mii->reg_num = (uint16_t)strtoul(argv[3], NULL, 16);
        if (ioctl(sockfd, SIOCGMIIREG, &ifr))
        {
            PRINT("SIOCGMIIREG fail %s\n", strerror(errno));
            close(sockfd);
            return -1;
        }
        else
        {
            PRINT("Read Success %s: phy %x reg %x value 0x%x\n", ifr.ifr_name, mii->phy_id, mii->reg_num, mii->val_out);
        }


    }
    else if (0 == strcasecmp(argv[1], "write"))
    {
        if (argc < 5)
        {
            Usage(argv[0]);
        }

		if (argc > 5)
		{
			strncpy(ifr.ifr_name, argv[5], IFNAMSIZ - 1);
		}

        mii->phy_id = (uint16_t)strtoul(argv[2], NULL, 16);
        mii->reg_num = (uint16_t)strtoul(argv[3], NULL, 16);
        mii->val_in = (uint16_t)strtoul(argv[4], NULL, 16);

        if (ioctl(sockfd, SIOCSMIIREG, &ifr))
        {
            PRINT("SIOCSMIIREG fail %s\n", strerror(errno));
            close(sockfd);
            return -1;
        }
        else
        {
            PRINT("Write Success %s: phy %x reg %x value 0x%x\n", ifr.ifr_name, mii->phy_id, mii->reg_num, mii->val_in);
        }
    }
    else if (0 == strcasecmp(argv[1], "status"))
    {
        mii->phy_id = (uint16_t)strtoul(argv[2], NULL, 16);

		if (argc > 3)
		{
			strncpy(ifr.ifr_name, argv[3], IFNAMSIZ - 1);
		}


        if (ioctl(sockfd, SIOCGIFFLAGS, &ifr))
        {
            PRINT("SIOCGIFFLAGS fail %s\n", strerror(errno));
            close(sockfd);
            return -1;
        }
        else
        {
            PRINT("SIOCGIFFLAGS Success %s: phy %x status 0x%x\n", ifr.ifr_name, mii->phy_id, ifr.ifr_flags);
        }
    }
    else if (0 == strcasecmp(argv[1], "test"))
    {
        uint16_t reg_num = 0;
        uint16_t phy_id = (uint16_t)strtoul(argv[2], NULL, 16);

        PRINT("common register\n");
        for (reg_num = 0; reg_num < 0x1F; reg_num++)
        {
            mii->reg_num = reg_num;
            mii->phy_id = phy_id;
            rm_read_mii_register(sockfd, &ifr, mii);
            usleep(1000*10);
        }

        PRINT("expansion register\n");
        static uint8_t array_expansion[] = {0x00, 0x04, 0x05, 0x06, 0x08, 0x70};
        for (reg_num = 0; reg_num < sizeof(array_expansion)/sizeof(uint8_t); reg_num++)
        {
            mii->val_in = (0x0F << 8) | array_expansion[reg_num];
            mii->reg_num = 0x17;
            mii->phy_id = phy_id;
            rm_write_mii_register(sockfd, &ifr, mii);

            mii->reg_num = 0x15;
            mii->phy_id = phy_id;
            rm_read_mii_register(sockfd, &ifr, mii);
            PRINT("expansion register: phy 0x%x reg 0x%x value 0x%x\n", mii->phy_id, array_expansion[reg_num], mii->val_out);
            usleep(1000*10);
        }

        PRINT("wol register\n");
        for (reg_num = 0x80; reg_num < 0x95; reg_num++)
        {
            mii->val_in = (0x0E << 8) | reg_num;
            mii->reg_num = 0x17;
            mii->phy_id = phy_id;
            rm_write_mii_register(sockfd, &ifr, mii);

            mii->reg_num = 0x15;
            mii->phy_id = phy_id;
            rm_read_mii_register(sockfd, &ifr, mii);
            PRINT("wol register: phy 0x%x reg 0x%x value 0x%x\n", mii->phy_id, reg_num, mii->val_out);
            usleep(1000*10);
        }

        PRINT("top misc register\n");
        static uint8_t array_misc[] = {0x00, 0x06, 0x07, 0x0C};
        for (reg_num = 0; reg_num < sizeof(array_misc)/sizeof(uint8_t); reg_num++)
        {
            mii->val_in = (0x0D << 8) | array_misc[reg_num];
            mii->reg_num = 0x17;
            mii->phy_id = phy_id;
            rm_write_mii_register(sockfd, &ifr, mii);

            mii->reg_num = 0x15;
            mii->phy_id = phy_id;
            rm_read_mii_register(sockfd, &ifr, mii);
            PRINT("top misc register: phy 0x%x reg 0x%x value 0x%x\n", mii->phy_id, array_misc[reg_num], mii->val_out);
            usleep(1000*10);
        }
    }
    else if (0 == strcasecmp(argv[1], "check"))
    {
        uint16_t phy_id = (uint16_t)strtoul(argv[2], NULL, 16);
        uint16_t error_event = 0;
        uint16_t link_status = 0;
        uint16_t reset_status = 0;
        int ret = 0;
        static int ping_count = 0;
        static int err_count = 0;

        PRINT("phy_id (0x%x)\n", phy_id);

        for(;;)
        {
            sleep(1);

            //ping
            ping_count++;
            if (ping_count == 60*10)
            {
                PRINT("\r\nrun ping cmd\r\n");
                system("ping 192.168.1.1 &");
            }

            //check 0x01 addr
            mii->reg_num = 0x01;
            mii->phy_id = phy_id;
            ret = rm_read_mii_register(sockfd, &ifr, mii);
            if (ret != 0)
            {
                rm_open_mii_device(&sockfd, &ifr);
                continue;
            }
            link_status = mii->val_out;
            PRINT("link_status(0x%x)\n", link_status);
            if (CFW_IS_BIT_SET(link_status, 2))
            {
                //check 0x13 addr
                mii->reg_num = 0x13;
                mii->phy_id = phy_id;
                ret = rm_read_mii_register(sockfd, &ifr, mii);
                if (ret != 0)
                {
                    rm_open_mii_device(&sockfd, &ifr);
                    continue;
                }
                error_event = mii->val_out;
                PRINT("error_event(0x%x)\n", error_event);
                if (error_event > 0)
                {
                    sleep(1);
                    mii->reg_num = 0x00;
                    mii->phy_id = phy_id;
                    ret = rm_read_mii_register(sockfd, &ifr, mii);
                    if (ret != 0)
                    {
                        rm_open_mii_device(&sockfd, &ifr);
                        continue;
                    }
                    reset_status = mii->val_out;

                    CFW_SET_BIT(reset_status, 9);
                    PRINT("val_out(0x%x) reset_status(0x%x) err_count=%d\n", mii->val_out, reset_status, err_count++);

                    //restart auto-negotiation
                    mii->reg_num = 0x00;
                    mii->phy_id = phy_id;
                    mii->val_in = reset_status;
                    ret = rm_write_mii_register(sockfd, &ifr, mii);
                    if (ret != 0)
                    {
                        rm_open_mii_device(&sockfd, &ifr);
                        continue;
                    }
                    sleep(1);

                    //reset phy
                    mii->reg_num = 0x00;
                    mii->phy_id = phy_id;
                    mii->val_in = 0x8000;
                    ret = rm_write_mii_register(sockfd, &ifr, mii);
                    if (ret != 0)
                    {
                        rm_open_mii_device(&sockfd, &ifr);
                        continue;
                    }
                    sleep(1);
                }
            }
        }
    }
    else
    {
        Usage(argv[0]);
    }

    close (sockfd);
    return 0;
}
