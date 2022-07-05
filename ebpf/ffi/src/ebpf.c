#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <ifaddrs.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <sys/uio.h>
#include <net/bpf.h>
#include <net/if.h>
#include <net/if_dl.h>
#include <net/if_arp.h>
#include <net/ethernet.h>
#include <netinet/in.h>
#include <arpa/inet.h>

int  openBpf(void);
void closeBpf(int fd);
int  setupBpf(int fd, const char *ifname);

void makeEther(struct ether_header* ether);
void makeArp(const u_char* restrict srcHardwareAddress,
        const in_addr_t* srcProtocolAddress,
        const in_addr_t* dstProtocolAddress,
        struct arphdr* restrict arp);

static int
getAddress(const char* restrict ifname,
        u_char* restrict hardwareAddress, in_addr_t* restrict protocolAddress);

int
openBpf(void)
{
    return open("/dev/bpf", O_RDWR);
}


void
closeBpf(int fd)
{
    close(fd);
}


int
setupBpf(int fd, const char *ifname)
{
    struct ifreq request;
    u_int tstamp;
    u_int type;

    strlcpy(request.ifr_name, ifname, sizeof(request.ifr_name) - 1);
    if (ioctl(fd, BIOCSETIF, &request) < 0) {
        perror("BIOCSETIF failed: ");
        return -1;
    }

    if (ioctl(fd, BIOCGDLT, &type) < 0) {
        perror("BIOCGDLT failed: ");
        return -1;
    }
    if (type != DLT_EN10MB) {
        printf("unsupported datalink type\n");
        return -1;
    }

    tstamp = BPF_T_NANOTIME;
    if (ioctl(fd, BIOCSTSTAMP, &tstamp) < 0) {
        perror("BIOCSTSTAMP faild: ");
        return -1;
    }

    return 0;
}


static int
getAddress(const char* restrict ifname,
        u_char* restrict hardwareAddress, in_addr_t* restrict protocolAddress)
{
    struct ifaddrs *addrList, *addr;
    struct sockaddr_dl *dlAddr = NULL;
    struct sockaddr_in *srcAddr = NULL;
    int ret = 0;

    if (getifaddrs(&addrList) < 0) {
        return -1;
    }
    addr = addrList;
    while (addr && (!dlAddr || !srcAddr)) {
        if (addr->ifa_addr->sa_family == AF_LINK) {
            if (strcmp(ifname, addr->ifa_name) == 0) {
                dlAddr = (struct sockaddr_dl*)addr->ifa_addr;
            }
        }
        else if (addr->ifa_addr->sa_family == AF_INET) {
            if (strcmp(ifname, addr->ifa_name) == 0) {
                srcAddr = (struct sockaddr_in*)addr->ifa_addr;
            }
        }
        addr = addr->ifa_next;
    }

    if (dlAddr && srcAddr) {
        memcpy(hardwareAddress, LLADDR(dlAddr), dlAddr->sdl_alen);
        memcpy(protocolAddress, &srcAddr->sin_addr, sizeof(in_addr_t));
    } else {
        printf("cannot find link layer address for %s", ifname);
        ret = -1;
    }

    freeifaddrs(addrList);

    return ret;
}


void
makeEther(struct ether_header* ether)
{
    static const u_char etherBroadcast[ETHER_ADDR_LEN] = {
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff
    };

    memset(&ether->ether_shost, 0, ETHER_ADDR_LEN);
    memcpy(&ether->ether_dhost, etherBroadcast, ETHER_ADDR_LEN);
    ether->ether_type = htons(ETHERTYPE_ARP);
}


void
makeArp(const u_char* restrict srcHardwareAddress,
        const in_addr_t* srcProtocolAddress,
        const in_addr_t* dstProtocolAddress,
        struct arphdr* restrict arp)
{
    arp->ar_hrd = htons(ARPHRD_ETHER);
    arp->ar_pro = htons(ETHERTYPE_IP);
    arp->ar_hln = ETHER_ADDR_LEN;
    arp->ar_pln = sizeof(in_addr_t);
    arp->ar_op = htons(ARPOP_REQUEST);
    memcpy(ar_sha(arp), srcHardwareAddress, ETHER_ADDR_LEN);
    memcpy(ar_spa(arp), srcProtocolAddress, sizeof(in_addr_t));
    memset(ar_tha(arp), 0, ETHER_ADDR_LEN);
    memcpy(ar_tpa(arp), dstProtocolAddress, sizeof(in_addr_t));
}


int
main(int argc, char **argv)
{
    struct ether_header ether;
    u_char srcHardwareAddress[ETHER_ADDR_LEN];
    in_addr_t srcProtocolAddress;
    struct in_addr dstProtocolAddress;
    int fdBpf = -1;
    u_char arp[sizeof(struct arphdr) +
        2 * (ETHER_ADDR_LEN + sizeof(in_addr_t))];

    if (argc != 3) {
        printf("usage: %s <interface> <target address>\n", argv[0]);
        return 1;
    }

    if ((fdBpf = openBpf()) < 0) {
        printf("BPF cannot be opened.\n");
        return 1;
    }

    if (setupBpf(fdBpf, argv[1]) < 0) {
        close(fdBpf);
        return 1;
    }

    getAddress(argv[1], srcHardwareAddress, &srcProtocolAddress);
    inet_pton(AF_INET, argv[2], &dstProtocolAddress);

    makeEther(&ether);
    makeArp(srcHardwareAddress, &srcProtocolAddress, &dstProtocolAddress.s_addr,
                (struct arphdr*)arp);

    /* write packet */
    {
        struct iovec  writeVec[2];

        writeVec[0].iov_base = &ether;
        writeVec[0].iov_len = sizeof(ether);
        writeVec[1].iov_base = arp;
        writeVec[1].iov_len = sizeof(arp);
        writev(fdBpf, writeVec, 2);
    }

    closeBpf(fdBpf);
    return 0;
}

