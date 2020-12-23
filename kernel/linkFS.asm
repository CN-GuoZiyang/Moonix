# 将文件系统链接到 .data 段

.section .data
    .global _fs_img_start
    .global _fs_img_end
_fs_img_start:
    .incbin "fs.img"
_fs_img_end: