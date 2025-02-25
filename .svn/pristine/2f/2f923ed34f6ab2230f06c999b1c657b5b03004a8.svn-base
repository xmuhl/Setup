// MyButton.cpp : 实现文件
//

#include "stdafx.h"
#include "Setup.h"
#include "MyButton.h"
#include "UDefault.h"

//using namespace Gdiplus;
//#pragma comment(lib, "GdiPlus.lib")
#include "resource.h"

// CMyButton

IMPLEMENT_DYNAMIC(CMyButton, CButton)

CMyButton::CMyButton()
	: m_strBtnName(_T(""))
	, m_bIsPicLoaded(FALSE)
	, m_bTrackMouseEvent(true)
	, m_bHover(false)
{
}

CMyButton::~CMyButton()
{
	FreeData();
}

BEGIN_MESSAGE_MAP(CMyButton, CButton)
	ON_WM_MOUSEMOVE()
	ON_WM_MOUSELEAVE()
	ON_WM_MOUSEHOVER()
END_MESSAGE_MAP()

// CMyButton 消息处理程序
void CMyButton::OnMouseMove(UINT nFlags, CPoint point)
{
	// TODO:  在此添加消息处理程序代码和/或调用默认值
	HCURSOR hCur = LoadCursor(NULL, IDC_HAND);
	::SetCursor(hCur);

	if (m_bTrackMouseEvent)
	{
		TRACKMOUSEEVENT cTrackEvent;
		cTrackEvent.cbSize = sizeof(TRACKMOUSEEVENT);
		cTrackEvent.dwFlags = TME_HOVER | TME_LEAVE;
		cTrackEvent.dwHoverTime = 10;		//鼠标停留多久触发悬停消息
		cTrackEvent.hwndTrack = GetSafeHwnd();
		if (::_TrackMouseEvent(&cTrackEvent))	//如果调用失败，不再跟踪鼠标事件
			m_bTrackMouseEvent = false;
	}

	CButton::OnMouseMove(nFlags, point);
}

void CMyButton::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	// TODO:  添加您的代码以绘制指定项

	if (lpDrawItemStruct->itemState & ODS_SELECTED)	//选中
	{
		/*if (m_strBtnName == _T("上一步"))
		{
			m_RecourceID = (UINT)IDB_PREFOCUS;
		}
		else if (m_strBtnName == _T("下一步"))
		{
			m_RecourceID = (UINT)IDB_NEXTFOCUS;
		}
		else if (m_strBtnName == _T("取消"))
		{
			m_RecourceID = (UINT)IDB_CANCELFOCUS;
		}
		else if (m_strBtnName == _T("关闭"))
		{
			m_RecourceID = (UINT)IDB_MAINCLOSESEL;
		}
		else if (m_strBtnName == _T("最小化"))
		{
			m_RecourceID = (UINT)IDB_MAINMINSEL;
		}*/
		if (m_strBtnName == _T("关闭"))
		{
			m_RecourceID = (UINT)IDB_MAINCLOSESEL;
		}
		else if (m_strBtnName == _T("最小化"))
		{
			m_RecourceID = (UINT)IDB_MAINMINSEL;
		}
	}
	else if (lpDrawItemStruct->itemState & ODS_FOCUS)	//焦点
	{
		/*if (m_strBtnName == _T("上一步"))
		{
			m_RecourceID = (UINT)IDB_PREFOCUS;
		}
		else if (m_strBtnName == _T("下一步"))
		{
			m_RecourceID = (UINT)IDB_NEXTFOCUS;
		}
		else if (m_strBtnName == _T("取消"))
		{
			m_RecourceID = (UINT)IDB_CANCELFOCUS;
		}
		else if (m_strBtnName == _T("关闭"))
		{
			m_RecourceID = (UINT)IDB_MAINCLOSESEL;
		}
		else if (m_strBtnName == _T("最小化"))
		{
			m_RecourceID = (UINT)IDB_MAINMINSEL;
		}*/
		if (m_strBtnName == _T("关闭"))
		{
			m_RecourceID = (UINT)IDB_MAINCLOSESEL;
		}
		else if (m_strBtnName == _T("最小化"))
		{
			m_RecourceID = (UINT)IDB_MAINMINSEL;
		}
	}
	else if (m_bHover)	//停留
	{
		/*if (m_strBtnName == _T("上一步"))
		{
			m_RecourceID = (UINT)IDB_PREHOVER;
		}
		else if (m_strBtnName == _T("下一步"))
		{
			m_RecourceID = (UINT)IDB_NEXTHOVER;
		}
		else if (m_strBtnName == _T("取消"))
		{
			m_RecourceID = (UINT)IDB_CANCELHOVER;
		}
		else if (m_strBtnName == _T("关闭"))
		{
			m_RecourceID = (UINT)IDB_MAINCLOSESEL;
		}
		else if (m_strBtnName == _T("最小化"))
		{
			m_RecourceID = (UINT)IDB_MAINMINSEL;
		}*/
		if (m_strBtnName == _T("关闭"))
		{
			m_RecourceID = (UINT)IDB_MAINCLOSESEL;
		}
		else if (m_strBtnName == _T("最小化"))
		{
			m_RecourceID = (UINT)IDB_MAINMINSEL;
		}
	}
	else				//正常
	{
		/*if (m_strBtnName == _T("上一步"))
		{
			m_RecourceID = IDB_PRE;
		}
		else if (m_strBtnName == _T("下一步"))
		{
			m_RecourceID = IDB_NEXT;
		}
		else if (m_strBtnName == _T("取消"))
		{
			m_RecourceID = IDB_CANCEL;
		}
		else if (m_strBtnName == _T("关闭"))
		{
			m_RecourceID = (UINT)IDB_MAINCLOSENOMAL;
		}
		else if (m_strBtnName == _T("最小化"))
		{
			m_RecourceID = (UINT)IDB_MAINMINNOMAL;
		}*/
		if (m_strBtnName == _T("关闭"))
		{
			m_RecourceID = (UINT)IDB_MAINCLOSENOMAL;
		}
		else if (m_strBtnName == _T("最小化"))
		{
			m_RecourceID = (UINT)IDB_MAINMINNOMAL;
		}
	}
	LoadImageFromResource(m_RecourceID, _T("PNG"));

	if (m_bIsPicLoaded)
	{
		//Get control measures

		//使用GDI画图
		if (m_image.GetBPP() == 32) //确认该图像包含Alpha通道
		{
			int i; int j;
			for (i = 0; i < m_image.GetWidth(); i++)
			{
				for (j = 0; j < m_image.GetHeight(); j++)
				{
					byte* pByte = (byte*)m_image.GetPixelAddress(i, j);
					pByte[0] = pByte[0] * pByte[3] / 255;
					pByte[1] = pByte[1] * pByte[3] / 255;
					pByte[2] = pByte[2] * pByte[3] / 255;
				}
			}
		}

		RECT rc;
		this->GetClientRect(&rc);
		m_image.Draw(lpDrawItemStruct->hDC, rc);
	}
}

// 设置显示的位图
void CMyButton::SetBitmap(UINT RecourceID)
{
	m_RecourceID = RecourceID;
}

void CMyButton::OnMouseLeave()
{
	// TODO:  在此添加消息处理程序代码和/或调用默认值
	m_bHover = false;
	m_bTrackMouseEvent = true;	//重新开始跟踪

	Invalidate(false);

	CButton::OnMouseLeave();
}

