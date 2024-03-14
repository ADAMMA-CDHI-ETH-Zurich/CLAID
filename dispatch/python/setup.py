from setuptools import setup, find_packages
from pathlib import Path
this_directory = Path(__file__).parent
long_description = (this_directory / "README.md").read_text()

setup(
    name='claid',
    version='0.0.6.6.1',    
    description='CLAID Python package (PyCLAID)',
    url='https://claid.ch',
    author='Patrick Langer',
    author_email='planger@ethz.ch',
    license='Apache 2',
    packages=find_packages(),
    include_package_data=True,
    long_description=long_description,
    long_description_content_type='text/markdown',
    install_requires=['protobuf==4.25.3',
                      'grpcio==1.59.3',
                      'numpy',      
                      ],

    classifiers=[
        'Development Status :: 4 - Beta',
        'Intended Audience :: Science/Research',
        'Intended Audience :: Developers',
        'Intended Audience :: Healthcare Industry',
        'Intended Audience :: Information Technology',
        'License :: OSI Approved :: Apache Software License',  
        'Operating System :: Android',
        'Operating System :: MacOS',
        'Operating System :: POSIX :: Linux',        
        'Operating System :: Unix',
        'Topic :: Scientific/Engineering :: Artificial Intelligence',
        'Programming Language :: Python :: 3.7',
        'Programming Language :: Python :: 3.8',
        'Programming Language :: Python :: 3.9',
    ],
    python_requires='>=3.7',

    entry_points='''
        [console_scripts]
        claid=claid.claid_cli:main
    ''',

)
