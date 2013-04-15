// LineCutPosDialog.cpp : implementation file
//

#include "stdafx.h"
#include "LineCutPosDialog.h"

#include "acedads.h"
#include "accmd.h"
#include <adscodes.h>

#include <adsdlg.h>

#include <dbapserv.h>

#include <dbregion.h>

#include <gepnt3d.h>

//symbol table
#include <dbsymtb.h>

#include <acdocman.h>

//3D Object
#include <dbsol3d.h>

// Hatch
#include <dbhatch.h>

// Leader
#include <dblead.h>
#include <dbmleader.h>

#include <ArxWrapper.h>

#include <ArxCustomObject.h>

#include <GlobalDataConfig.h>

#include <LMAUtils.h>

using namespace com::guch::assistant::arx;

// LineCutPosDialog dialog

CString LineCutPosDialog::m_CutLayerName = L"";
AcDbObjectIdArray LineCutPosDialog::m_CutObjects = AcDbObjectIdArray();
wstring LineCutPosDialog::m_CutHatchStyle = L"NET";

IMPLEMENT_DYNAMIC(LineCutPosDialog, CAcUiDialog)

LineCutPosDialog::LineCutPosDialog(CWnd* pParent /*=NULL*/)
	: CAcUiDialog(LineCutPosDialog::IDD, pParent),
	m_Direction(0),
	m_strOffset(0)
{
}

LineCutPosDialog::~LineCutPosDialog()
{
}

BOOL LineCutPosDialog::OnInitDialog()
{
	//和页面交互数据
	CAcUiDialog::OnInitDialog();

	//默认X轴选中
	m_DirectionX.SetCheck(BST_CHECKED);
	m_Direction = 1;

	//默认偏移为0
	m_EditOffset.SetWindowTextW(L"0.00");

	//加载图片
	m_PickCutPosButton.AutoLoad();

	return TRUE;
}

void LineCutPosDialog::DoDataExchange(CDataExchange* pDX)
{
	CAcUiDialog::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_X, m_DirectionX);
	DDX_Control(pDX, IDC_Y, m_DirectionY);
	DDX_Control(pDX, IDC_Z, m_DirectionZ);

	DDX_Control(pDX, IDC_OFFSET, m_EditOffset);

	DDX_Control(pDX, IDC_BUTTON_PICKCUT,m_PickCutPosButton);
}

BEGIN_MESSAGE_MAP(LineCutPosDialog, CAcUiDialog)
	ON_BN_CLICKED(IDC_X, &LineCutPosDialog::OnBnClickedX)
	ON_BN_CLICKED(IDC_Y, &LineCutPosDialog::OnBnClickedY)
	ON_BN_CLICKED(IDC_Z, &LineCutPosDialog::OnBnClickedZ)
	ON_BN_CLICKED(IDC_BUTTON_PICKCUT, &LineCutPosDialog::OnBnPickCutPos)
	ON_BN_CLICKED(IDOK, &LineCutPosDialog::OnBnClickedOk)
END_MESSAGE_MAP()

void LineCutPosDialog::OnBnClickedX()
{
	m_Direction = 1;
}

void LineCutPosDialog::OnBnClickedY()
{
	m_Direction = 2;
}

void LineCutPosDialog::OnBnClickedZ()
{
	m_Direction = 3;
}

void LineCutPosDialog::OnBnPickCutPos()
{
	// Hide the dialog and give control to the editor
	BeginEditorCommand();

	CString temp;
	ads_point pt;

	// Get a point
	if (acedGetPoint(NULL, _T("\n选取切割点: "), pt) == RTNORM) 
	{
		// If the point is good, continue
		CompleteEditorCommand();

		if( m_Direction == 1 )
		{
			temp.Format(_T("%g"), pt[X]);
		}
		else if ( m_Direction == 2 )
		{
			temp.Format(_T("%g"), pt[Y]);
		}
		else if ( m_Direction == 3 )
		{
			temp.Format(_T("%g"), pt[Z]);
		}
	}
	else 
	{
		// otherwise cancel the command (including the dialog)
		CancelEditorCommand();
	}

	m_EditOffset.SetWindowTextW(temp.GetBuffer());
	UpdateData(FALSE);
}

