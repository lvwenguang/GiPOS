// FloorDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "POSClient.h"
#include "FloorDlg.h"
#include "NumberInputDlg.h"
#include "TableStatusDlg.h"
#include "PickUpDlg.h"
#include "VoidReasonDlg.h"
#include "OrderDlg.h"
#include "PreOrderDlg.h"
#include "NotifyKitchenDlg.h"
#include "FloorChooseDlg.h"
#include "OpenTableDlg.h"
#include "ReserveDlg.h"
#include "ReserveAddDlg.h"
#include "MSGBoxDlg.h"
#include "CustomerSearchDlg2.h"

// FloorDlg 对话框
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNAMIC(FloorDlg, CPosPage)

std::vector<TableItem> FloorDlg::m_Tables;
extern BOOL NO_FLOOR_PRICE;

FloorDlg::FloorDlg(CWnd* pParent /*=NULL*/)
: CPosPage(FloorDlg::IDD)
{
	m_nStatus=0;
	ITEM_COLUMNSIZE=8;
	ITEM_LINESIZE=6;
	m_nPageSize=ITEM_LINESIZE*ITEM_COLUMNSIZE-2;//每页的大小，需扣除翻页按钮
	m_nCurrentPage=0;
	m_nCurrentRvc=0;
	m_nPageCount=1;
	TIMER=10*1000;
	m_bpStatusNormal[0]=NULL;
	m_bpStatusNormal[1]=NULL;
	m_bpStatusPressed[0]=NULL;
	m_bpStatusPressed[1]=NULL;
}

FloorDlg::~FloorDlg()
{
	for (std::vector<CButton*>::iterator iter = m_itemButtons.begin(); iter!= m_itemButtons.end();iter++)
	{
		CButton *b = (*iter);
		b->DestroyWindow();
		delete b;
	}
	m_itemButtons.clear();
	delete m_bpStatusNormal[0];
	delete m_bpStatusNormal[1];
	delete m_bpStatusPressed[0];
	delete m_bpStatusPressed[1];
}

void FloorDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_STATIC_LOG, m_logCtrl);
	DDX_Control(pDX, IDC_STATIC_TIME, m_time);
}


BEGIN_MESSAGE_MAP(FloorDlg, CDialog)
	ON_WM_ERASEBKGND()
	ON_BN_CLICKED(IDC_BUTTON_CHANGE, &FloorDlg::OnBnClickedButtonChange)
	ON_BN_CLICKED(IDC_BUTTON_BEGIN, &FloorDlg::OnBnClickedBeginTable)
	ON_BN_CLICKED(IDC_BUTTON_PICKTAB, &FloorDlg::OnBnClickedPickTable)
	ON_BN_CLICKED(IDC_BUTTON_PICKCHK, &FloorDlg::OnBnClickedPickCheck)
	ON_BN_CLICKED(IDC_BUTTON_CANCEL, &FloorDlg::OnBnClickedButtonCancel)
	ON_BN_CLICKED(IDC_BUTTON_PRE, &FloorDlg::OnBnClickedButtonCancel)
	ON_BN_CLICKED(IDC_BUTTON_STATUS, &FloorDlg::OnBnClickedStatus)
	ON_BN_CLICKED(IDC_BUTTON_EDITCHK, &FloorDlg::OnBnClickedEditCheck)
	ON_BN_CLICKED(IDC_BUTTON_TAKEOUT, &FloorDlg::OnBnClickedTakeOut)
	ON_BN_CLICKED(IDC_BUTTON_EATIN, &FloorDlg::OnBnClickedEatIn)
	ON_BN_CLICKED(IDC_BUTTON_REFUND, &FloorDlg::OnBnClickedRefund)
	ON_BN_CLICKED(IDC_BUTTON_PARTY, &FloorDlg::OnBnClickedParty)
	ON_BN_CLICKED(IDC_BUTTON_FUNCTION, &FloorDlg::OnBnClickedFunction)
	ON_BN_CLICKED(IDC_BUTTON_NOTIFYKIT, &FloorDlg::OnBnClickedNotifyKitchen)
	ON_BN_CLICKED(IDC_BUTTON_CLEANTBL, &FloorDlg::OnBnClickedCleanTable)
	ON_BN_CLICKED(IDC_BUTTON_RESERVE,&FloorDlg::OnBnClickedReserve)
	ON_BN_CLICKED(IDC_BUTTON_TAINFO,&FloorDlg::OnMsgIncall)
	ON_COMMAND_RANGE(IDC_TABLE0,IDC_TABLE0+999,&FloorDlg::OnTableClicked)
	ON_COMMAND_RANGE(IDC_BUTTON_NEXTPAGE,IDC_BUTTON_NEXTPAGE+50,&CPosPage::OnNextPage)
	ON_WM_SIZE()
	ON_WM_CTLCOLOR()
	ON_WM_TIMER()
	ON_COMMAND_RANGE(IDC_CLASS_BUTTON,IDC_CLASS_BUTTON+99, &FloorDlg::OnBnClickedRadio)
	ON_MESSAGE(WM_MESSAGE_UPDATE,OnMsgUpdate)
END_MESSAGE_MAP()

// FloorDlg 消息处理程序

BOOL FloorDlg::OnInitDialog()
{
	if (m_strBackground.GetLength()==0)
	{
		m_strBackground=_T("Picture\\floor.png");
	}
	CPosPage::InitDialog(m_strBackground);
	m_bpStatusNormal[0]=Gdiplus::Image::FromFile(_T("Picture\\bt_status.png"));
	m_bpStatusNormal[1]=Gdiplus::Image::FromFile(_T("Picture\\bt_status_.png"));
	m_bpStatusPressed[0]=Gdiplus::Image::FromFile(_T("Picture\\bt_status1.png"));
	m_bpStatusPressed[1]=Gdiplus::Image::FromFile(_T("Picture\\bt_status1_.png"));
	
	for (int i=0;i<4;i++)
	{
		CString path;
		path.Format(_T("Icon\\%d.ico"),i+1);
		m_icTableStatus[i]=(HICON)::LoadImage(NULL,path , IMAGE_ICON,57,14,LR_LOADFROMFILE);
	}

	if (m_strTmplate.GetLength()==0)
	{
		m_strTmplate=_T("Page\\IDD_2_FLOOR.ini");
	}
	m_btnCtrl.GenaratePage2(m_strTmplate,this);

// 	LOGFONT m_tLogFont;
// 	memset(&m_tLogFont,0,sizeof(LOGFONT));
// 	m_tLogFont.lfHeight = 20;
// 	wcscpy_s(m_tLogFont.lfFaceName, _T("Microsoft YaHei"));

	TIMER=macrosInt[_T("TABLE_UPDATE_TIMER")]*1000;
	if (TIMER<3)
		TIMER=10*1000;
	SetTimer(1000,10000,NULL);//每隔10秒刷新时间显示
	SetTimer(1001,TIMER,NULL);//每隔15秒刷新桌位状态
	SYSTEMTIME st;
	CString strTime;
	GetLocalTime(&st);
	strTime.Format(_T("%02d:%02d"),st.wHour,st.wMinute);
	m_time.SetWindowText(strTime);

	CRoundButton2* pButton=(CRoundButton2*)GetDlgItem(IDC_BUTTON_STATUS);
	if(pButton)
	{
		pButton->SetImages(m_bpStatusNormal[0],m_bpStatusNormal[1],false);
	}
	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}
LRESULT FloorDlg::OnMsgUpdate(WPARAM wParam, LPARAM lParam)
{
	{//CMainSheet已保证在开单界面
		FloorDlg::OnSetActive();
		KillTimer(1001);
		SetTimer(1001,TIMER,NULL);
	}
	return 0;
}
void FloorDlg::OnSetActive()
{
	try{
		theLang.UpdatePage(this,_T("IDD_2_FLOOR"),TRUE);
		m_logCtrl.SetWindowText(((CPOSClientApp*)AfxGetApp())->m_strUserName);
		CPosPage::OnSetActive();
		UpdateTables();
		UpdateRvc(m_nCurrentRvc,FALSE);
	}
	catch(...)
	{
		LOG4CPLUS_ERROR(log_pos,"Catch Exception GetLastError="<<GetLastError());
	}
}
void FloorDlg::UpdateTables()
{
	try
	{
		if(OpenDatabase()==FALSE)
			return;
		m_Tables.clear();
		CRecordset rs( &theDB);
		CString strSQL;
		CString str2;
		//刷新桌态
		strSQL.Format(_T("SELECT * FROM tables WHERE table_id>0 AND table_status<>6 ORDER BY rvc_center_id,table_id"));
		if(!rs.Open(CRecordset::forwardOnly,strSQL))
		{
			LOG4CPLUS_ERROR(log_pos,"CRecordset Open Failed");
			return;
		}
		nStatus[0]=0;
		nStatus[1]=0;
		nStatus[2]=0;
		nStatus[3]=0;
		CDBVariant variant;
		while(!rs.IsEOF())
		{
			TableItem table={0};
			CString strName;
			rs.GetFieldValue(_T("table_id"),variant);
			table.id=variant.m_lVal;
			rs.GetFieldValue(_T("seat_num"),variant);
			table.seat_num=variant.m_iVal;
			rs.GetFieldValue(_T("table_status"),variant);
			table.status=variant.m_iVal-1;
			if (table.status<0||table.status>7)
				table.status=0;
			if (table.status<4)
				nStatus[table.status]++;

			rs.GetFieldValue(_T("rvc_center_id"),variant);
			table.rvc=variant.m_lVal;
			rs.GetFieldValue(_T("table_name"),strName);
			table.name.Format(_T("%s"),strName);
			m_Tables.push_back(table);
			rs.MoveNext();
		}
		rs.Close();
		for(int i=0;i<4;i++)
		{
			CButtonST* pCtrl=(CButtonST*)GetDlgItem(IDC_SLU_BUTTON+i+1);
			if (pCtrl)
			{
				CString text;
				text.Format(_T("%s (%d)"),strStatus[i],nStatus[i]);
				pCtrl->SetWindowText(text);
				if(!pCtrl->m_bDrawTransparent)
				{
					pCtrl->DrawTransparent();
					pCtrl->SetColor(CButtonST::BTNST_COLOR_FG_IN,RGB(255,255,255),0);
					pCtrl->SetColor(CButtonST::BTNST_COLOR_FG_OUT,RGB(255,255,255),0);
					pCtrl->SetColor(CButtonST::BTNST_COLOR_FG_FOCUS,RGB(255,255,255),0);
				}
			}
		}
		//设置预定桌位
		strSQL.Format(_T("SELECT distinct table_id FROM pre_order WHERE pre_order_status=2 AND ")
			_T("ABS(TIMESTAMPDIFF(MINUTE,NOW(),arrived_time))<=60 ORDER BY arrived_time;"));
		if(!rs.Open(CRecordset::forwardOnly,strSQL))
			return;
		theLang.LoadString(str2,IDS_BOOKED);
		while(!rs.IsEOF())
		{
			TableItem table;
			rs.GetFieldValue(_T("table_id"),variant);
			table.id=variant.m_iVal;
			vector <TableItem>::iterator pos=find(m_Tables.begin(),m_Tables.end(),table);
			if (pos!=m_Tables.end())
			{
				(*pos).name.Append(str2);
			}
			rs.MoveNext();
		}
		rs.Close();
		//刷新关联桌
		strSQL.Format(_T("SELECT t1.table_id,t2.table_name FROM order_head AS t1,order_head AS t2 WHERE t1.party_id=t2.party_id GROUP BY table_id"));
		rs.Open(CRecordset::forwardOnly,strSQL);
		while(!rs.IsEOF())
		{
			TableItem table={};
			rs.GetFieldValue(_T("table_id"),variant);
			table.id=variant.m_lVal;
			vector <TableItem>::iterator pos=find(m_Tables.begin(),m_Tables.end(),table);
			if (pos!=m_Tables.end())
			{
				rs.GetFieldValue(_T("table_name"),str2);
				(*pos).party_table.Format(str2);
			}
			rs.MoveNext();
		}
		rs.Close();
		//刷新金额
		int total_gst=0,cur_gst=0;
		double total_amount=0;
		CString strName;
		strSQL.Format(_T("SELECT table_id,raw_talbe,table_name,rvc_center_id,customer_num,customer_id,is_make,TIME_FORMAT(order_start_time,'%%H:%%i') as start_time,round(sum(should_amount),%d) as total FROM order_head group by table_id")
			,theApp.DECIMAL_PLACES);
		if(rs.Open(CRecordset::forwardOnly,strSQL))
		{
			while(!rs.IsEOF())
			{
				CDBVariant variant;
				rs.GetFieldValue(_T("table_id"),variant);
				if (variant.m_dwType==DBVT_NULL)
					break;
				if(variant.m_lVal<0){//过滤快餐的单据
					rs.MoveNext();
					continue;
				}
				TableItem table={0};
				table.id=variant.m_lVal;
				variant.m_lVal=0;
				vector <TableItem>::iterator pos=find(m_Tables.begin(),m_Tables.end(),table);
				if (pos!=m_Tables.end())
				{
					variant.m_lVal=0;
					rs.GetFieldValue(_T("customer_num"),variant);
					cur_gst+=variant.m_lVal;
					(*pos).cur_gst=variant.m_lVal;
					variant.m_lVal=0;
					rs.GetFieldValue(_T("is_make"),variant);
					(*pos).ismake=variant.m_iVal;
					rs.GetFieldValue(_T("start_time"),str2);
					(*pos).open_time.Format(_T("%s"),str2);
					rs.GetFieldValue(_T("total"),str2);
					total_amount+=_wtof(str2);
					if(NO_FLOOR_PRICE==FALSE)
						(*pos).name.AppendFormat(_T("\n%s%s"),theApp.CURRENCY_SYMBOL,str2);
					rs.GetFieldValue(_T("customer_id"),variant);
					if(variant.m_lVal==1)
						(*pos).isWechat=TRUE;
				}
				else{//搭台的桌,桌号为1000+head_id
					rs.GetFieldValue(_T("table_name"),strName);
					table.name.Format(_T("%s"),strName);
					rs.GetFieldValue(_T("rvc_center_id"),variant);
					table.rvc=variant.m_lVal;
					variant.m_lVal=0;
					rs.GetFieldValue(_T("raw_talbe"),variant);
					table.raw_id=variant.m_lVal;
					variant.m_lVal=0;
					rs.GetFieldValue(_T("customer_num"),variant);
					cur_gst+=variant.m_lVal;
					table.cur_gst=variant.m_lVal;
					variant.m_iVal=0;
					rs.GetFieldValue(_T("is_make"),variant);
					table.ismake=variant.m_iVal;
					rs.GetFieldValue(_T("start_time"),str2);
					table.open_time.Format(_T("%s"),str2);
					rs.GetFieldValue(_T("total"),str2);
					total_amount+=_wtof(str2);
					if(NO_FLOOR_PRICE==FALSE)
						table.name.AppendFormat(_T("\n%s%s"),theApp.CURRENCY_SYMBOL,str2);
					//table.ismake=0;
					table.status=1;
					table.seat_num=0;
					TableItem tmp;
					tmp.id=table.raw_id;
					pos=find(m_Tables.begin(),m_Tables.end(),tmp);
					m_Tables.insert(pos,table);
				}
				rs.MoveNext();
			}
			rs.Close();
		}
	}
	catch(CDBException* e)
	{
		LOG4CPLUS_ERROR(log_pos,(LPCTSTR)e->m_strError);
		e->Delete();
		return;
	}
	catch(...)
	{
		LOG4CPLUS_ERROR(log_pos,"Catch Exception GetLastError="<<GetLastError());
	}
}
/************************************************************************
* 函数介绍：显示当前页的桌子
* 输入参数：
* 输出参数：
* 返回值  ：
************************************************************************/
void FloorDlg::ShowCurrentPage()
{
	try{
		for(int i=0;i<m_Tables.size();i++)
		{
			TableItem table=m_Tables[i];
			if(table.id>999)
				continue;//超过范围的ID
			CRoundButton2* pButton2=(CRoundButton2*)GetDlgItem(IDC_TABLE0+table.id);
			if(pButton2)
			{
				pButton2->SetWindowText(table.name);
				pButton2->SetStrTop(table.party_table);
				if(table.status>0)
				{
					pButton2->SetIcon(m_icTableStatus[table.status],CRoundButton2::DOWN,FALSE);
				}
				else
					pButton2->SetIcon((HICON)NULL,CRoundButton2::DOWN,FALSE);
			}
		}
	}
	catch(...)
	{
		LOG4CPLUS_ERROR(log_pos,"Catch Exception GetLastError="<<GetLastError());
	}
}

