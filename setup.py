from setuptools import setup, find_packages
import site

import distutils.sysconfig
import os
import sys

NAME = 'pysubdivision'
VERSION = '0.0.1'
DESCRIPTION = 'Python Catmull-Clark Subdivision.'
LONG_DESCRIPTION = 'A Python wrapper for the OpenSubdiv C++ Library far topology refiner.'


# print(site.USER_BASE)
# print(sys.prefix)
setup(
    name = NAME,
    version = VERSION,
    author = 'GeneralPancake',
    description = DESCRIPTION,
    long_description = LONG_DESCRIPTION,
    packages = find_packages(),
    include_package_data=True,
    package_data={'pysubdivision.clib':['*.so','*.dll']}, # https://stackoverflow.com/questions/70334648/how-to-correctly-install-data-files-with-setup-py
    install_requires = ["numpy"],
    keywords = ['subdivision','opensubdiv','Catmull-Clark','hard-surface'],
    classifiers= [
        "Development Status :: 0 - Alpha",
        "Intended Audience :: Developers",        
        "Programming Language :: Python :: 3",        
        "Operating System :: Microsoft :: Windows",
        "Operating System :: Unix",
    ]
)