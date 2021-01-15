[![License](https://img.shields.io/badge/License-GPL%20v2-blue.svg)](https://gitlab.expidus.com/expidus/lunar/COPYING)

# lunar


Lunar is a modern file manager for the Expidus Desktop Environment. Lunar has been designed from the ground up to be fast and easy to use. Its user interface is clean and intuitive and does not include any confusing or useless options by default. Lunar starts up quickly and navigating through files and folders is fast and responsive.

----

### Homepage

[Lunar documentation](https://docs.expidus.com/expidus/lunar/start)

### Changelog

See [NEWS](https://gitlab.expidus.com/expidus/lunar/-/blob/master/NEWS) for details on changes and fixes made in the current release.

### Source Code Repository

[Lunar source code](https://gitlab.expidus.com/expidus/lunar)

### Download a Release Tarball

[Lunar archive](https://archive.expidus.org/src/expidus/lunar)
    or
[Lunar tags](https://gitlab.expidus.com/expidus/lunar/-/tags)

### Installation

From source: 

    % cd lunar
    % ./autogen.sh
    % make
    % make install

From release tarball:

    % tar xf lunar-<version>.tar.bz2
    % cd lunar-<version>
    % ./configure
    % make
    % make install

 Both autogen.sh and configure will list missing dependencies. 
 If your distribution provides development versions of the related packages, 
 install them. Otherwise you will need to build and install the missing dependencies from source.

For some additional build & debug hints, as well check the [Lunar Wiki pages](https://wiki.expidus.org/lunar/dev).

### Reporting Bugs

Visit the [reporting bugs](https://docs.expidus.com/expidus/lunar/bugs) page to view currently open bug reports and instructions on reporting new bugs or submitting bugfixes.

