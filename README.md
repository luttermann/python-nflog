# python-nflog
Python module to read from nflog.

### Usage

This is a basic example, the **nf_callback_packet_handler** obviously needs to be filld with something that actually does someting with the packet.

```python

def nf_callback_packet_handler(
      indev,        # (int) Inbound network device number
      outdev,       # (int) Outbound network device number
      ifname,       # (str) Network device name
      proto,        # (int) Ethertype, see net/ethernet.h
      payload_len,  # (int) Length of payload
      payload,      # (bytes) Payload
      hw_hdr_len,   # (int) Length of hardware link-layer header
      hw_hdr,       # (bytes) Hardware header
    ):
  pass

import nflog
nflog.setgroup(1) 

nflog.setcb( nf_callback_packet_handler )
nflog.start()
n = nflog.handle()
```

Too make all the above work, you also have to have setup nflog rules in your packetfilter! And use the right group, once started the group can not be changed.

### Install
This packet depends on **libnetfilter_log**, so make sure that it is installed.
Building and installing, is just normal procedure:
```sh
python setup.py build
python setup.py install
```

