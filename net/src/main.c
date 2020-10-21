#include "../include/arp.h"
#include "../include/ether.h"
#include <arpa/inet.h>
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <linux/if.h>
#include <linux/if_tun.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>


int tun_open(char *dev) {
    struct ifreq ifr;
    int fd, err;

    if ((fd = open("/dev/net/tap", O_RDWR)) == -1) {
        printf("unable to open tun/tap device: %s \n", strerror(errno));
        return fd;
    }

    memset(&ifr, 0, sizeof(ifr));
    ifr.ifr_ifru.ifru_flags = IFF_TAP | IFF_NO_PI;
    if (*dev) {
        strncpy(ifr.ifr_ifrn.ifrn_name, dev, IFNAMSIZ);
    }

    if ((err = ioctl(fd, TUNSETIFF, (void *) &ifr)) == -1) {
        close(fd);
        printf("unable to set interface for tun: %s \n", strerror(errno));
        return err;
    }
    return fd;
}


void printIpV4(unsigned int ip) {
    printf("ip: %d.%d.%d.%d \n",
           (unsigned char) ((ip >> 24) & 0xFF),
           (unsigned char) ((ip >> 16) & 0xFF),
           (unsigned char) ((ip >> 8) & 0xFF),
           (unsigned char) (ip & 0xFF));
}

int main() {
    ssize_t nbytes;
    char buf[1600];

    char *name = "tun1";

    int fd = tun_open(name);
    printf("Device %s opened\n", name);
    while (1) {
        nbytes = read(fd, buf, sizeof(buf));

        printf("Read %zd bytes from %s:\n[ ", nbytes, name);
        for (int i = 0; i < nbytes; i++) {
            printf("%d, ", buf[i]);
        }
        printf("]\n");
        struct EtherHeader *eth = (struct EtherHeader *) buf;
        printf("Ether Header:\n");
        printf("dest_mac: %d:%d:%d:%d:%d:%d\n", eth->destnationMac[0], eth->destnationMac[1], eth->destnationMac[2], eth->destnationMac[3], eth->destnationMac[4], eth->destnationMac[5]);
        printf("src_mac: %d:%d:%d:%d:%d:%d\n", eth->sourceMac[0], eth->sourceMac[1], eth->sourceMac[2], eth->sourceMac[3], eth->sourceMac[4], eth->sourceMac[5]);
        if (ntohs(eth->etherType) >= 1536) {
            printf("ether_type: %d: %s\n", ntohs(eth->etherType), getEtherTypeName(ntohs(eth->etherType)));
        }

        struct ArpHeader *arp = (ArpHeader *) eth->payload;
        if (ntohs(arp->hardwareType) == 0x1) {     // Ethernet
            assert(arp->hardwareAddressLen == 6);  // 6 for ethernet
            if (arp->protocolAddressLen == 4) {    //ipv4
                if (ntohs(arp->operation) == 0x1) {// it's a arp request
                    struct ArpIpV4 *arpv4 = (ArpIpV4 *) arp->data;
                    printIpV4(ntohl(arpv4->senderIp));
                    arp->operation = 2;

                    ssize_t result = write(fd, buf, (ssize_t) nbytes);
                    if (result == -1) {
                        printf("arp response failed: %s \n", strerror(errno));
                    }
                }
            }
        }
    }
}
