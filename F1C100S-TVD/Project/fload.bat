@echo off %关闭显示命令%
color 2e %设置窗口颜色%
title=FLOAD下载
color 2e %设置窗口颜色%

cd "%~dp0"

echo 下载中
fload spi-nor-flash-write 0x50000 f1c100s.bin
echo 下载完成

TIMEOUT /T 2 %延时退出%

