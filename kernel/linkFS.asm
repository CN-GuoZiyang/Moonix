/*
 *  kernel/linkFS.asm
 *  
 *  (C) 2021  Ziyang Guo
 */

 /*
  * 目前的文件系统实现是在链接时直接将文件系统镜像链接到 .data 段
  * 在运行时文件系统就会和内核一起直接被加载进内存
  */

.section .data
    .globl _fs_img_start
    .globl _fs_img_end
_fs_img_start:
    .incbin "fs.img"
_fs_img_end: