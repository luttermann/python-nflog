from distutils.core import setup, Extension
# import numpy.distutils.misc_util

setup(
        name="nflog",
        version='0.1.2',
        author='Lasse Luttermann Poulsen',
        author_email='llp@k-net.dk',
        url='https://github.com/luttermann/python-nflog',
        classifiers=[
            'Development Status :: 4 - Beta',
            'Environment :: Plugins',
            'Intended Audience :: Developers',
            'Intended Audience :: Information Technology',
            'Intended Audience :: System Administrators',
            'License :: OSI Approved :: BSD License',
            'Operating System :: POSIX :: Linux',
            'Programming Language :: C',
            'Programming Language :: Python :: 3.5',
            'Topic :: Communications',
            'Topic :: Internet :: Log Analysis',
            'Topic :: System :: Networking :: Monitoring'
            ],
        ext_modules=[Extension(
            name="nflog",
            sources=["nflog.c"],
            libraries=["netfilter_log", "nfnetlink"]
            )],
    )

