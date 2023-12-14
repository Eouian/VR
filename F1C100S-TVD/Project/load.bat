@echo off %关闭显示命令%
color 2e %设置窗口颜色%
title=F1C100S/F1C200S下载
color 2e %设置窗口颜色%
echo 小淘气科技
echo 制作人：张伟
echo 日期：2021-11-15
echo QQ: 718595426

echo 准备下载...


cd\  %标回到根目录%
cd "%~dp0sunxi-tools-烧录"
echo 烧录工具地址：%~dp0sunxi-tools-烧录
echo 程序地址：%~dp0


echo 下载中
sunxi-fel -p spiflash-write 0x50000 "%~dp0f1c100s.bin"
echo 下载完成

TIMEOUT /T 2 %延时退出%