bool CMyButton::LoadImageFromResource(IN CImage* pImage, IN UINT nResID, IN LPCTSTR lpTyp)
{
	SetLastError(ERROR_SUCCESS);
	if (m_bIsPicLoaded)
		FreeData();

	// 查找资源
	HRSRC hRsrc = ::FindResource(AfxGetResourceHandle(), MAKEINTRESOURCE(nResID), lpTyp);
	if (hRsrc == NULL) return false;

	// 加载资源
	HGLOBAL hImgData = ::LoadResource(AfxGetResourceHandle(), hRsrc);
	if (hImgData == NULL)
	{
		::FreeResource(hImgData);
		return false;
	}

	// 锁定内存中的指定资源
	LPVOID lpVoid = ::LockResource(hImgData);

	LPSTREAM pStream = NULL;
	DWORD dwSize = ::SizeofResource(AfxGetResourceHandle(), hRsrc);
	HGLOBAL hNew = ::GlobalAlloc(GHND, dwSize);
	LPBYTE lpByte = (LPBYTE)::GlobalLock(hNew);
	::memcpy(lpByte, lpVoid, dwSize);

	// 解除内存中的指定资源
	::GlobalUnlock(hNew);

	// 从指定内存创建流对象
	HRESULT ht = ::CreateStreamOnHGlobal(hNew, TRUE, &pStream);
	if (ht != S_OK)
	{
		GlobalFree(hNew);
	}
	else
	{
		// 加载图片
		pImage->Load(pStream);
		m_bIsPicLoaded = TRUE;
		GlobalFree(hNew);
	}

	// 释放资源
	::FreeResource(hImgData);

	return true;
}

bool CMyButton::LoadImageFromResource(/*IN CImage* pImage,*/ IN UINT nResID, IN LPCTSTR lpTyp)
{
	SetLastError(ERROR_SUCCESS);
	if (m_bIsPicLoaded)
		FreeData();

	// 查找资源
	HRSRC hRsrc = ::FindResource(AfxGetResourceHandle(), MAKEINTRESOURCE(nResID), lpTyp);
	if (hRsrc == NULL) return false;

	// 加载资源
	HGLOBAL hImgData = ::LoadResource(AfxGetResourceHandle(), hRsrc);
	if (hImgData == NULL)
	{
		::FreeResource(hImgData);
		return false;
	}

	// 锁定内存中的指定资源
	LPVOID lpVoid = ::LockResource(hImgData);

	LPSTREAM pStream = NULL;
	DWORD dwSize = ::SizeofResource(AfxGetResourceHandle(), hRsrc);
	HGLOBAL hNew = ::GlobalAlloc(GHND, dwSize);
	LPBYTE lpByte = (LPBYTE)::GlobalLock(hNew);
	::memcpy(lpByte, lpVoid, dwSize);

	// 解除内存中的指定资源
	::GlobalUnlock(hNew);

	// 从指定内存创建流对象
	HRESULT ht = ::CreateStreamOnHGlobal(hNew, TRUE, &pStream);
	if (ht != S_OK)
	{
		GlobalFree(hNew);
	}
	else
	{
		// 加载图片
		m_image.Load(pStream);
		m_bIsPicLoaded = TRUE;
		GlobalFree(hNew);
	}

	// 释放资源
	::FreeResource(hImgData);

	return true;
}

BOOL CMyButton::Load(HMODULE hModule, UINT resourceID, CString lpTyp /*= _T("")*/)
{
	if (m_bIsPicLoaded)
		FreeData();

	//判断是BMP还是其他格式
	if (lpTyp == _T(""))
	{
		//BMP格式
		m_image.LoadFromResource(hModule, resourceID);//加载提示图片
	}
	else
	{
		//其它格式
		LoadImageFromResource(/*&m_image, */resourceID, lpTyp);
	}

	//Mark as loaded

	m_bIsPicLoaded = TRUE;

	Invalidate();
	RedrawWindow();

	return TRUE;
}

BOOL CMyButton::Load(CString& szFilePath)
{
	//Set success error state
	SetLastError(ERROR_SUCCESS);
	if (m_bIsPicLoaded)
		FreeData();
	//Open the specified file

	m_image.Load(szFilePath);

	m_bIsPicLoaded = TRUE;

	Invalidate();
	RedrawWindow();

	return TRUE;
}

void CMyButton::FreeData()
{
	if (m_image)
	{
		HBITMAP hbitmap = m_image.Detach();
		DeleteObject(hbitmap);
		m_image.Destroy();
	}

	m_bIsPicLoaded = FALSE;
}

void CMyButton::OnMouseHover(UINT nFlags, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	m_bTrackMouseEvent = false;	//不再跟踪，因为已经在控件范围内了
	m_bHover = true;
	Invalidate();				//重绘按钮

	CButton::OnMouseHover(nFlags, point);
}