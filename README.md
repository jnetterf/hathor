This is the development site for Hathor, a music player built on last.fm and Rdio.

See http://hathor.nettek.ca

To compile on linux, you will need to have g++ as well as the development packages for qt, liblastfm and qoauth. Next, run:

qmake
make
On Windows, compiling is a bit more involved. Install Microsoft Visual Studio 2008, the Qt SDK with msvc, CMake, qca/qca-ossl from svn with CMake, qoauth, and liblastfm. If you're interested, I can upload a package with the last four, because it is a true pain to get working. Then, in an environment with both the MSVC directory and qt in PATH, run:

qmake
nmake
You could also just install linux.hathor
======

Hathor is a new kind of music player built on discovery
This is the development site for Hathor, a music player built on last.fm and Rdio.

See http://shdw.x10.mx/hathor

To compile on linux, you will need to have g++ as well as the development packages for qt, liblastfm and qoauth. Next, run:

qmake
make
On Windows, compiling is a bit more involved. Install Microsoft Visual Studio 2008, the Qt SDK with msvc, CMake, qca/qca-ossl from svn with CMake, qoauth, and liblastfm. If you're interested, I can upload a package with the last four, because it is a true pain to get working. Then, in an environment with both the MSVC directory and qt in PATH, run:

qmake
nmake
You could also just install linux.
