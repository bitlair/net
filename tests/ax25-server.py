from net.family import ax25
import sys

def tests():
    call = '\x9c\x98`\x9a\xb4@\x10"E'
    print ax25.null_address()
    data = ax25.aton('NL0MZ-8')
    print repr(data)
    print data == call
    print repr(ax25.ntoa(call))
    for c in ['NL1MZ', 'NL1MZ-8', 'NL1MAZ', 'NL1MAZ-1', 'NL0MZ08', 'NL0MZ-99']:
        try:
            print c,
            print ax25.validate(ax25.aton(c))
        except ax25.error, e:
            print 'ERROR', e

def hello_bbs(call):
    print 'sock = ax25.socket()'
    sock = ax25.socket()
    print 'sock', sock
    print 'sock.bind("%s")' % (call,)
    sock.bind(call)
    print 'sock.listen(10)'
    sock.listen(10)
    while True:
        call, client = sock.accept()
        print 'new client', call

        print >>client, 'Hi thar, this is packet radio from Python'
        client.close()
     
if __name__ == '__main__':
    if len(sys.argv) != 2:
        print '%s <call>' % (sys.argv[0],)
    else:
        tests()
        hello_bbs(sys.argv[1])

