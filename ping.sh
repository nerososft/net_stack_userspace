#!/bin/bash

# mknod /dev/net/tap c 10 200
sudo ip tuntap add mode tun dev tun1
ip addr add 11.0.0.0/24 dev tun1  # give it an ip
ip link set dev tun1 up  # bring the if up
ip route get 11.0.0.3  # check that packets to 10.0.0.x are going through tun0
ping 11.0.0.3  # leave this running in another shell to be able to see the effect of the next example