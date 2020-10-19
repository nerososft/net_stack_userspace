#include "../include/ether.h"
#include <arpa/inet.h>
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
        printf("unable to open tun/tap device\n");
        return fd;
    }

    memset(&ifr, 0, sizeof(ifr));
    ifr.ifr_ifru.ifru_flags = IFF_TAP | IFF_NO_PI;
    if (*dev) {
        strncpy(ifr.ifr_ifrn.ifrn_name, dev, IFNAMSIZ);
    }

    if ((err = ioctl(fd, TUNSETIFF, (void *) &ifr)) == -1) {
        close(fd);
        printf("unable to set interface for tun\n");
        return err;
    }
    return fd;
}

int main() {
    printf("works\n");
    ssize_t nbytes;
    char buf[1600];

    int fd = tun_open("tun0");
    printf("Device tun0 opened\n");
    while (1) {
        nbytes = read(fd, buf, sizeof(buf));
        printf("Read %zd bytes from tun0:\n[ ", nbytes);
        for (int i = 0; i < nbytes; i++) {
            printf("%d, ", buf[i]);
        }
        printf("]\n");
        struct eth_hdr *hdr = (struct eth_hdr *) buf;
        printf("Ether Header:\n");
        printf("dest_mac: %d:%d:%d:%d:%d:%d\n", hdr->dest_mac[0], hdr->dest_mac[1], hdr->dest_mac[2], hdr->dest_mac[3], hdr->dest_mac[4], hdr->dest_mac[5]);
        printf("src_mac: %d:%d:%d:%d:%d:%d\n", hdr->src_mac[0], hdr->src_mac[1], hdr->src_mac[2], hdr->src_mac[3], hdr->src_mac[4], hdr->src_mac[5]);
        if (hdr->ether_type >= 1536) {
            printf("ether_type: %d: %s\n", ntohs(hdr->ether_type), getEtherTypeName(ntohs(hdr->ether_type)));
        }
    }
}
