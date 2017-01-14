from setuptools import setup, Extension

module1 = Extension('ephemeris',
                    sources = ['ephemeris_main.cpp', 'ephemeris.cpp'],
                    libraries = ['boost_python3', 'nova'],
            extra_compile_args=['-std=c++14'])

setup (name = 'Ephemeris',
       version = '0.1.0',
       description = 'Native ephemeris module for SkyPlanner',
       ext_modules = [module1])

