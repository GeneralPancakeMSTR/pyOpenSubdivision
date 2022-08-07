from setuptools import setup, find_packages
# import distutils.sysconfig

NAME = 'pyOpenSubdiv'
# VERSION = '0.0.1'
VERSION = '0.0.2'
DESCRIPTION = 'Python Catmull-Clark Subdivision.'
LONG_DESCRIPTION = 'A Python wrapper for the OpenSubdiv C++ Library far topology refiner.'

# Also needs (maybe)
# sudo apt-get install libatlas-base-dev (https://stackoverflow.com/questions/53347759/importerror-libcblas-so-3-cannot-open-shared-object-file-no-such-file-or-dire)

# import site
# import sys
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
    package_data={f'{NAME}.clib':['*.so','*.dll']}, # https://stackoverflow.com/questions/70334648/how-to-correctly-install-data-files-with-setup-py
    install_requires = ["numpy"],    
    keywords = ['subdivision','opensubdiv','Catmull-Clark','hard-surface'],
    classifiers= [
        "Development Status :: 3 - Alpha",
        "Intended Audience :: Developers",        
        "Programming Language :: Python :: 3",        
        "Operating System :: Microsoft :: Windows",
        "Operating System :: Unix",
    ]
)