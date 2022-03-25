# 工具链配置

## Centos7

许多服务器可能还在使用 Centos7, 它自带的工具链版本为: GCC 4.8.5/Cmake 2.8.12/make 3.82  

我们需要升级一下工具链, 当然你也可以使用 Docker 编译

### 更新 Cmake

使用 `Python3` 的包管理工具 `pip3` 可以快速安装新版本 `Cmake`

```
sudo pip3 install cmake --upgrade
```

检查版本: 

```
[zt@VM-4-4-centos ~]$ cmake -version
cmake version 3.21.3

CMake suite maintained and supported by Kitware (kitware.com/cmake).
```

如果 `pip3` 很慢, 请[换源到国内](https://mirrors.tuna.tsinghua.edu.cn/help/pypi/), 自带的 `pip3` 是 `Python3.6` 的老版本,
也需要升级


### 更新 GCC

安装新版本:

```
sudo yum install centos-release-scl-rh -y
sudo yum install devtoolset-10-toolchain -y
```

激活环境:

```
source /opt/rh/devtoolset-10/enable
```

bash 退出后就失效了, 如果需要长期使用可以添加一个配置, 每次登录时执行一下:

```
echo "source /opt/rh/devtoolset-10/enable" > /etc/profile.d/devtoolset-10.sh
```

检查版本:

```
[zt@VM-4-4-centos ~]$ gcc -v
Using built-in specs.
COLLECT_GCC=gcc
COLLECT_LTO_WRAPPER=/opt/rh/devtoolset-10/root/usr/libexec/gcc/x86_64-redhat-linux/10/lto-wrapper
Target: x86_64-redhat-linux
Configured with: ../configure --enable-bootstrap --enable-languages=c,c++,fortran,lto --prefix=/opt/rh/devtoolset-10/root/usr --mandir=/opt/rh/devtoolset-10/root/usr/share/man --infodir=/opt/rh/devtoolset-10/root/usr/share/info --with-bugurl=http://bugzilla.redhat.com/bugzilla --enable-shared --enable-threads=posix --enable-checking=release --enable-multilib --with-system-zlib --enable-__cxa_atexit --disable-libunwind-exceptions --enable-gnu-unique-object --enable-linker-build-id --with-gcc-major-version-only --with-linker-hash-style=gnu --with-default-libstdcxx-abi=gcc4-compatible --enable-plugin --enable-initfini-array --with-isl=/builddir/build/BUILD/gcc-10.2.1-20210130/obj-x86_64-redhat-linux/isl-install --disable-libmpx --enable-gnu-indirect-function --with-tune=generic --with-arch_32=x86-64 --build=x86_64-redhat-linux
Thread model: posix
Supported LTO compression algorithms: zlib
gcc version 10.2.1 20210130 (Red Hat 10.2.1-11) (GCC) 

[zt@VM-4-4-centos ~]$ make -v
GNU Make 4.2.1
Built for x86_64-redhat-linux-gnu
Copyright (C) 1988-2016 Free Software Foundation, Inc.
License GPLv3+: GNU GPL version 3 or later <http://gnu.org/licenses/gpl.html>
This is free software: you are free to change and redistribute it.
There is NO WARRANTY, to the extent permitted by law.
```