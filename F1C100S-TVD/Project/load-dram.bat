@echo off %�ر���ʾ����%
:startsel
title=F1C100S/F1C200S�������ص�DRAM������ %~n0
color 2e %���ô�����ɫ%
echo С�����Ƽ�
echo ���ڣ�2022-07-04
echo QQ: 718595426
echo . 
cd\  %��ص���Ŀ¼%
%~d0
cd "%~dp0sunxi-tools-��¼"

set addr=0x80000000 
echo [1][��λ���������FELģʽ]
echo [2]��ʼ��DRAM...
sunxi-fel spl boot0_to_fel.bin
echo [3]���Ƴ���DRAM[%addr%]
sunxi-fel -p write %addr% "%~dp0obj\F1C100S.bin"
echo [4]���г���...
sunxi-fel exec %addr%

TIMEOUT /T 2 %��ʱ�˳�%