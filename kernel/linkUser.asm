# 将用户程序链接到 .data 段

.section .data
    .global _user_img_start
    .global _user_img_end
_user_img_start:
    .incbin "User"
_user_img_end: