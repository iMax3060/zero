[![Documentation](https://codedocs.xyz/iMax3060/zero.svg)](https://codedocs.xyz/iMax3060/zero/)

# About

Zero is a transactional storage manager used mainly for prototyping in Database Systems research. It supports operations on a [Foster B-tree](http://dl.acm.org/citation.cfm?id=2338630) data structure with ACID semantics. Zero is designed for highly scalable, high-throughput OLTP. Being a research prototype, it does not provide certain usability features expected on a real production-level system.

# History

Zero is a fork of [Shore-MT](https://sites.google.com/site/shoremt/), which itself is derived from [Shore](http://research.cs.wisc.edu/shore/). The latter was developed in the early 90's by researchers in the University of Wisconsin-Madison, whereas the former was a continuation of the project at Carnegie Mellon University and, later on, EPFL. Shore-MT focuses on improving scalability in multi-core CPUs. Several published techniques in the database literature are based on Shore/Shore-MT, including recent and ongoing research.

Zero was developed at HP Labs. The initial milestone of the project was to implement the Foster B-tree data structure, thereby eliminating support for traditional ARIES-based B-trees and heap files. Zero also supports the Orthogonal Key-Value Locking Protocol and an improved Lock Manager design, as well as a novel swizzling technique that eliminates critical buffer manager overheads, deliviring performance comparable to in-memory systems despite being disk-based.

The latest developments in Zero are focused on Instant Recovery, a novel family of algorithms that extends the traditional ARIES design with on-demand recovery without blocking the system for new transactions, thereby significantly improving system availability. This [project](http://instantrecovery.github.io) is developed in a collaboration between HP Labs and the University of Kaiserslautern.

For a list of publications based on the Zero storage manager, see below.

# Project structure

The Zero source code directory is divided into three major parts:
- `src/common` Common utilities and infrastructure such as threads, latches, debug, etc.
- `src/sm` The Zero storage manager, providing an ACID-compliant key-value store based on Foster B-trees
- `src/cmd` Benchmarks (derived from [Shore-Kits](https://bitbucket.org/shoremt/shore-kits/src)) and utility programs packed in a tool called **zapps**

The storage manager consists of the following main components:
- The volume manager, which manages a file on which database pages are stored. Each page belongs to a *store* (i.e., a B-tree)
  - Main files: `vol.cpp`, `alloc_cache.cpp`, `stnode_cache.cpp`
- The buffer manager, which caches pages of a volume and keeps track of writes for transaction consistency
  - Main files: `bf_tree.cpp`, `bf_tree_cleaner.cpp`, `chkpt.cpp`
- The log manager, which manages a write-ahead log as well as its indexed log archive, used for some instant recovery techniques
  - Main files: `log_*.cpp`, `logarchiver.cpp`
- The Foster B-tree implementation
  - Main files: `btree.cpp`, `btree_*.cpp`, `btcursor.cpp`
- The transaction manager, which keeps track of active transactions and supports commit/abort functionality
  - Main files: `xct.cpp`
- The lock manager, which provides transaction isolation with orthogonal key-value locking
  - Main files: `lock.cpp`, `lock_*.cpp`
- The recovery manager, which provides ACID-compliant recovery from system and media failures with instant recovery
  - Main files: `restart.cpp`, `restore.cpp`, `chkpt.cpp`, page-based REDO logic in `bf_tree.cpp`
- The top-level SM interface: `sm.cpp`

# Building

## Dependencies

Zero is developed in C++14 and uses the CMake build system (version 3.9 or later). Its dependencies are a few [Boost libraries](http://www.boost.org/) (Program Options, System, Thread, Atomic, Filesystem and Regex in version 1.48 or later), which are widely available in the major Linux distributions and the [CDS library](http://libcds.sourceforge.net/doc/cds-api/index.html) (Concurrent Data Structures library) which needs to be build from source (it's planned to integrate it as a git submodule).

Currently, Zero is supported only on **Linux**.

### CMake, GCC, Boost and git

- **Ubuntu 12.04 Precise Pangolin (and possibly most other unsupported versions)**
  
  The GCC version of older and unsupported Ubuntu versions is too outdated and a later version of GCC (e.g. 7.3.0) needs to be installed from source. Unfortunately, the compilation takes a long time.

  The CMake version of older and unsupported Ubuntu versions is too outdated, so a later version of CMake (e.g. 3.10.2) has to be installed manually.

  The Boost version of older and unsupported Ubuntu versions is too outdated, so a later version of the Boost library (e.g. 1.66.0) has to be installed from source. Unfortunately, the compilation takes a long time.
  ```
  sudo apt-get install git build-essential gcc-multilib
  
  wget --no-check-certificate ftp://ftp.gnu.org/gnu/gcc/gcc-7.3.0/gcc-7.3.0.tar.xz
  tar -xvf gcc-7.3.0.tar.xz
  cd gcc-7.3.0
  ./contrib/download_prerequisites
  mkdir ../objdir
  cd ../objdir
  ../gcc-7.3.0/configure --prefix=/usr/local --enable-languages=c,c++
  make -j `cat /proc/cpuinfo | grep processor | wc -l`
  sudo make install
  rm -rf gcc-7.3.0.tar.xz gcc-7.3.0 objdir
  sudo ln -sf /usr/bin/g++-7 /usr/bin/g++
  sudo ln -sf /usr/bin/gcc-7 /usr/bin/gcc
  
  wget --no-check-certificate https://cmake.org/files/v3.10/cmake-3.10.2-Linux-x86_64.sh
  sudo sh ./cmake-3.10.2-Linux-x86_64.sh --prefix=/usr/local --skip-license
  rm cmake-3.10.2-Linux-x86_64.sh
  
  wget https://dl.bintray.com/boostorg/release/1.66.0/source/boost_1_66_0.tar.bz2
  tar -xjf boost_1_66_0.tar.bz2
  cd boost_1_66_0
  ./bootstrap.sh --prefix=/usr/local
  sudo ./b2 install
  cd ..
  sudo rm -rf boost_1_66_0 boost_1_66_0.tar.bz2
  ```

- **Ubuntu 14.04 Trusty Tahr**
  
  The default GCC version of Ubuntu 14.04 *Trusty Tahr* is too outdated, but a later version of GCC (e.g. 7) is available in a testing repository.

  The CMake version of Ubuntu 14.04 *Trusty Tahr* is too outdated, so a later version of CMake (e.g. 3.10.2) has to be installed manually.
  ```
  sudo apt install git
  sudo apt install libboost-dev libboost-program-options-dev libboost-system-dev libboost-thread-dev libboost-atomic-dev libboost-filesystem-dev libboost-regex-dev
  
  sudo add-apt-repository ppa:ubuntu-toolchain-r/test
  sudo apt update
  sudo apt install gcc-7 g++-7
  sudo ln -sf /usr/bin/g++-7 /usr/bin/g++
  sudo ln -sf /usr/bin/gcc-7 /usr/bin/gcc
  
  wget --no-check-certificate https://cmake.org/files/v3.10/cmake-3.10.2-Linux-x86_64.sh
  sudo sh ./cmake-3.10.2-Linux-x86_64.sh --prefix=/usr/local --skip-license
  rm cmake-3.10.2-Linux-x86_64.sh
  ```

- **Ubuntu 16.04 Xenial Xerus**
  
  The default GCC version of Ubuntu 16.04 *Xenial Xerus* is too outdated, but a later version of GCC (e.g. 7) is available in a testing repository.

  The default CMake version of Ubuntu 16.04 *Xenial Xerus* is too outdated, but a later version of CMake (3.10.1) is available in a nightly repository.

  The default Boost version of Ubuntu 16.04 *Xenial Xerus* doesn't compiler with later GCC versions, so another version of the Boost library (e.g. 1.66.0) has to be installed from source. Unfortunately, the compilation takes a long time.
  ```
  sudo apt install git
  sudo add-apt-repository ppa:ubuntu-toolchain-r/test
  sudo add-apt-repository ppa:nschloe/cmake-nightly
  sudo apt update
  sudo apt install gcc-7 g++-7 cmake
  sudo ln -sf /usr/bin/g++-7 /usr/bin/g++
  sudo ln -sf /usr/bin/gcc-7 /usr/bin/gcc
  
  wget https://dl.bintray.com/boostorg/release/1.66.0/source/boost_1_66_0.tar.bz2
  tar -xjf boost_1_66_0.tar.bz2
  cd boost_1_66_0
  ./bootstrap.sh --prefix=/usr/local
  sudo ./b2 install
  cd ..
  sudo rm -rf boost_1_66_0 boost_1_66_0.tar.bz2
  ```

- **Ubuntu 17.04 Zesty Zapus**

  Ubuntu 17.04 *Zesty Zapus* isn't supported anymore and the ``old-releases``-repositories on the archive server needs to be added to the ``sources.list``.

  The default CMake version of Ubuntu 17.04 *Zesty Zapus* is too outdated, but a later version of CMake (3.10.1) is available in a nightly repository.
  ```
  sudo sh -c 'echo "deb http://old-releases.ubuntu.com/ubuntu/ zesty main restricted universe multiverse\ndeb http://old-releases.ubuntu.com/ubuntu/ zesty-updates main restricted universe multiverse\ndeb http://old-releases.ubuntu.com/ubuntu/ zesty-security main restricted universe multiverse\ndeb http://old-releases.ubuntu.com/ubuntu/ zesty-backports main restricted universe multiverse" > /etc/apt/sources.list'
  sudo apt update
  sudo apt install git build-essential gcc-6 g++-6
  sudo apt install libboost-dev libboost-program-options-dev libboost-system-dev libboost-thread-dev libboost-atomic-dev libboost-filesystem-dev libboost-regex-dev
  sudo add-apt-repository ppa:nschloe/cmake-nightly
  sudo apt update
  sudo apt install cmake
  ```

- **Ubuntu 17.10 Artful Aardvard (and newer)**
  ```
  sudo apt install git cmake build-essential
  sudo apt install libboost-dev libboost-program-options-dev libboost-system-dev libboost-thread-dev libboost-atomic-dev libboost-filesystem-dev libboost-regex-dev
  ```

- **Debian (Stretch)**

  The CMake version of Debian Stretch is too outdated, so a later version of CMake (e.g. 3.10.2) has to be installed manually.
  ```
  su
  apt-get install git build-essential
  apt-get install libboost-dev libboost-program-options-dev libboost-system-dev libboost-thread-dev libboost-atomic-dev libboost-filesystem-dev libboost-regex-dev
  wget https://cmake.org/files/v3.10/cmake-3.10.2-Linux-x86_64.sh
  sh ./cmake-3.10.2-Linux-x86_64.sh --prefix=/usr/local --skip-license
  rm cmake-3.10.2-Linux-x86_64.sh
  exit
  ```
- **Debian (Buster and later)**
  ```
  su
  apt-get install git cmake build-essential
  sudo apt install libboost-dev libboost-program-options-dev libboost-system-dev libboost-thread-dev libboost-atomic-dev libboost-filesystem-dev libboost-regex-dev
  exit
  ```

- **CentOS (6 and later)**

  The default GCC version of CentOS is too outdated, but there is a later version of GCC (e.g. 7) in the SCLo RH repository.
  
  The CMake version of CentOS is too outdated, so a later version of CMake (e.g. 3.10.2) has to be installed manually.
  ```
  sudo yum install centos-release-scl
  sudo yum install devtoolset-7-gcc-c++
  export PATH=/opt/rh/devtoolset-7/root/usr/bin:${PATH}
  
  sudo yum install make git
  sudo yum install boost-devel
  
  wget https://cmake.org/files/v3.10/cmake-3.10.2-Linux-x86_64.sh
  sudo sh ./cmake-3.10.2-Linux-x86_64.sh --prefix=/usr/local --skip-license
  rm cmake-3.10.2-Linux-x86_64.sh
  ```

- **Arch Linux**

  The default CMake version of Arch Linux is too outdated, but a later version of CMake (e.g. 3.10.2) is available as package that needs to be downloaded manually.
  ```
  sudo pacman -S git boost-libs boost
  wget -O cmake-x86_64.pkg.tar.xz https://www.archlinux.org/packages/extra/x86_64/cmake/download/
  sudo pacman -U cmake-x86_64.pkg.tar.xz
  rm cmake-x86_64.pkg.tar.xz
  ```

### CDS library
- Download and compile the CDS library:
  ```
  git clone https://github.com/khizmax/libcds.git
  mkdir libcds/build-release
  cd libcds/build-release
  cmake -DCMAKE_BUILD_TYPE=RELEASE -DCMAKE_INSTALL_PREFIX=/usr/local ..
  make
  ```
- Install the CDS library
  * Without `sudo` (isn't installed on Debian by default)
    ```
    su
    make install
    exit
    ```
  * With `sudo`
    ```
    sudo make install
    ```
- Clean up the download directory
  ```
  cd ../..
  rm -rf libcds
  ```
  
## Compilation

CMake supports out-of-source builds, which means that binaries are generated in a different directory than the source files. This not only maintains a clean source directory, but also allows multiple coexisting builds with different configurations.

The typical approach is to create a `build` folder inside the project root after cloning it with git:

```
git clone https://github.com/iMax3060/zero
cd zero
git submodule init
git submodule update
mkdir build
```

To generate the build files, type:

```
cd build
cmake ..
```

On some older Ubuntu versions there are issues with the interprocedural optimization. If the compilation with ``make`` causes linker errors, it might help to deactivate interprocedural optimization, which causes a significantly lower performance of Zero.

```
cmake -DINTERPROCEDURAL_OPTIMIZATION=OFF ..
```

By default, a release version with debug symbols (many compiler optimizations) of Zero is compiled. But there are build types which support a more elaborate debugging of Zero with additional checks that are disabled by default:

- ```cmake -DCMAKE_BUILD_TYPE=Debug1 ..```
- ```cmake -DCMAKE_BUILD_TYPE=Debug3 ..```
- ```cmake -DCMAKE_BUILD_TYPE=Debug5 ..```

Finally, to compile:

```
make -j `cat /proc/cpuinfo | grep processor | wc -l` sm; make -j `cat /proc/cpuinfo | grep processor | wc -l` zapps
```

The `-j` flag enables compilation in parallel on multi-core CPUs. It is a standard feature of the Make build system. The `sm` target builds the static storage manager library, `libsm` and the `zapps` target builds an executable used to benchmark Zero.

To use the benchmarks and utilities, compile the *zapps* binary with:

```
make -j <number_of_cores> zapps
```

The binary is stored in the `src/cmd` subfolder. As an example, a TPC-C benchmark can be loaded and executed for 60 seconds with:

```
src/cmd/zapps kits -b tpcc --load --duration 60
```

For reference on the commands supported and their arguments, see the source files `src/cmd/base/command.cpp` and `src/cmd/kits/kits_cmd.cpp`

## Testing

## Current status
> Since February 2016, the test cases are not being maintained.

Zero is designed to be used as a library for transactional applications. As such, there is no program to be executed after compilation. The generated storage manager library is `libsm.a`. However, the test suite can be ran with:

```
make test
```

It uses the Google Test libraries, which are embedded in the source code.

# Publications

The following publications describe novel techniques which were implemented and evaluated on Zero:

* Caetano Sauer: [Modern techniques for transaction-oriented database recovery](http://wwwlgis.informatik.uni-kl.de/cms/fileadmin/publications/2017/PhD_Thesis_Caetano_Sauer.pdf) -- My PhD thesis
* Caetano Sauer, Goetz Graefe, Theo Härder [Instant restore after a media failure](https://arxiv.org/pdf/1702.08042.pdf). Link to an extended preprint version. Published on ADBIS 2017.
* Caetano Sauer, Lucas Lersch, Theo Härder, Goetz Graefe [Update propagation strategies for high-performance OLTP](http://wwwlgis.informatik.uni-kl.de/cms/typo3/sysext/cms/tslib/media/fileicons/pdf.gif). ADBIS 2016 (best-paper award), LNCS 9809, pp. 152-165
Springer-Verlag, August 2016 
* Caetano Sauer, Goetz Graefe, Theo Härder: [Single-pass restore after a media failure](http://btw-2015.de/res/proceedings/Hauptband/Wiss/Sauer-Single-pass_restore_after_a.pdf). BTW 2015:217-236 [[Slides](http://btw-2015.de/res/slides/Sauer-Single-pass_restore_after_a_m_slides.pdf)]
* Goetz Graefe, Hideaki Kimura: [Orthogonal key-value locking](http://btw-2015.de/res/proceedings/Hauptband/Wiss/Graefe-Orthogonal_key-value_locking.pdf). BTW 2015:237-256 [[Slides](http://btw-2015.de/res/slides/Graefe-Orthogonal_key-value_locking_slides.pdf)]
* Goetz Graefe, Haris Volos, Hideaki Kimura, Harumi A. Kuno, Joseph Tucek, Mark Lillibridge, Alistair C. Veitch: [In-Memory Performance for Big Data](http://www.vldb.org/pvldb/vol8/p37-graefe.pdf). PVLDB 8(1):37-48 (2014)
* Goetz Graefe, Mark Lillibridge, Harumi A. Kuno, Joseph Tucek, Alistair C. Veitch: [Controlled lock violation](http://doi.acm.org/10.1145/2463676.2465325). SIGMOD 2013:85-96
* Goetz Graefe, Hideaki Kimura, Harumi A. Kuno: [Foster b-trees](http://doi.acm.org/10.1145/2338626.2338630). ACM Trans. Database Syst. (TODS) 37(3):17 (2012)
* Hideaki Kimura, Goetz Graefe, Harumi A. Kuno: [Efficient Locking Techniques for Databases on Modern Hardware](http://www.adms-conf.org/kimura_adms12.pdf). ADMS@VLDB 2012:1-12
* Goetz Graefe, Harumi A. Kuno: [Definition, Detection, and Recovery of Single-Page Failures, a Fourth Class of Database Failures](http://vldb.org/pvldb/vol5/p646_goetzgraefe_vldb2012.pdf). PVLDB 5(7):646-655 (2012)

# Contact

This repository is maintained by Max Gilbert at the University of Kaiserslautern. If you wish to experiment with Zero, feel free to contact me at `m_gilbert13@cs.uni-kl.de`. Being a research prototype used by many developers over the course of two decades, getting started in the code may be a daunting task, due to the lack of thorough documentation and the high heterogeneity of the code.
