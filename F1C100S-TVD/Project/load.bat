@echo off %�ر���ʾ����%
color 2e %���ô�����ɫ%
title=F1C100S/F1C200S����
color 2e %���ô�����ɫ%
echo С�����Ƽ�
echo �����ˣ���ΰ
echo ���ڣ�2021-11-15
echo QQ: 718595426

echo ׼������...


cd\  %��ص���Ŀ¼%
cd "%~dp0sunxi-tools-��¼"
echo ��¼���ߵ�ַ��%~dp0sunxi-tools-��¼
echo �����ַ��%~dp0


echo ������
sunxi-fel -p spiflash-write 0x50000 "%~dp0f1c100s.bin"
echo �������

TIMEOUT /T 2 %��ʱ�˳�%
