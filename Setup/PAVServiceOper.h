#pragma once
//��װ�������߳�
UINT ClosePAV(LPVOID pParam);
//����PAV��
class CPAVServiceOper
{
public:
	CPAVServiceOper();
	~CPAVServiceOper();

	// ��������
	bool RunService();
	// ��������
	bool CloseService();
};
