@echo off %关闭显示命令%
:startsel
title=F1C100S/F1C200S程序下载到DRAM并运行 %~n0
color 2e %设置窗口颜色%
echo 小淘气科技
echo 日期：2022-07-04
echo QQ: 718595426
echo . 
cd\  %标回到根目录%
%~d0
cd "%~dp0sunxi-tools-烧录"

set addr=0x80000000 
echo [1][复位开发板进入FEL模式]
echo [2]初始化DRAM...
sunxi-fel spl boot0_to_fel.bin
echo [3]复制程序到DRAM[%addr%]
sunxi-fel -p write %addr% "%~dp0obj\F1C100S.bin"
echo [4]运行程序...
sunxi-fel exec %addr%

TIMEOUT /T 2 %延时退出%