void LineCutPosDialog::OnBnClickedOk()
{
	//得到用户输入的数据
	UpdateData(FALSE);

	//首先恢复视图
	CutBack();

	//得到切面
	GenerateCutPlane();

	//得到转换矩阵
	GenerateTransform();

	//生成切图
	GenerateCutRegion();

	//显示该图层
	ShowCutRegion();

	//关闭对话框
	CAcUiDialog::OnOK();
}

void LineCutPosDialog::GenerateCutPlane()
{
	//创建切面
	CString offset;
	m_EditOffset.GetWindowTextW(offset);

	if( offset.GetLength())
		m_strOffset = _wtoi(offset);

	if( m_Direction == 1)
	{
		m_CutLayerName.Format(L"与X轴垂直偏移量为【%d】的切面",m_strOffset);
		m_CutPlane.set(AcGePoint3d(m_strOffset,0,0),AcGeVector3d(1,0,0));
	}
	else if( m_Direction == 2)
	{
		m_CutLayerName.Format(L"与Y轴垂直偏移量为【%d】的切面",m_strOffset);
		m_CutPlane.set(AcGePoint3d(0,m_strOffset,0),AcGeVector3d(0,1,0));
	}
	else if( m_Direction == 3)
	{
		m_CutLayerName.Format(L"与Z轴垂直偏移量为【%d】的切面",m_strOffset);
		m_CutPlane.set(AcGePoint3d(0,0,m_strOffset),AcGeVector3d(0,0,1));
	}

#ifdef DEBUG
	acutPrintf(L"\n切面为【%s】",m_CutLayerName.GetBuffer());
#endif
}

void LineCutPosDialog::GenerateTransform()
{
	//进行相应的转换
	if( m_Direction == 1 )
	{
		acutPrintf(L"\n切面与X轴垂直,先偏移至YZ平面，然后翻转到XZ平面，最后翻转到XY平面");

		//偏移至YZ平面
		m_MoveMatrix.setToTranslation(AcGeVector3d(-m_strOffset,0,0));

		//进行翻转到XZ平面
		m_RotateMatrixFirst = AcGeMatrix3d::rotation( -ArxWrapper::kRad90, AcGeVector3d::kZAxis, AcGePoint3d::kOrigin);

		//进行翻转到XY平面
		m_RotateMatrixSecond = AcGeMatrix3d::rotation( -ArxWrapper::kRad90, AcGeVector3d::kXAxis, AcGePoint3d::kOrigin);
	}
	else if ( m_Direction == 2 )
	{
		acutPrintf(L"\n切面与Y轴垂直,先偏移至XZ平面，然后进行翻转到XY平面");

		//偏移至XZ平面
		m_MoveMatrix.setToTranslation(AcGeVector3d(0,-m_strOffset,0));

		//进行翻转到XY平面
		m_RotateMatrixFirst = AcGeMatrix3d::rotation( -ArxWrapper::kRad90, AcGeVector3d::kXAxis, AcGePoint3d::kOrigin);
	} 
	else if ( m_Direction == 3 )
	{
		acutPrintf(L"\n切面与Z轴垂直，直接偏移至XY平面即可");
	
		//进行偏移
		m_MoveMatrix.setToTranslation(AcGeVector3d(0,0,-m_strOffset));
	}
}

void LineCutPosDialog::GenerateCutRegion()
{
	ArxWrapper::LockCurDoc();

	//得到当前的实体文件管理器
	LineEntityFile* pLineFile = LineEntityFileManager::GetCurrentLineEntryFile();
	if( pLineFile == NULL )
	{
		acutPrintf(L"\n没能找到管线文件管理器，检查下吧！");
		return;
	}

	//得到实体列表
	LineList* lineList = pLineFile->GetList();
	if( lineList == NULL )
	{
		acutPrintf(L"\n当前文件中没有管线，无需进行切图！");
		return;
	}

	//遍历实体文件管理，针对每一条实体进行切图
	LineIterator lineIter = lineList->begin();

	for(;lineIter != lineList->end();
		lineIter++)
	{
#ifdef DEBUG
		acutPrintf(L"\n对管线【%s】进行切图！",(*lineIter)->m_LineName.c_str());
#endif
		if( *lineIter != NULL )
			GenerateCutRegion(*lineIter);
	}

	ArxWrapper::UnLockCurDoc();
}

void LineCutPosDialog::GenerateCutRegion(LineEntity* lineEntry)
{
	PointList* pointList = lineEntry->m_PointList;
	if( pointList == NULL )
	{
		acutPrintf(L"\n该管线没有线段，无需切图！");
		return;
	}

	//创建截图区域的注释
	double markOffset = 0;
	if( lineEntry->m_LineBasiInfo->mShape == GlobalData::LINE_SHAPE_CIRCLE )
	{
		double radius;
		acdbDisToF(lineEntry->m_LineBasiInfo->mSize.mRadius.c_str(), -1, &radius);
		markOffset = ( radius * 1.5 ) / 1000 ;
	}
	else// if ( lineEntry->m_LineBasiInfo->mShape == GlobalData::LINE_SHAPE_SQUARE )
	{
		double width,height;
		acdbDisToF(lineEntry->m_LineBasiInfo->mSize.mWidth.c_str(), -1, &width);
		acdbDisToF(lineEntry->m_LineBasiInfo->mSize.mHeight.c_str(), -1, &height);
		markOffset = ( width / 2 + height / 2 ) / 1000;
	}

	//对所有的线段进行遍历
	PointIter pointIter = pointList->begin();
	for(;pointIter != pointList->end();pointIter++)
	{
		if( pointIter == pointList->begin() )
			continue;

		PointEntity* pointEntity = (*pointIter);

		if( pointEntity == NULL )
		{
			acutPrintf(L"\n该线段不合法，需要注意！");
			continue;
		}

#ifdef DEBUG
		acutPrintf(L"\n对第【%d】条线段进行切图！",pointEntity->m_PointNO);
#endif

		GenerateCutRegion( pointEntity, markOffset );
	}
}

void LineCutPosDialog::GenerateCutRegion(PointEntity* pointEntity, double markOffset)
{
	//得到其对应的管线实体对象
	AcDbObjectId lineEntityId = pointEntity->m_DbEntityCollection.GetWallLineEntity();
	if( !lineEntityId.isValid() )
	{
		acutPrintf(L"\n当前线段没有对应的管线壁实体，尝试用管线切图！");

		lineEntityId = pointEntity->m_DbEntityCollection.GetLineEntity();
		if( !lineEntityId.isValid() )
		{
			acutPrintf(L"\n当前线段没有对应的管线实体，不必切图！");
			return;
		}
	}

	AcDbEntity* pLineObj;
	Acad::ErrorStatus es = acdbOpenAcDbEntity(pLineObj, lineEntityId, AcDb::kForRead);

	if( es == Acad::eOk )
	{
		LMALineDbObject* pLMALine = LMALineDbObject::cast(pLineObj);

		if( pLMALine == NULL )
		{
			acutPrintf(L"\n当前线段不是有效的辅助系统管理的实体，不考虑切图！");
			return;
		}

		//得到实体与切面相切的截面
		AcDbRegion *pSelectionRegion = NULL;
		pLMALine->getSection(m_CutPlane, pSelectionRegion);

		//得到注释的中心点
		AcGePoint3d centerPoint = pLMALine->GetCutCenter(m_CutPlane);

		//设置注释的内容
		CString markContent;

		//如果用户设置了标注内容,则用标注内容#段号
		if( pointEntity->m_DbEntityCollection.mCategoryData->mCutMark.length() != 0
			&& pointEntity->m_DbEntityCollection.mCategoryData->mCutMark != L"无" )
		{
			markContent.Format(L"%s#%d",pointEntity->m_DbEntityCollection.mCategoryData->mCutMark.c_str(), pointEntity->m_PointNO);
		}
		else //未定义标注内容，则用管线名称#段号
		{
			markContent.Format(L"%s#%d",pointEntity->m_DbEntityCollection.mLayerName.c_str(), pointEntity->m_PointNO);
		}

		//关闭实体
		pLMALine->close();

		if( pSelectionRegion )
		{
			acutPrintf(L"\n生成切面区域");

			//创建切面所在的图层
			if( ArxWrapper::createNewLayer(m_CutLayerName.GetBuffer()) == false )
				return;

			//将截面加入到模型空间
			AcDbObjectId regionId = ArxWrapper::PostToModelSpace(pSelectionRegion,m_CutLayerName.GetBuffer());
			m_CutObjects.append(regionId);

			//创建该界面的填充区域
			AcDbObjectId hatchId = CreateHatch(regionId);
			m_CutObjects.append(hatchId);

			//转移到XY平面
			AcDbObjectIdArray transformObjs;
			transformObjs.append(regionId);
			transformObjs.append(hatchId);
			TransformToXY( transformObjs );

			AcDbObjectId mLeaderId = CreateMLeader(centerPoint, markContent.GetBuffer(), markOffset);
			m_CutObjects.append(mLeaderId);
		}
		else
		{
			acutPrintf(L"\n切面与该管线（阻隔体）无相交区域！");
		}
	}
	else
	{
		acutPrintf(L"\n打开管线实体失败！");
		rxErrorMsg(es);
	}
}

AcDbObjectId LineCutPosDialog::CreateHatch( AcDbObjectId entityId )
{
	acutPrintf(L"\n创建填充图案,样式为【%s】",m_CutHatchStyle.c_str());

	if( !entityId.isValid() )
		return 0;

	AcGeVector3d normal = m_CutPlane.normal();

	AcDbObjectIdArray objIds;
	objIds.append(entityId);

	Acad::ErrorStatus es;
	AcDbHatch *pHatch = new AcDbHatch();

	// 设置填充平面
	pHatch->setNormal(normal);

	// 设置高度
	pHatch->setElevation(m_strOffset);

	// 设置关联性
	pHatch->setAssociative(true);

	// 设置填充图案
	pHatch->setPattern(AcDbHatch::kPreDefined, m_CutHatchStyle.c_str());

	// 添加填充边界
	es = pHatch->appendLoop(AcDbHatch::kExternal, objIds);

	// 显示填充对象
	es = pHatch->evaluateHatch();

	// 添加到模型空间
	return ArxWrapper::PostToModelSpace(pHatch, m_CutLayerName.GetBuffer());
}

AcDbObjectId LineCutPosDialog::CreateMLeader(const AcGePoint3d& start, const wstring& content, double markOffset)
{
	//标注的起点
	AcGePoint3d startPoint(start.x, start.y, 0);

	//进行相应的转换
	{
		if( m_Direction == 1 )
		{
#ifdef DEBUG
			acutPrintf(L"\n切面与X轴垂直,把Z轴位置转换为Y，把Y轴位置转换为X");
#endif
			startPoint.y = start.z;
			startPoint.x = start.y;
		}
		else if ( m_Direction == 2 )
		{
#ifdef DEBUG
			acutPrintf(L"\n切面与Y轴垂直，把Z轴位置转换为Y轴位置");
#endif		
			startPoint.y = start.z;
		} 
	}

	//折点为编译6个单位，且位置在左下方
	AcGePoint3d endPointCorner(startPoint.x + markOffset, startPoint.y - markOffset, 0);
		
	//保证标注字体在允许范围内
	double textHeight = markOffset;
	if( textHeight > 2 )
		textHeight = 2;

	AcGePoint3d endPointText(endPointCorner.x + textHeight, endPointCorner.y, 0);

	AcDbObjectId textId;
	{
		//设置标注的文字
		AcDbMText* mtext = new AcDbMText;
		mtext->setContents(content.c_str());
		mtext->setAttachment(AcDbMText::AttachmentPoint::kBottomLeft);
		mtext->setLocation(endPointText);

		//文字的高度为半径（或者高/宽）
		acutPrintf(L"\n【%s】的高度为【%0.2lf】",content.c_str(), textHeight/2);
		mtext->setTextHeight(textHeight/2);

		textId = ArxWrapper::PostToModelSpace(mtext, m_CutLayerName.GetBuffer());
		m_CutObjects.append(textId);
	}

	AcDbLeader* leader = new AcDbLeader;

	//设置标注的内容
	{
		leader->appendVertex(startPoint);
		leader->appendVertex(endPointCorner);
		leader->appendVertex(endPointText);

		leader->attachAnnotation(textId);
		leader->evaluateLeader();
	}

	//添加到模型空间中
	return ArxWrapper::PostToModelSpace(leader, m_CutLayerName.GetBuffer());
}

