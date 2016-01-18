import nflog

def nfprint(*args):
    print(args[1])
    print("{:x}".format(args[2]))
    print(args[3])
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
    print()

nflog.setgroup(1) 

nflog.setcb(nfprint)
nflog.start()
n = nflog.handle() 


print("Recv {} bytes in internal buffer.".format(n))