void FloorDlg::OnBnClickedButtonChange()
{
	//((CPOSClientApp*)AfxGetApp())->m_pMain->SetActivePage(DLG_ORDER);
}
void FloorDlg::OnBnClickedButtonCancel()
{
	((CPOSClientApp*)AfxGetApp())->m_pMain->SetActivePage(DLG_LOGIN);
}
void FloorDlg::OnSize(UINT nType, int cx, int cy)
{
	CPosPage::OnSize(nType, cx, cy);
	cx=CreatButton::m_nFullWidth;
	cy=CreatButton::m_nFullHeight;
	if (m_logCtrl.m_hWnd)
	{
		m_logCtrl.MoveWindow((int)(cx*0.84),20,(int)(cx*0.12),25);
		m_time.MoveWindow((int)(cx*0.84),50,(int)(cx*0.12),25);
		m_time.GetWindowRect(&m_timeRect);    
		ScreenToClient(&m_timeRect);
	}
}
/************************************************************************
* 函数介绍：按时刷新时间显示
* 输入参数：
* 输出参数：
* 返回值  ：
************************************************************************/
void FloorDlg::OnTimer(UINT_PTR nIDEvent)
{
	if (nIDEvent==1000)
	{
		if(m_time.m_hWnd)
		{
			SYSTEMTIME st;
			CString strTime;
			GetLocalTime(&st);
			strTime.Format(_T("%02d:%02d"),st.wHour,st.wMinute);
			m_time.SetWindowText(strTime);
			InvalidateRect(m_timeRect); 
		}
	}
	else if (nIDEvent==1001)
	{
		CPOSClientApp* pApp=(CPOSClientApp*)AfxGetApp();
		if(pApp->m_pMain->GetActiveIndex()==DLG_FLOOR)
		{//只有在桌台界面才自动刷新
			FloorDlg::OnSetActive();
		}
	}
	CPosPage::OnTimer(nIDEvent);
}
/************************************************************************
* 函数介绍：开台
* 输入参数：
* 输出参数：
* 返回值  ：
************************************************************************/
void FloorDlg::OnBnClickedBeginTable()
{
	NumberInputDlg dlg;
	theLang.LoadString(dlg.m_strHint,IDS_INPUTTBL);
	if(dlg.DoModal()==IDOK)
	{
		int num=_wtoi(dlg.m_strNum);
		if (num<=0||num>MAX_TABLE_NUM)
		{
			POSMessageBox(IDS_TBLNOERROR);
			return;
		}
		m_nStatus=0;
		if(GetLock(num)==TRUE)
		{
			if(BeginTable(num,0)==FALSE)//开桌失败
			{
				ReleaseLock(num);
			}
		}
	}
}
/************************************************************************
* 函数介绍：调单
* 输入参数：
* 输出参数：
* 返回值  ：
************************************************************************/
void FloorDlg::OnBnClickedPickCheck()
{
	try
	{
		PickUpDlg dlg;
		dlg.pParentWnd=this;
		if(dlg.DoModal()==IDCANCEL)
			return;
		if (dlg.m_nIndex>=0)
		{
			((CPOSClientApp*)AfxGetApp())->m_nActiveCheck=dlg.m_Items[dlg.m_nIndex].check_id-1;
			int table_id=dlg.m_Items[dlg.m_nIndex].table_id;
			if(GetLock(table_id)==TRUE)
			{
				if(BeginTable(table_id,1)==FALSE)//开桌失败
					ReleaseLock(table_id);
			}
		}
	}
	catch(...)
	{
		LOG4CPLUS_ERROR(log_pos,"Catch Exception GetLastError="<<GetLastError());
	}
}
/************************************************************************
* 函数介绍：获取桌号的命名锁，调用后注意 RELEASE_LOCK
* 输入参数：
* 输出参数：
* 返回值  ：
************************************************************************/
BOOL FloorDlg::GetLock(int table_id)
{
	CString strSQL;
	CString strCount;
	OpenDatabase();
	CRecordset rs(&theDB);
	try{//尝试加锁
		strSQL.Format(_T("SELECT GET_LOCK('table%d',0);"),table_id);
		if(!rs.Open(-1,strSQL))
		{
			LOG4CPLUS_ERROR(log_pos,"CRecordset Open Failed");
			return FALSE;
		}
		rs.GetFieldValue((short)0,strCount);
		if (strCount!="1")
		{
			MSGBoxDlg dlg;
			theLang.LoadString(dlg.m_strText,IDS_TBLINOPERATE);
			if(theApp.m_strUserID==_T("999"))
			{
				dlg.m_nType=MB_YESNO;
				theLang.LoadString(dlg.m_strYes,IDS_FORCELOCK);
				if(dlg.DoModal()==IDOK)
				{
					strSQL.Format(_T("kill(SELECT IS_USED_LOCK('table%d'));"),table_id);
					theDB.ExecuteSQL(strSQL);
				}
			}
			else
			{
				dlg.m_nType=MB_OK;
				dlg.DoModal();
			}
			return FALSE;
		}
		rs.Close();
	}
	catch(CDBException* e)
	{
		LOG4CPLUS_ERROR(log_pos,(LPCTSTR)e->m_strError);
		e->Delete();
		return FALSE;
	}
	return TRUE;
}
BOOL FloorDlg::ReleaseLock(int table_id)
{
	try
	{
		CString strSQL;
		strSQL.Format(_T("SELECT RELEASE_LOCK('table%d');"),table_id);
		theDB.ExecuteSQL(strSQL);
	}
	catch(CDBException* e)
	{
		LOG4CPLUS_ERROR(log_pos,(LPCTSTR)e->m_strError);
		e->Delete();
		return FALSE;
	}
	return TRUE;
}
/************************************************************************
* 函数介绍：检查table_id是否在操作中
* 输入参数：
* 输出参数：
* 返回值  ：
************************************************************************/
BOOL FloorDlg::IsFreeLock(int table_id)
{
	CString strSQL;
	CString strCount;
	CRecordset rs(&theDB);
	try{
		strSQL.Format(_T("SELECT IS_FREE_LOCK('table%d');"),table_id);
		if(!rs.Open(-1,strSQL))
		{
			LOG4CPLUS_ERROR(log_pos,"CRecordset Open Failed");
			return FALSE;
		}
		rs.GetFieldValue((short)0,strCount);
		if (strCount!="1")
		{
			return FALSE;
		}
		rs.Close();
	}
	catch(CDBException* e)
	{
		LOG4CPLUS_ERROR(log_pos,(LPCTSTR)e->m_strError);
		e->Delete();
		return FALSE;
	}
	return TRUE;
}
/************************************************************************
* 函数介绍：调台
* 输入参数：
* 输出参数：
* 返回值  ：
************************************************************************/
void FloorDlg::OnBnClickedPickTable()
{
	NumberInputDlg dlg;
	theLang.LoadString(dlg.m_strHint,IDS_INPUTTBL);
	if(dlg.DoModal()==IDOK)
	{
		int num=_wtoi(dlg.m_strNum);
		if (num<=0||num>MAX_TABLE_NUM)
		{
			POSMessageBox(IDS_TBLNOERROR);
			return;
		}
		m_nStatus=0;
		((CPOSClientApp*)AfxGetApp())->m_nActiveCheck=0;
		if(GetLock(num)==TRUE)
		{
			if(BeginTable(num,1)==FALSE)//开桌失败
			{
				ReleaseLock(num);
			}
		}
	}
}
/************************************************************************
* 函数介绍：开台或者调台
* 输入参数：uTableID --桌位号
nType    --0：开台；1：调台；2:两者同时进行；3：退款
nIsTakeout	--是堂食还是外带
* 输出参数：
* 返回值  ：TRUE- 开桌成功；  FALSE- 没有开桌
************************************************************************/
BOOL FloorDlg::BeginTable(int table_id,int nType,int parent_table,int nIsTakeout)
{
	LOG4CPLUS_INFO(log_pos,"BeginTable TableID="<<table_id<<" nType="<<nType);
	CString strSQL;
	CString strCount;
	int preorderid=0;//预定id
	CString userid;
	if (OrderDlg::RequeastAuth(userid,_T("is_order"))!=0)
	{
		return FALSE;
	}
	try
	{
		CRecordset rs(&theDB);
		strSQL.Format(_T("SELECT COUNT(*) FROM order_head WHERE table_id=\'%d\'"),table_id);
		if(!rs.Open(-1,strSQL))
		{
			LOG4CPLUS_ERROR(log_pos,"CRecordset Open Failed");
			return FALSE;
		}
		rs.GetFieldValue((short)0,strCount);
		rs.Close();
		CPOSClientApp* pApp=(CPOSClientApp*)AfxGetApp();
		pApp->m_nTableId=table_id;
		pApp->m_nEatType=nIsTakeout;
		pApp->m_bRefund=FALSE;
		pApp->m_bDirty=TRUE;
		pApp->m_nRawTable=parent_table;
		//检查tables是否有这张桌
		int tid=table_id;
		if(parent_table>0)
			tid=parent_table;

		strSQL.Format(_T("SELECT * FROM tables LEFT JOIN consumption_limit ON tables.consumption_limit=")
			_T("consumption_limit.consumption_limit_id WHERE table_id=\'%d\';"),tid);
		if(!rs.Open(-1,strSQL))
		{
			LOG4CPLUS_ERROR(log_pos,"CRecordset Open Failed");
			return FALSE;
		}
		if(rs.IsEOF())
		{
			POSMessageBox(IDS_NOTFINDTBL);
			return FALSE;
		}
		else
		{
			CString strVal;
			rs.GetFieldValue(_T("table_name"),strVal);
			pApp->m_strTblName.Format(_T("%s"),strVal);
			CDBVariant variant;
			variant.m_lVal=0;
			rs.GetFieldValue(_T("rvc_center_id"),variant);
			pApp->m_nRVC=variant.m_lVal;
			variant.m_iVal=0;
			rs.GetFieldValue(_T("limit_type"),variant);
			pApp->m_nLimitType=variant.m_iVal;
			variant.m_fltVal=0;
			rs.GetFieldValue(_T("limit_amount"),variant);
			pApp->m_fLimitAmount=variant.m_fltVal;
			if (macrosInt[_T("SHOW_CONSUMPTION_LIMIT")]>0&&pApp->m_nLimitType>0)
			{
				CString str2,str;
				if(pApp->m_nLimitType==1)
					theLang.LoadString(str2,IDS_CONSUMLIMIT_TBL);
				else if(pApp->m_nLimitType==2)
					theLang.LoadString(str2,IDS_CONSUMLIMIT_GST);
				if(pApp->m_fLimitAmount>0)
				{
					str.Format(str2,pApp->m_fLimitAmount);
					POSMessageBox(str);
				}
			}
		}
		rs.Close();
		//查询RVC name
		strSQL.Format(_T("select rvc_center_name from rvc_center where rvc_center_id = %d"),pApp->m_nRVC);
		if(rs.Open(-1,strSQL))
		{
			if(!rs.IsEOF())
			{
				rs.GetFieldValue(_T("rvc_center_name"),pApp->m_strRVC);
			}
		}
		rs.Close();
		BOOL bNeedAuth=FALSE;
		if (strCount=="0")//数据库中没有座位的信息
		{
			if (nType==1)
			{//调台时，没有找到这个桌位
				POSMessageBox(IDS_NOTFINDTBL);
				return FALSE;
			}
			if (nType==3)
			{
				pApp->m_nGuests=0;
				pApp->m_bRefund=TRUE;
			}
			else
			{
				BOOL b_needGuest=TRUE;;
				//检查预定
				strSQL.Format(_T("SELECT * FROM pre_order WHERE pre_order_status=2 AND table_id=%d AND ABS(TIMESTAMPDIFF(MINUTE,NOW(),arrived_time))<=60 ORDER BY arrived_time")
					,table_id);
				rs.Open( CRecordset::forwardOnly,strSQL);
				if (!rs.IsEOF())
				{
					PreOrderDlg dlg;
					dlg.rs=&rs;
					if(dlg.DoModal()==IDCANCEL)
						return FALSE;
					preorderid=dlg.m_nPreorderId;
					pApp->m_nGuests=dlg.m_nGuestNum;
					if(pApp->m_nGuests>0)
						b_needGuest=FALSE;
				}
				rs.Close();
				if(b_needGuest)
				{
					if(macrosInt[_T("OPEN_TABLE_GUEST")]==1)
					{
						pApp->m_nGuests=1;
					}
					else
					{
						COpenTableDlg dlg;
						if(dlg.DoModal()==IDCANCEL)
						{
							return FALSE;
						}
						pApp->m_nGuests=_wtoi(dlg.m_strGst);
						if (pApp->m_nGuests>MAX_GUEST_NUM)
						{
							POSMessageBox(IDS_TOMANYG);
							return FALSE;
						}
						pApp->m_strChkName=dlg.m_strRemark;
					}
				}
			}
			//将开单信息存入数据库中
			strSQL.Format(_T("CALL update_checknum(%d);"),pApp->m_nDeviceId);
			theDB.ExecuteSQL(strSQL);
			strSQL.Format(_T("SELECT check_num FROM user_workstations WHERE workstations_id=%d;"),pApp->m_nDeviceId);
			rs.Open( CRecordset::forwardOnly,strSQL);
			CString strRet;
			if (!rs.IsEOF())
			{
				rs.GetFieldValue((short)0,strRet);
			}
			rs.Close();
			//对表加锁，保证原子性
			strSQL.Format(_T("lock table history_order_head READ ,order_head WRITE,order_head AS T2 READ,history_order_head AS T3 READ;"));
			theDB.ExecuteSQL(strSQL);
			long head_id=GetHeadID(macrosInt[_T("HEAD_BEGIN_ID")]);
			if(pApp->m_nTableId<0){//搭台开桌
				pApp->m_nTableId=head_id+1000;
				table_id=pApp->m_nTableId;
				strSQL.Format(_T("SELECT COUNT(*) FROM order_head WHERE raw_talbe=%d"),parent_table);
				int count=1;
				rs.Open( CRecordset::forwardOnly,strSQL);
				if (!rs.IsEOF())
				{
					rs.GetFieldValue((short)0,strRet);
					count=_wtoi(strRet)+1;
				}
				rs.Close();
				pApp->m_strTblName.AppendFormat(_T("-%d"),count);
			}
			
			strSQL.Format(_T("INSERT INTO order_head(order_head_id,check_number,table_id,table_name,check_id,customer_num,order_start_time,eat_type")
				_T(",open_employee_id,open_employee_name,pos_device_id,pos_name,rvc_center_id,rvc_center_name,check_name,raw_talbe)")
				_T(" VALUES(\'%d\',\'%s\',\'%d\',\'%s\',\'1\',\'%d\',NOW(),\'%d\',\'%s\',\'%s\',\'%d\',\'%s\',\'%d\',\'%s\',\'%s\',%d);"),
				head_id,strRet,pApp->m_nTableId,pApp->m_strTblName,pApp->m_nGuests,nIsTakeout,pApp->m_strUserID
				,pApp->m_strUserName,pApp->m_nDeviceId,pApp->m_strDeviceName,pApp->m_nRVC,pApp->m_strRVC,pApp->m_strChkName,parent_table);
			theDB.ExecuteSQL(strSQL);
			//解锁
			strSQL.Format(_T("unlock tables;"));
			theDB.ExecuteSQL(strSQL);
			//自动插入茶位费
			if(nIsTakeout==0&&nType!=3)
			{
				pApp->m_bDirty=2;//需重设已结状态
				AddDefaultItem();
			}
			//存储桌态信息
			strSQL.Format(_T("UPDATE tables SET table_status=2 WHERE table_id=%d;"),table_id);
			theDB.ExecuteSQL(strSQL);
		}
		else if(nType==0||nType==3)//开台时，该桌位已经被人点了
		{
			POSMessageBox(IDS_ALREADYOPEN);
			return FALSE;
		}
		else
		{
			bNeedAuth=TRUE;
		}

		CDBVariant variant;
		strSQL.Format(_T("SELECT * FROM order_head WHERE table_id=%d"),table_id);
		if(!rs.Open(-1,strSQL))
		{
			LOG4CPLUS_ERROR(log_pos,"CRecordset Open Failed");
			return FALSE;
		}
		if(bNeedAuth)
		{//调台检查权限
			int nAllCheck=0;
			theApp.m_privilegeMap.Lookup(_T("authority_4"),nAllCheck);
			if(nAllCheck==0)
			{//没有权限，检查开桌员工是否相同
				rs.GetFieldValue(_T("open_employee_id"),variant);
				int uid=_wtoi(theApp.m_strUserID);
				if(uid!=variant.m_iVal)
				{//开桌员工是否下班
					CRecordset rs2(&theDB);
					strSQL.Format(_T("SELECT end_date FROM employee WHERE employee_id=\'%d\'"),variant.m_iVal);
					rs2.Open( CRecordset::forwardOnly,strSQL);
					CDBVariant clockout;
					if (!rs2.IsEOF())
					{
						rs2.GetFieldValue(_T("end_date"),clockout);
					}
					rs2.Close();
					if(clockout.m_dwType==DBVT_NULL)
					{//初始化，未下班
						POSMessageBox(IDS_NOTALLOWTBL);
						return FALSE;
					}
					rs.GetFieldValue(_T("order_start_time"),variant);
					CTime open_t(variant.m_pdate->year,variant.m_pdate->month,variant.m_pdate->day,variant.m_pdate->hour,variant.m_pdate->minute,variant.m_pdate->second);
					CTime shift_t(clockout.m_pdate->year,clockout.m_pdate->month,clockout.m_pdate->day,clockout.m_pdate->hour,clockout.m_pdate->minute,clockout.m_pdate->second);
					if(shift_t<open_t)
					{
						POSMessageBox(IDS_NOTALLOWTBL);
						return FALSE;
					}
				}
			}
		}
		if (nType!=1)
		{//调单
			pApp->m_nActiveCheck=0;
		}
		rs.GetFieldValue(_T("table_name"),pApp->m_strTblName);
		rs.GetFieldValue(_T("order_head_id"),variant);
		pApp->m_nOrderHeadid=variant.m_lVal;
		rs.GetFieldValue(_T("check_number"),variant);
		pApp->m_nCheckNum=variant.m_lVal;
		rs.GetFieldValue(_T("check_name"),pApp->m_strChkName);
		rs.GetFieldValue(_T("customer_num"),variant);
		pApp->m_nGuests=variant.m_lVal;
		rs.GetFieldValue(_T("order_start_time"),variant);
		pApp->m_strBeginDate.Format(_T("%04d-%02d-%02d"),variant.m_pdate->year,variant.m_pdate->month,variant.m_pdate->day);
		pApp->m_strBeginTime.Format(_T("%02d:%02d:%02d"),variant.m_pdate->hour,variant.m_pdate->minute,variant.m_pdate->second);
		pApp->m_strEndTime.Empty();
		rs.GetFieldValue(_T("edit_time"),variant);
		if (variant.m_dwType!=DBVT_NULL)
		{//存在edit_time字段，说明是反结帐的单
			rs.GetFieldValue(_T("order_end_time"),pApp->m_strEndTime);
		}
		rs.GetFieldValue(_T("party_id"),variant);
		if (variant.m_dwType==DBVT_NULL)
			pApp->m_nPartyId=-1;
		else
			pApp->m_nPartyId=variant.m_lVal;
		rs.Close();
// 		if (preorderid!=0)
// 		{
// 			//插入预定菜品
// 			pApp->m_bDirty=2;//需重设已结状态
// 			CTypedPtrList<CPtrList,OrderDetail *> *pOrderList=&((CPOSClientApp*)AfxGetApp())->m_orderList;
// 			//更新已点菜品信息
// 			strSQL.Format(_T("SELECT * FROM pre_order_detail WHERE pre_order_id=\'%d\';"),preorderid);
// 			if(!rs.Open( CRecordset::forwardOnly,strSQL))
// 				return FALSE;
// 			while(!pOrderList->IsEmpty())
// 			{
// 				OrderDetail* item=pOrderList->GetTail();
// 				pOrderList->RemoveTail();
// 				delete item;
// 			}
// 
// 			while(!rs.IsEOF())
// 			{
// 				CDBVariant variant;
// 				OrderDetail* order=new OrderDetail;
// 				memset(order,0,sizeof(OrderDetail));
// 				order->n_checkID=1;
// 				variant.m_lVal=0;
// 				rs.GetFieldValue(_T("condiment_belong_item"),variant);
// 				order->n_belong_item=variant.m_lVal;
// 				CString name;
// 				rs.GetFieldValue(_T("menu_item_name"),name);
// 				wcsncpy_s(order->item_name,name,63);
// 				rs.GetFieldValue(_T("price"),variant);
// 				order->item_price=variant.m_fltVal;
// 				variant.m_fltVal=0;
// 				rs.GetFieldValue(_T("quantity"),variant);
// 				order->quantity=variant.m_fltVal;
// 				ComputeTotalPrice(order);
// 				rs.GetFieldValue(_T("menu_item_id"),variant);
// 				order->item_id=variant.m_lVal;
// 				pOrderList->AddTail(order);
// 				rs.MoveNext();
// 			}
// 			rs.Close();
// 		}
		LOG4CPLUS_INFO(log_pos,"order_head_id="<<theApp.m_nOrderHeadid);
		pApp->m_pMain->SetActivePage(DLG_ORDER);
	}
	catch(CDBException* e)
	{
		LOG4CPLUS_ERROR(log_pos,_T("CDBException: ")<<(LPCTSTR)e->m_strError);
		AfxMessageBox(e->m_strError,MB_ICONEXCLAMATION);
		e->Delete();
		return FALSE;
	}
	catch(...)
	{
		LOG4CPLUS_ERROR(log_pos,"Catch Exception GetLastError="<<GetLastError());
		return FALSE;
	}
	return TRUE;
}
/************************************************************************
* 函数介绍：从数据库初始化折扣、服务费的属性(开台消费使用)
* 输入参数：
* 输出参数：
* 返回值  ：
************************************************************************/
BOOL FloorDlg::ReadDiscountService(OrderDetail* order,int id)
{
	CString strSQL;
	CDBVariant variant;
	CRecordset rs(&theDB);
	if (order->item_id==ITEM_ID_DISCOUNT)
	{//折扣
		strSQL.Format(_T("SELECT * FROM discount_service WHERE discount_service_id=%d;"),id);
		if(!rs.Open( CRecordset::forwardOnly,strSQL))
			return FALSE;
		if(!rs.IsEOF())
		{
			variant.m_lVal=0;
			rs.GetFieldValue(_T("discount_service_id"), variant);
			order->n_discount_type=variant.m_lVal;
			CString name;
			rs.GetFieldValue(_T("display_name"), name);
			if (name.GetLength()==0)
				rs.GetFieldValue(_T("discount_service_name"), name);	
			wcsncpy_s(order->item_name,name,63);
		}
		else
		{
			rs.Close();
			return FALSE;
		}

	}
	else if (order->item_id==ITEM_ID_SERVICE)
	{//服务费
		strSQL.Format(_T("SELECT * FROM service_tip WHERE service_tip_id=%d;"),id);
		if(!rs.Open( CRecordset::forwardOnly,strSQL))
			return FALSE;
		if(!rs.IsEOF())
		{
			variant.m_lVal=0;
			rs.GetFieldValue(_T("service_tip_id"), variant);
			order->n_discount_type=variant.m_lVal;
			CString name;
			rs.GetFieldValue(_T("display_name"), name);
			if (name.GetLength()==0)
				rs.GetFieldValue(_T("service_tip_name"), name);
			wcsncpy_s(order->item_name,name,63);
			variant.m_boolVal=FALSE;
			rs.GetFieldValue(_T("discount_over_threshold"), variant);
			order->noDiscount=variant.m_boolVal;
		}
		else
		{
			rs.Close();
			return FALSE;
		}
	}
	//公共属性部分
	order->discount_percent=-1;//部分折
	rs.GetFieldValue(_T("preset"), variant);
	if(variant.m_boolVal)
	{
		variant.m_boolVal=FALSE;
		rs.GetFieldValue(_T("select_discount"), variant);
		if(variant.m_boolVal==FALSE)
		{//部分打折不生效(整单折)
			CString strVal;
			rs.GetFieldValue(_T("percent"), strVal);
			order->discount_percent=_wtof(strVal);
			variant.m_boolVal=FALSE;
		}
	}
	else
	{
		CString strVal;
		rs.GetFieldValue(_T("amount"), strVal);
		order->total_price=_wtof(strVal);
		order->item_price=order->total_price;
		if (order->item_id==ITEM_ID_DISCOUNT)
			order->total_price*=(-1);
	}
	variant.m_lVal=0;
	rs.GetFieldValue(_T("menu_level_class"), variant);
	order->round_type=variant.m_lVal;
	rs.Close();
	return TRUE;
}

