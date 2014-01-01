Building the udsp library requires standard C and Fortran compilers,
and the [SCons][] build tool.

Unpack the udsp source code:

    $ gunzip < udsp-0.1.tar.gz | tar -xf -
    $ cd udsp-0.1/

The udsp library consists of the files: `udsp.c`, `udsp.h`,
`fltop.c`, and `fltop.h`.  Copy these files into your project.

To test udsp:

    $ scons test
    $ ./test-udsp && echo Success!

To build and install udsp as a shared library:

    $ scons install


[SCons]: <http://www.scons.org/>