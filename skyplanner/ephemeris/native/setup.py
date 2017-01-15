from setuptools import setup, Extension
import os
boost_python3 = os.environ.get('BOOST_PYTHON3_LIB')
if not boost_python3:
	boost_python3 = 'boost_python3'
module1 = Extension('ephemeris',
                    sources = ['ephemeris_main.cpp', 'ephemeris.cpp'],
                    libraries = [boost_python3, 'nova'],
            extra_compile_args=['-std=c++14'])

setup (name = 'Ephemeris',
       version = '0.1.0',
       description = 'Native ephemeris module for SkyPlanner',
       ext_modules = [module1])

