from distutils.core import setup, Extension
from distutils.ccompiler import new_compiler
import select
import sys
try:
    import setuptools
except ImportError:
    pass

extensions = []

def find_epoll():
    print 'looking for epoll ..',
    if hasattr(select, 'epoll'):
        print 'using select.epoll'
    else:
        print 'using extension'
        extensions.append(Extension('net/async/_epoll',
            sources = ['src/async/_epoll.c'],
        ))        

def find_kqueue():
    # kqueue support for Python before 2.6
    print 'looking for kqueue ..',
    if hasattr(select, 'kqueue'):
        print 'using select.kqueue'
    else:
        print 'using extension'
        extensions.append(Extension('net/async/_kqueue',
            sources = ['src/async/_kqueue.c'],
        ))

print 'looking for platform ..',
if sys.platform.startswith('linux'):
    print 'Linux'

    # depends on AF_AX25 and libax25
    compiler = new_compiler()
    lib_dirs = ['/lib', '/lib64', '/usr/lib', '/usr/lib64', '/usr/local/lib',
        '/usr/local/lib64']

    print 'looking for libax25 ..',
    lib_ax25 = compiler.find_library_file(lib_dirs, 'ax25')
    if lib_ax25:
        print lib_ax25
        extensions.append(Extension('net/protocol/_ax25',
            sources = ['src/protocol/_ax25.c'],
            libraries = ['ax25', 'ax25io'],
        ))
    else:
        print 'no'

    # Linux uses epoll interface
    find_epoll()

elif sys.platform == 'darwin':
    print 'darwin'
    
    # Darwin uses kqueue interface
    find_kqueue()

elif sys.platform == 'freebsd':
    print 'freebsd'

    # FreeBSD uses kqueue interface
    find_kqueue()

else:
    print 'unsupported (%s)' % (sys.platform,)


setup(name = 'net',
    version = '0.0.1',
    ext_modules = extensions,
    packages = [
        'net',
        'net.async',
        'net.protocol',
    ]
)
 
