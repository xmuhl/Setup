///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////
// PictureCtrl.h
//
// Author: Tobias Eiseler
//
// E-Mail: tobias.eiseler@sisternicky.com
//
// Function: A MFC Picture Control to display
//           an image on a Dialog, etc.
///////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////

#pragma once
#include "afxwin.h"

class CPictureCtrl :
	public CStatic
{
public:

	//Constructor
	CPictureCtrl(void);

	//Destructor
	~CPictureCtrl(void);

public:

	bool LoadImageFromResource(/*IN CImage* pImage,*/ IN UINT nResID, IN LPCTSTR lpTyp);
	bool LoadImageFromResource(IN CImage* pImage, IN UINT nResID, IN LPCTSTR lpTyp);
	//Overload - Single load function
	BOOL Load(CString& szFilePath);
	BOOL Load(HMODULE hModule, UINT resourceID, CString lpTyp = _T(""));

	//Frees the image data
	void FreeData();

protected:
	virtual void PreSubclassWindow();

	//Draws the Control
	virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);
	virtual BOOL OnEraseBkgnd(CDC* pDC);

private:

	//Internal image stream buffer
	CImage m_image;

	//Control flag if a pic is loaded
	BOOL m_bIsPicLoaded;
public:
	DECLARE_MESSAGE_MAP()
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
};
