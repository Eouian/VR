@echo off %�ر���ʾ����%
color 2e %���ô�����ɫ%
title=FLOAD����
color 2e %���ô�����ɫ%

cd "%~dp0"

echo ������
fload spi-nor-flash-write 0x50000 f1c100s.bin
echo �������

TIMEOUT /T 2 %��ʱ�˳�%