/************************************************************************
* 函数介绍：开台关联点菜，自动插入茶位费等菜品
* 输入参数：
* 输出参数：
* 返回值  ：
************************************************************************/
void FloorDlg::AddDefaultItem()
{
	try{
		CPOSClientApp* pApp=(CPOSClientApp*)AfxGetApp();
		CRecordset rs1(&theDB);
		CTypedPtrList<CPtrList,OrderDetail *> *pOrderList=&((CPOSClientApp*)AfxGetApp())->m_orderList;
		CString strSQL;
		while(!pOrderList->IsEmpty())
		{
			OrderDetail* item=pOrderList->GetTail();
			pOrderList->RemoveTail();
			delete item;
		}
		strSQL.Format(_T("select * from order_detail_default where extend_1 in (select order_default_groupid from ")
			_T("order_default_group where  (serving_place_class=-1 or serving_place_class in (select ")
			_T(" rvc_class_id from serving_rvc_class where rvc_center_id=%d)) and (serving_period_class=-1")
			_T(" or serving_period_class in (select period_class_id from serving_period_class where ")
			_T(" period in(select period_id from periods where (start_time<end_time and CURTIME() BETWEEN start_time and end_time) or (start_time>=end_time and (start_time<CURTIME() or end_time>CURTIME()))))));")
			,pApp->m_nRVC);
		if(!rs1.Open( CRecordset::forwardOnly,strSQL))
			return ;
		while(!rs1.IsEOF())
		{
			CDBVariant variant;
			rs1.GetFieldValue(_T("menu_item_id"),variant);
			OrderDetail* order=new OrderDetail;
			memset(order,0,sizeof(OrderDetail));
			order->item_id=variant.m_lVal;
			order->n_checkID=1;

			if(order->item_id<0)
			{
				rs1.GetFieldValue(_T("discount_service_id"),variant);
				if(ReadDiscountService(order,variant.m_lVal)==FALSE)
				{
					rs1.MoveNext();
					continue;
				}
			}
			else//菜品
			{
				strSQL.Format(_T("SELECT * FROM menu_item LEFT JOIN menu_item_class ON menu_item.class_id")
					_T("=menu_item_class.item_class_id  WHERE item_id=%d;"),variant.m_lVal);
				rs1.GetFieldValue(_T("unit_id"),variant);
				if (variant.m_lVal<1)
					variant.m_lVal=1;
				CString strUnit,strPrice;
				strPrice.Format(_T("price_%d"),variant.m_lVal);
				strUnit.Format(_T("unit_%d"),variant.m_lVal);
				variant.m_fltVal=0;
				rs1.GetFieldValue(_T("quantity"),variant);
				order->quantity=variant.m_fltVal;
				rs1.GetFieldValue(_T("is_cus_num"),variant);
				if(variant.m_iVal==1)//按客人数收取
					order->quantity=pApp->m_nGuests*order->quantity;
				CRecordset rs(&theDB);
				if(!rs.Open(CRecordset::forwardOnly,strSQL))
				{
					rs1.MoveNext();
					continue ;
				}
				if(!rs.IsEOF())
				{
					CString name;
					rs.GetFieldValue(_T("item_name1"),name);
					wcsncpy_s(order->item_name,name,63);
					variant.m_fltVal=0;
					rs.GetFieldValue(strPrice,variant);
					order->item_price=variant.m_fltVal;
					rs.GetFieldValue(strUnit,name);
					wcsncpy_s(order->unit,name,9);
					rs.GetFieldValue(_T("tax_group"), variant);
					order->tax_group=variant.m_iVal;

					ComputeTotalPrice(order);
					variant.m_iVal=9;
					rs.GetFieldValue(_T("discount_itemizer"),variant);
					order->n_discount_type=variant.m_iVal;
					variant.m_iVal=9;
					rs.GetFieldValue(_T("service_itemizer"),variant);
					order->n_service_type=variant.m_iVal;
				}
				rs.Close();
			}
			pOrderList->AddTail(order);
			rs1.MoveNext();
		}
		rs1.Close();
	}
	catch(CDBException* e)
	{
		LOG4CPLUS_ERROR(log_pos,(LPCTSTR)e->m_strError);
		AfxMessageBox(e->m_strError,MB_ICONEXCLAMATION);
		e->Delete();
	}
}
/************************************************************************
* 函数介绍：获取唯一的headid，保证连续性
* 输入参数：
* 输出参数：
* 返回值  ：
************************************************************************/
long FloorDlg::GetHeadID(long beginId)
{
	CString strSQL;
	try
	{
		CRecordset rs(&theDB);
		CString strId;
		strSQL.Format(_T("SELECT MIN(order_head_id) + 1 FROM (SELECT order_head_id FROM order_head UNION SELECT order_head_id FROM history_order_head) ")
			_T(" T1 WHERE (T1.order_head_id>=%d) AND (T1.order_head_id+1) NOT IN (SELECT order_head_id FROM order_head AS T2 UNION ")
			_T(" SELECT order_head_id FROM history_order_head AS T3 WHERE T3.order_head_id>=%d)")
			,beginId,beginId);
		if(!rs.Open(-1,strSQL))
		{
			LOG4CPLUS_ERROR(log_pos,"CRecordset Open Failed");
			return 0;
		}
		if (!rs.IsEOF())
		{
			rs.GetFieldValue((short)0,strId);
			if (!strId.IsEmpty())
				return _wtol(strId);
		}
		return beginId;
	}
	catch(CDBException* e)
	{
		LOG4CPLUS_ERROR(log_pos,(LPCTSTR)e->m_strError);
		AfxMessageBox(e->m_strError,MB_ICONEXCLAMATION);
		e->Delete();
		return FALSE;
	}
	catch(...)
	{
		LOG4CPLUS_ERROR(log_pos,"Catch Exception GetLastError="<<GetLastError());
		return FALSE;
	}
}
/************************************************************************
* 函数介绍：点击了桌台，开台或查看桌态
* 输入参数：
* 输出参数：
* 返回值  ：
************************************************************************/
void FloorDlg::OnTableClicked(UINT uID)
{
	int table_id=uID-IDC_TABLE0;
	//LOG4CPLUS_ERROR(log_pos,"table_id="<<table_id);
	if(m_nStatus==0)
	{
		if(GetLock(table_id)==TRUE)
		{
			if(BeginTable(table_id,2)==FALSE)//开桌失败
				ReleaseLock(table_id);
		}
	}
	else
	{
		TableStatusDlg dlg;
		dlg.m_nTblID=table_id;
		dlg.DoModal();
	}

}
/************************************************************************
* 函数介绍：查看桌态
* 输入参数：
* 输出参数：
* 返回值  ：
************************************************************************/
void FloorDlg::OnBnClickedStatus()
{
	//从数据库中查询桌态
	if(m_nStatus==0)
	{//设置为查看桌态
		m_nStatus=1;
		CRoundButton2* pButton=(CRoundButton2*)GetDlgItem(IDC_BUTTON_STATUS);
		if(pButton)
		{
			pButton->SetImages(m_bpStatusPressed[0],m_bpStatusPressed[1],false);
			pButton->SetStrTop(_T(""));//强制刷新
		}
	}
	else
	{
		m_nStatus=0;
		CRoundButton2* pButton=(CRoundButton2*)GetDlgItem(IDC_BUTTON_STATUS);
		if(pButton)
		{
			pButton->SetImages(m_bpStatusNormal[0],m_bpStatusNormal[1],false);
			pButton->SetStrTop(_T(""));//强制刷新
		}
	}
}
/************************************************************************
* 函数介绍：查看已结账单
* 输入参数：
* 输出参数：
* 返回值  ：
************************************************************************/
void FloorDlg::OnBnClickedEditCheck()
{
	CString userid;
	int auth=OrderDlg::RequeastAuth(userid,_T("authority_3"));
	if (auth!=0)
		return;
	theApp.m_pMain->SetActivePage(DLG_VIEWCHK);
}
/************************************************************************
* 函数介绍：获取详细的点菜信息
* 输入参数：
* 输出参数：
* 返回值  ：
************************************************************************/
int FloorDlg::GetOrderDetail(CString strSQL,CTypedPtrList<CPtrList,OrderDetail *> *pOrderList)
{
	CRecordset rs( &theDB);
	if(!rs.Open( CRecordset::forwardOnly,strSQL))
		return -1;
	if (rs.GetRecordCount()==0)
	{
		return -1;
	}
	while(!rs.IsEOF())
	{
		CDBVariant variant;
		//忽略退掉的菜品
		rs.GetFieldValue(_T("is_return_item"),variant);
		if(variant.m_boolVal)
		{
			rs.MoveNext();
			continue;
		}
		OrderDetail* order=new OrderDetail;
		memset(order,0,sizeof(OrderDetail));
		COrderPage::readOrderDetail(rs,order);
		//如果前一个是配料的所属
		if(order->n_belong_item!=0&&pOrderList->IsEmpty()==FALSE)
		{
			OrderDetail* pBefore=pOrderList->GetTail();
			if (pBefore!=NULL&&pBefore->n_belong_item<=0)
			{
				pBefore->b_hascondiment=TRUE;
			}
		}
		pOrderList->AddTail(order);
		rs.MoveNext();
	}
	rs.Close();
	return 0;
}
/************************************************************************
* 函数介绍：外带
* 输入参数：
* 输出参数：
* 返回值  ：
************************************************************************/
void FloorDlg::EmptyTable()
{
	theApp.m_nOrderHeadid=0;
	theApp.m_nTableId=-1;
	theApp.m_strTblName=_T("QS");
	theApp.m_nRVC=1;
	theApp.m_nGuests=1;
	theApp.m_bQuickService=TRUE;
	theApp.m_bQuickOnce=TRUE;
	theApp.m_bDirty=1;
	theApp.m_nEatType=TYPE_TAKEOUT;
	theApp.m_bRefund=FALSE;
	theApp.m_nRawTable=0;
	theApp.m_nLimitType=0;
	theApp.m_fLimitAmount=0;
}
void FloorDlg::OnBnClickedTakeOut()
{
	//输入外卖信息
	if(macrosInt[_T("NO_AUTO_CUSTOMER")]==0)
	{
		CustomerSearchDlg2 dlg;
		dlg.m_bIgnore=TRUE;
		if(dlg.DoModal()==IDOK)
		{
			COrderPage::m_strTAInfo=dlg.m_strResult;
			COrderPage::bNewTainfo=TRUE;
		}
		else if(dlg.m_bIgnore==TRUE)
			return;
	}
	//点外卖单
	EmptyTable();
	theApp.m_pMain->SetActivePage(DLG_QUICK_ORDER);
	
}

