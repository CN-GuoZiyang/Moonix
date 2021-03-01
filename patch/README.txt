使用较新版本的 gcc 编译 qemu 时，可能会出现 
ld: Error: unable to disambiguate: -no-pie (did you mean --no-pie ?)
的问题

解决方案：
将本文件夹下所有 .img 文件复制到 pc-bios/optionrom/ 下即可
