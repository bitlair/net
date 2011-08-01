from distutils.core import setup, Extension

setup(name = 'net',
    version = '0.0.1',
    ext_modules = [
        Extension('net/protocol/_ax25', 
            sources = ['src/protocol/_ax25.c'],
            libraries = ['ax25', 'ax25io'],
        )
    ],
    packages = [
        'net',
        'net.protocol',
    ]
)
 