void FloorDlg::OnMsgIncall()
{
	LOG4CPLUS_INFO(log_pos,"OnMsgIncall");
	EmptyTable();
	theApp.m_pMain->SetActivePage(DLG_QUICK_ORDER);
}
/************************************************************************
* 函数介绍：堂食
* 输入参数：
* 输出参数：
* 返回值  ：
************************************************************************/
void FloorDlg::OnBnClickedEatIn()
{
	NumberInputDlg dlg;
	theLang.LoadString(IDS_INPUTTBL,dlg.m_strHint);
	if(dlg.DoModal()==IDOK)
	{
		int num=_wtoi(dlg.m_strNum);
		if (num<=0||num>MAX_TABLE_NUM)
		{
			POSMessageBox(IDS_TBLNOERROR);
			return;
		}
		m_nStatus=0;
		if(GetLock(num)==TRUE)
		{
			if(BeginTable(num,0,0,TYPE_DINE)==FALSE)//开桌失败
				ReleaseLock(num);
		}
	}
}
/************************************************************************
* 函数介绍：退款
* 输入参数：
* 输出参数：
* 返回值  ：
************************************************************************/
void FloorDlg::OnBnClickedRefund()
{
	CString userid;
	WCHAR authUser[10]={0};
	int auth=OrderDlg::RequeastAuth(userid,_T("refund"),1,authUser);
	if (auth!=0)
		return;
	theApp.m_strUserID.Format(_T("%s"),userid);
	theApp.m_strUserName.Format(_T("%s"),authUser);
	CFloorChooseDlg dlg;
	if(dlg.DoModal()==IDCANCEL)
		return;
	{
		int num=dlg.m_nTableId;
		if (num<=0)
		{
			POSMessageBox(IDS_TBLNOERROR);
			return;
		}
		CVoidReasonDlg reasonDlg;
		if(reasonDlg.DoModal()==IDCANCEL)
			return;
		theApp.m_strRefundReason=reasonDlg.m_strReason;
		theApp.m_nGuests=0;
		m_nStatus=0;
		if(GetLock(num)==TRUE)
		{
			if(BeginTable(num,3)==FALSE)//开桌失败
				ReleaseLock(num);
		}
		if(theApp.m_VCR.IsOpen())
		{
			theApp.m_VCR.Print(_T("REFUND\n"));
		}
		CPOSClientApp::CriticalLogs(OPR_REFUND,theApp.m_strTblName);
	}
}
/************************************************************************
* 函数介绍：预定
* 输入参数：
* 输出参数：
* 返回值  ：
************************************************************************/
void FloorDlg::OnBnClickedReserve()
{
	ReserveDlg* pDlg=new ReserveDlg();
	pDlg->m_hParent=this;
	pDlg->Create(IDD_RESERVE,theApp.m_pMain);
	pDlg->ShowWindow(SW_SHOW);
	pDlg->SetFocus();
}
/************************************************************************
* 函数介绍：经理功能
* 输入参数：
* 输出参数：
* 返回值  ：
************************************************************************/
void FloorDlg::OnBnClickedFunction()
{
	CString userid;
	int auth=OrderDlg::RequeastAuth(userid,_T("manager_privilege"));
	if(auth!=0)
		return;
	theApp.m_pMain->SetActivePage(DLG_MANAGER); 
}
/************************************************************************
* 函数介绍：通知厨房，叫起、等叫等信息
* 输入参数：
* 输出参数：
* 返回值  ：
************************************************************************/
void FloorDlg::OnBnClickedNotifyKitchen()
{
	NotifyKitchenDlg dlg;
	dlg.DoModal();
}
void FloorDlg::OnBnClickedRadio(UINT uID)
{
	int rvc_id=uID-IDC_CLASS_BUTTON;
	if (rvc_id<=0)
	{
		m_nCurrentRvc=0;
		OnSetActive();
	}
	else
		UpdateRvc(rvc_id,TRUE);
}
void FloorDlg::UpdateRvc(int rvc_id,BOOL reset)
{
	m_RVCTables.clear();
	if(rvc_id==0)
	{
		m_RVCTables=m_Tables;
	}
	else
	{
		vector<TableItem>::iterator iter;
		for (iter=m_Tables.begin();iter!=m_Tables.end();iter++)
		{
			if (rvc_id==iter->rvc)
			{
				m_RVCTables.push_back(*iter);
			}
		}
	}
	m_nCurrentRvc=rvc_id;
	if(reset)
		m_nCurrentPage=0;
	m_nPageCount=(int)ceil((float)m_RVCTables.size()/m_nPageSize);
	ShowCurrentPage();
}
void FloorDlg::OnBnClickedParty()
{
	PickUpDlg dlg;
	dlg.pParentWnd=this;
	dlg.m_nType=1;
	if(dlg.DoModal()==IDCANCEL)
		return;
	//确定宴会桌
	CString strSQL;
	long party_id,new_party_id;
	int table_nums=0;
	party_id=dlg.m_Items[dlg.m_nIndex].order_head_id;
	strSQL.Format(_T("SELECT table_id FROM party_table WHERE party_id=%d"),party_id);
	CRecordset rs( &theDB);
	CRecordset rs1( &theDB);
	CDBVariant variant;
	std::vector<int> tableList;
	if(!rs.Open(CRecordset::forwardOnly,strSQL))
		return;
	while(!rs.IsEOF())
	{
		rs.GetFieldValue(_T("table_id"),variant);
		strSQL.Format(_T("SELECT order_head_id FROM order_head WHERE table_id=\'%d\';"),variant.m_lVal);
		rs1.Open(-1,strSQL);
		if (rs1.GetRecordCount()==0)
		{
			tableList.push_back(variant.m_lVal);
		}
		rs1.Close();
		table_nums++;//记录总的桌子数目
		rs.MoveNext();
	}
	rs.Close();
	//查看party_id是否已被使用
	new_party_id=party_id;
	strSQL.Format(_T("SELECT MIN(party_id)+1 FROM (SELECT party_id FROM order_head)")
	_T("T1 WHERE T1.party_id>=%d AND (T1.party_id+1) NOT IN (SELECT party_id FROM order_head)"),party_id);
	rs.Open(CRecordset::forwardOnly,strSQL);
	if (!rs.IsEOF())
	{
		CString strValue;
		rs.GetFieldValue((short)0,strValue);
		if (!strValue.IsEmpty())
		{
			new_party_id=_wtol(strValue);
		}
	}
	rs.Close();
	//查宴会信息
	strSQL.Format(_T("SELECT table_price,table_num,sale_employee,cus_num,employee_last_name FROM parties ")
		_T("LEFT JOIN employee ON parties.sale_employee=employee.employee_id WHERE party_id=%d"),party_id);
	if(!rs.Open(CRecordset::forwardOnly,strSQL))
		return;
	CString table_price;
	if (rs.IsEOF())
		return;
	//缓存登录用户
	CString b4strUserID=theApp.m_strUserID;
	CString b4strUserName=theApp.m_strUserName;
	rs.GetFieldValue(_T("table_price"),table_price);
	rs.GetFieldValue(_T("table_num"),variant);
	if (variant.m_lVal>table_nums)
	{
		table_nums=variant.m_lVal;
	}
	rs.GetFieldValue(_T("sale_employee"),theApp.m_strUserID);
	rs.GetFieldValue(_T("employee_last_name"),theApp.m_strUserName);
	rs.GetFieldValue(_T("cus_num"),variant);
	theApp.m_nGuests=variant.m_lVal;
	rs.Close();
	//换桌
	CFloorChooseDlg fDlg;
	fDlg.m_tableList=tableList;
	fDlg.m_nNoUsedTable=1;
	fDlg.m_bMultiSelect=TRUE;
	CString str2;
	theLang.LoadString(str2,IDS_PARTYCHOOSE);
	fDlg.m_strTitle.Format(str2,table_nums);
	if(fDlg.DoModal()==IDCANCEL)
		return;
	//一个都没选
	if (fDlg.m_tableList.size()==0)
	{
		return;
	}
	CPOSClientApp* pApp=(CPOSClientApp*)AfxGetApp();
	pApp->m_nEatType=TYPE_DINE;
	pApp->m_bRefund=FALSE;
	pApp->m_bDirty=TRUE;
	//统一开台
	for (int i=0;i<fDlg.m_tableList.size();i++)
	{
		//是否已开台
		strSQL.Format(_T("SELECT order_head_id FROM order_head WHERE table_id=\'%d\';"),fDlg.m_tableList[i]);
		rs.Open(-1,strSQL);
		if (rs.GetRecordCount()>0)
		{
			rs.Close();
			continue;
		}
		rs.Close();
		//检查tables是否有这张桌
		strSQL.Format(_T("SELECT * FROM tables WHERE table_id=\'%d\';"),fDlg.m_tableList[i]);
		rs.Open(-1,strSQL);
		if(rs.IsEOF())
		{
			rs.Close();
			continue;
		}
		else
		{
			rs.GetFieldValue(_T("table_name"),pApp->m_strTblName);
			variant.m_iVal=0;
			rs.GetFieldValue(_T("rvc_center_id"),variant);
			pApp->m_nRVC=variant.m_iVal;
		}
		rs.Close();
		//查询RVC name
		strSQL.Format(_T("select rvc_center_name from rvc_center where rvc_center_id = %d"),pApp->m_nRVC);
		if(rs.Open(-1,strSQL))
		{
			if(!rs.IsEOF())
			{
				rs.GetFieldValue(_T("rvc_center_name"),pApp->m_strRVC);
			}
		}
		rs.Close();
		//将开单信息存入数据库中
		strSQL.Format(_T("CALL update_checknum(%d);"),pApp->m_nDeviceId);
		theDB.ExecuteSQL(strSQL);
		strSQL.Format(_T("SELECT check_num FROM user_workstations WHERE workstations_id=%d;"),pApp->m_nDeviceId);
		rs.Open( CRecordset::forwardOnly,strSQL);
		CString strRet;
		if (!rs.IsEOF())
		{
			rs.GetFieldValue((short)0,strRet);
		}
		rs.Close();
		//对表加锁，保证原子性
		strSQL.Format(_T("lock table history_order_head READ ,order_head WRITE,order_head AS T2 READ,history_order_head AS T3 READ;"));
		theDB.ExecuteSQL(strSQL);
		long head_id=GetHeadID(macrosInt[_T("HEAD_BEGIN_ID")]);
		strSQL.Format(_T("INSERT INTO order_head(order_head_id,check_number,table_id,table_name,check_id,customer_num,order_start_time,eat_type")
			_T(",open_employee_id,open_employee_name,pos_device_id,pos_name,rvc_center_id,rvc_center_name,check_name,party_id,should_amount)")
			_T(" VALUES(\'%d\',\'%s\',\'%d\',\'%s\',\'1\',\'%d\',NOW(),\'%d\',%s,\'%s\',\'%d\',\'%s\',\'%d\',\'%s\',\'%s\',%d,'%s');"),
			head_id,strRet,fDlg.m_tableList[i],pApp->m_strTblName,pApp->m_nGuests,FALSE,pApp->m_strUserID
			,pApp->m_strUserName,pApp->m_nDeviceId,pApp->m_strDeviceName,pApp->m_nRVC,pApp->m_strRVC,pApp->m_strChkName,new_party_id,table_price);
		theDB.ExecuteSQL(strSQL);
		pApp->m_nGuests=0;//只有第一张桌填人数
		//解锁
		strSQL.Format(_T("unlock tables;"));
		theDB.ExecuteSQL(strSQL);
		//加菜
		strSQL.Format(_T("SELECT * FROM party_item WHERE party_id=%d;"),party_id);
		OrderDetail* order=new OrderDetail;
		memset(order,0,sizeof(OrderDetail));
		order->n_checkID=1;
		if(!rs.Open( CRecordset::forwardOnly,strSQL))
			continue ;
		while(!rs.IsEOF())
		{
			rs.GetFieldValue(_T("menu_item_id"),variant);
			order->item_id=variant.m_lVal;
			CString name;
			rs.GetFieldValue(_T("item_course_name"),name);
			wcsncpy_s(order->item_name,name,63);
			rs.GetFieldValue(_T("description"),name);
			wcsncpy_s(order->description,name,99);
			variant.m_fltVal=0;
			rs.GetFieldValue(_T("price"),name);
			order->item_price=_wtof(name);
			rs.GetFieldValue(_T("unit"),name);
			wcsncpy_s(order->unit,name,9);
			rs.GetFieldValue(_T("num"),name);
			order->quantity=_wtof(name);
			ComputeTotalPrice(order);
			COrder_Detail rsDetail(&theDB);
			rsDetail.Open(CRecordset::snapshot, NULL, CRecordset::appendOnly);
			rsDetail.AddNew();
			OrderDlg::SetRsDetail(rsDetail,order);
			rsDetail.m_order_head_id=head_id;
			rsDetail.Update();
			rsDetail.Close();
			rs.MoveNext();
		}
		rs.Close();
		//存储桌态信息
		strSQL.Format(_T("UPDATE tables SET table_status=2 WHERE table_id=%d;"),fDlg.m_tableList[i]);
		theDB.ExecuteSQL(strSQL);
		delete order;
	}
	//更新party状态
	strSQL.Format(_T("UPDATE parties SET status=1 WHERE party_id=%d"),party_id);
	theDB.ExecuteSQL(strSQL);
	//恢复原用户
	theApp.m_strUserID=b4strUserID;
	theApp.m_strUserName=b4strUserName;
	OnSetActive();
}

