import nflog
import select

def nfprint(*args):
    print('Interface: {}'.format(args[1]))
    print("Ethertype: 0x{:04x}".format(args[2]))
    print('Pkt size: {}'.format(args[3]))
    print('Pkt:')
    i = 0
    end = ' '
    for c in args[4]:
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

plist = poll_handle.poll(timeout)

if len(plist) > 0:
    nflog.handle()
else:
    print('Nothing was recivede before {}ms'.format(timeout))
