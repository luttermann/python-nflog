import nflog
import select

def nfprint(
        indev,
        outdev,
        ifname,
        proto,
        payload_len,
        payload,
        hwll_hdr_len,
        hwll_hdr
        ):
    print('Interface: {}'.format(ifname))
    print("Ethertype: 0x{:04x}".format(proto))
    print('Hw Hdr size: {}'.format(hwll_hdr_len))
    print('Hw Hdr:')
    i = 0
    end = ' '
    for c in hwll_hdr:
        if i < 7 or (i > 7 and i < 15):
            end = ' '
            i = i + 1
        elif i == 15:
            end = '\n'
            i = 0
        else:
            end = '  '
            i = i + 1
        print("{:02x}".format(int(c)), end=end)
    print()
    print('Pkt size: {}'.format(payload_len))
    print('Pkt:')
    i = 0
    end = ' '
    for c in payload:
        if i < 7 or (i > 7 and i < 15):
            end = ' '
            i = i + 1
        elif i == 15:
            end = '\n'
            i = 0
        else:
            end = '  '
            i = i + 1
        print("{:02x}".format(int(c)), end=end)
    print('\n')

nflog.setgroup(1) 

nflog.setcb(nfprint)
nflog.start()

timeout = 5000
fd = nflog.getfd()
poll_handle = select.poll()
poll_handle.register(fd, select.POLLIN)
import time
t0 = time.time()

while time.time() < (t0 + 600):
    plist = poll_handle.poll(timeout)
    if len(plist) > 0:
        nflog.handle()
