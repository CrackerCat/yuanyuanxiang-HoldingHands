#pragma once


// CSettingDlg �Ի���

class CConfig;
class CSettingDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CSettingDlg)

public:
	CSettingDlg(CConfig&config,CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~CSettingDlg();

// �Ի�������
	enum { IDD = IDD_SETTING };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

	DECLARE_MESSAGE_MAP()

	CConfig & m_config;
public:
	afx_msg void OnBnClickedOk();
	CEdit m_EditPort;
	CEdit m_EditMaxConnection;
	CButton m_BnOverwrite;
	CEdit m_EditDesktopSavePath;
	CEdit m_EditCameraSavePath;
	virtual BOOL OnInitDialog();
};
