# footunnel - a Foo-over-UDP tunnel broker and client

footunnel takes care to update the brokers tunnel configuration if the clients public IP orchanges.

## Limitations of the proof of concept
* only a single client is supported
* no UDP port changes are recognized... yet! ;)

## Requirements for using FoU
Foo-over-UDP has become a the mainline Linux kernel with version 3.18.

More info about Foo-over-UDP: https://lwn.net/Articles/614348/

## Building broker and client
```
git clone https://github.com/justusbeyer/footunnel.git
cd footunnel
make
```

## Broker Setup
Please replace 10.0.0.1 with your broker's tunnel IP and <BROKER-IP> with it's public IP.
```
# Listen for incoming FoU packets on port 42424 
sudo ip fou add port 42424 ipproto 4

# Create a new FoU tunnel
sudo ip link add name fou0 type ipip \
  remote 1.2.3.4 local <BROKER-IP> \
  encap fou encap-sport 42424 encap-dport 42424

# Enable the tunnel
sudo ip link set fou0 up mtu 1472

# Assign a tunnel IP address
sudo ip addr add 10.0.0.1/30 dev fou0

# Run footunneld to keep track of the peer's IP (SECRET is a shared secret or identifier)
sudo ./broker/footunneld -v fou0 SECRET
```

## Client setup
```
# Listen for incoming FoU packets on port 42424
sudo ip fou add port 42424 ipproto 4

# Create a new FoU tunnel
sudo ip link add name fou0 type ipip \
  remote <BROKER-IP> local <CLIENT-IP> \
  encap fou encap-sport 42424 encap-dport 42424

# Enable the tunnel
sudo ip link set fou0 up mtu 1472

# Assign a tunnel IP address
sudo ip addr add 10.0.0.2/30 dev fou0

# Make outgoing heartbeats from footunnel appear from the same UDP port as the FoU packets to make them undergo the same NAT port replacement so the broker can derive the client's FoU UDP port from the heartbeat packets.
sudo iptables -t nat -I POSTROUTING -o <OUTGOING-INTERFACE> -p udp --sport 42425 -j SNAT --to-source <CLIENT-IP>:42424

# Use footunnelc to send heartbeats to the broker to adjust its tunnel configuration to IP changes
./client/footunnelc <BROKER-IP> SECRET
```