void FloorDlg::OnBnClickedCleanTable()
{
	CFloorChooseDlg dlg;
	if(dlg.DoModal()==IDCANCEL)
		return;
	int num=dlg.m_nTableId;
	CString strSQL;
	if(IsFreeLock(dlg.m_nTableId)==FALSE)
		return;
	long headId=0;
	strSQL.Format(_T("SELECT * FROM order_head WHERE table_id=%d"),dlg.m_nTableId);
	CRecordset rs( &theDB);
	if(rs.Open(-1,strSQL))
	{
		if (!rs.IsEOF())
		{
			CDBVariant variant;
			rs.GetFieldValue(_T("order_head_id"),variant);
			headId=variant.m_lVal;
			strSQL.Format(_T("CALL flush_order(%d,%d);"),headId,dlg.m_nTableId);
			theDB.ExecuteSQL(strSQL);
		}
		else
		{
			strSQL.Format(_T("UPDATE tables SET table_status=0 WHERE table_id=%d"),dlg.m_nTableId);
			theDB.ExecuteSQL(strSQL);
		}
		rs.Close();
	}
	
	FloorDlg::OnSetActive();
}
HBRUSH FloorDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);
	if(nCtlColor==CTLCOLOR_STATIC)
	{
		pDC->SetTextColor(RGB(255,255,255));
		pDC->SetBkMode(TRANSPARENT);
		return (HBRUSH)::GetStockObject(NULL_BRUSH); //返回此画刷可以使静态文本透明
	}
	return   hbr; 
}