# 第九章 实现终端

----

目前我们的操作系统运行的应用程序都是硬编码在代码中的。本章实现一个终端，我们可以输入应用程序的路径来选择执行的程序。

本章内容：

- 开启键盘中断
- 实现输入缓冲区
- 实现 echo 程序，功能是将输入的字符显示在屏幕上
- 基于 echo 实现一个终端