void LineCutPosDialog::TransformToXY(AcDbObjectIdArray entityIds)
{
	Acad::ErrorStatus es;
	AcDbEntity* pEntity = NULL;

	//进行相应的转换
	for( int i = 0; i < entityIds.length(); i++ )
	{
		AcDbObjectId objId = entityIds[i];

		ArxWrapper::LockCurDoc();

		es = acdbOpenAcDbEntity(pEntity, objId, AcDb::kForWrite);

		if( es != Acad::eOk )
		{
			acutPrintf(L"\n打开要移至XY平面的实体失败");
			rxErrorMsg(es);

			ArxWrapper::UnLockCurDoc();
			continue;
		}

		if( m_Direction == 1 )
		{
			//偏移至YZ平面
			pEntity->transformBy(m_MoveMatrix);

			//进行翻转到XZ平面
			pEntity->transformBy(m_RotateMatrixFirst);

			//进行翻转到XY平面
			pEntity->transformBy(m_RotateMatrixSecond);
		}
		else if ( m_Direction == 2 )
		{
			//偏移至XZ平面
			pEntity->transformBy(m_MoveMatrix);

			//进行翻转到XY平面
			pEntity->transformBy(m_RotateMatrixFirst);
		} 
		else if ( m_Direction == 3 )
		{
			//进行偏移即可
			pEntity->transformBy(m_MoveMatrix);
		}

		pEntity->close();

		ArxWrapper::UnLockCurDoc();
	}
}

void LineCutPosDialog::ShowCutRegion()
{
	//只显示切面图层
	ArxWrapper::ShowLayer(m_CutLayerName.GetBuffer());

	//切换视图
	//ArxWrapper::ChangeView(m_Direction);

	acedCommand(RTSTR, _T("._-VIEW"), RTSTR, L"TOP", 0);
}

void LineCutPosDialog::Reset()
{
	acutPrintf(L"\n存在的切图为【%s】！",m_CutLayerName.GetBuffer());

	if( m_CutLayerName.GetLength() > 0 )
	{
		acutPrintf(L"\n首先锁定该文档");
		ArxWrapper::LockCurDoc();

		//acutPrintf(L"\n恢复WCS视窗");
		acedCommand(RTSTR, _T("UCS"), RTSTR, L"W", 0);

		acutPrintf(L"\n删除切图相关的对象");
		CutBack();

		acutPrintf(L"\n删除切图所在的图层");
		if( ArxWrapper::DeleteLayer(m_CutLayerName.GetBuffer(),true) )
		{
			acutPrintf(L"\n初始化设置");
			m_CutLayerName.Format(L"");
		}

		acutPrintf(L"\n显示其他图层");
		ArxWrapper::ShowLayer(L"");

		acutPrintf(L"\n解除文档锁定");
		ArxWrapper::UnLockCurDoc();
	}
	else
	{
		acutPrintf(L"\n当前系统内没有切图！");
	}
}

void LineCutPosDialog::CutBack()
{
	while( m_CutObjects.length() )
	{
		AcDbObjectId objId = m_CutObjects.at(0);

		if( objId.isValid() )
		{
			ArxWrapper::RemoveDbObject(objId);
		}

		m_CutObjects.removeAt(0);
	}
}

// LineCutPosDialog message handlers
