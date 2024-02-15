from setuptools import setup

setup(
    name='claid',
    version='0.0.1',    
    description='A example Python package',
    url='https://github.com/shuds13/pyexample',
    author='Patrick Langer',
    author_email='patrick.langer97@gmx.de',
    license='Apache 2',
    packages=['claid'],
    install_requires=['protobuf',
                      'grpcio',                     
                      ],

    classifiers=[
        'Development Status :: 1 - Planning',
        'Intended Audience :: Science/Research',
        'License :: OSI Approved :: BSD License',  
        'Operating System :: POSIX :: Linux',        
        'Programming Language :: Python :: 2',
        'Programming Language :: Python :: 2.7',
        'Programming Language :: Python :: 3',
        'Programming Language :: Python :: 3.4',
        'Programming Language :: Python :: 3.5',
    ],
)
