Building the udsp library requires standard C and Fortran compilers,
and the [SCons][] build tool.

Unpack the udsp source code:

    $ gunzip < udsp-0.1.tar.gz | tar -xf -
    $ cd udsp-0.1/

To test udsp:

    $ scons test
    $ ./test-udsp && echo Success!

To build and install the static library:

    $ scons install


[SCons]: <http://www.scons.org/>
