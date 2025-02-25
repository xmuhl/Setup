#pragma once

// CMyButton

class CMyButton : public CButton
{
	DECLARE_DYNAMIC(CMyButton)

public:
	CMyButton();
	virtual ~CMyButton();

protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);

	// 设置显示的位图
	void SetBitmap(UINT RecourceID);
	UINT m_RecourceID;  //存放资源ID

	bool m_bTrackMouseEvent;	//是否跟踪鼠标状态
	bool m_bHover;				//鼠标是否悬停上方

	// 标识该按钮是什么
	CString m_strBtnName;
	afx_msg void OnMouseLeave();

	bool LoadImageFromResource(/*IN CImage* pImage,*/ IN UINT nResID, IN LPCTSTR lpTyp);
	bool LoadImageFromResource(IN CImage* pImage, IN UINT nResID, IN LPCTSTR lpTyp);
	//Overload - Single load function
	BOOL Load(CString& szFilePath);
	BOOL Load(HMODULE hModule, UINT resourceID, CString lpTyp = _T(""));

	//Frees the image data
	void FreeData();

private:
	//Internal image stream buffer
	CImage m_image;

	//Control flag if a pic is loaded
	BOOL m_bIsPicLoaded;

public:
	afx_msg void OnMouseHover(UINT nFlags, CPoint point);
};
