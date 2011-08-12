from distutils.core import setup, Extension
from distutils.ccompiler import new_compiler
import sys
try:
    import setuptools
except ImportError:
    pass

extensions = []
if 'linux' in sys.platform:
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

    # epoll support for Python before 2.6
    if sys.hexversion < 0x020600f0:
        extensions.append(Extension('net/async/_epoll',
            sources = ['src/async/_epoll.c'],
        ))

setup(name = 'net',
    version = '0.0.1',
    ext_modules = extensions,
    packages = [
        'net',
        'net.async',
        'net.protocol',
    ]
)
 
