#include <string>

#include <ArxWrapper.h>
#include <GlobalDataConfig.h>
#include <LineCategoryItemData.h>
#include <LineConfigDataManager.h>
#include <LineEntryData.h>

#include <Hlr.h>

#include "acedads.h"
#include "accmd.h"
#include <adscodes.h>

#include <adsdlg.h>

#include <dbapserv.h>

#include <dbregion.h>

#include <gepnt3d.h>

//symbol table
#include <dbsymtb.h>

//3D Object
#include <dbsol3d.h>

// Hatch
#include <dbhatch.h>

// Leader
#include <dblead.h>
#include <dbmleader.h>

// Custom object
#include <ArxCustomObject.h>

#include <LMAException.h>

#include <LMAUtils.h>

void TestHatch();
void TestText();
void TestLeader();
void TestMLeader();
void TestLayer();
void TestArxPath();

using namespace std;
using namespace com::guch::assistant::arx;
using namespace com::guch::assistant::data;
using namespace com::guch::assistant::config;
using namespace com::guch::assistant::exception;

const double ArxWrapper::kPi       = 3.14159265358979323846;
const double ArxWrapper::kHalfPi   = 3.14159265358979323846 / 2.0;
const double ArxWrapper::kTwoPi	  = 3.14159265358979323846 * 2.0;
const double ArxWrapper::kRad45    = 3.14159265358979323846 / 4.0;
const double ArxWrapper::kRad90    = 3.14159265358979323846 / 2.0;
const double ArxWrapper::kRad135   = (3.14159265358979323846 * 3.0) / 4.0;
const double ArxWrapper::kRad180   = 3.14159265358979323846;
const double ArxWrapper::kRad270   = 3.14159265358979323846 * 1.5;
const double ArxWrapper::kRad360   = 3.14159265358979323846 * 2.0;

extern wstring gLmaArxLoadPath;

#ifdef AUTOCAD2008
const wstring HLR_MODULE_NAME = L"AsdkHlrApi17.dbx";
#endif

#ifdef AUTOCAD2013
const wstring HLR_MODULE_NAME = L"AsdkHlrApi19.dbx";
#endif

/**
 * 加载依赖的库
 **/
void ArxWrapper::LoadDependencyLibrary()
{
	wstring hlrModule = gLmaArxLoadPath + L"\\" + HLR_MODULE_NAME;

	//----- We need the HLR engine loaded
	if ( !acrxServiceIsRegistered (AsdkHlrApiServiceName) )
		acrxLoadModule(hlrModule.c_str(), false, true) ;
	
	//----- bump the reference count
	acrxLoadModule(hlrModule.c_str(), false, false) ;
}

/**
 * 卸载依赖的库
 **/
void ArxWrapper::UnLoadDependencyLirarby()
{
	wstring hlrModule = gLmaArxLoadPath + wstring(L"\\") + HLR_MODULE_NAME;
	acrxUnloadModule(hlrModule.c_str()) ;
}

/**
 * 将对象放置在命名词典中
 **/
AcDbObjectId ArxWrapper::PostToNameObjectsDict(AcDbObject* pNameObj,const wstring& key )
{
	//首先锁闭文档
	LockCurDoc();

	AcDbObjectId newNameObjId;

	try
	{
		AcDbDictionary *pNamedDict,*pKeyDict;
		acdbHostApplicationServices()->workingDatabase()->
			getNamedObjectsDictionary(pNamedDict, AcDb::kForWrite);

		// Check to see if the dictionary we want to create is
		// already present. If not, create it and add
		// it to the named object dictionary.
		//
		AcDbObjectId keyDictId;
		if (pNamedDict->getAt(key.c_str(), (AcDbObject*&) pKeyDict,
			AcDb::kForWrite) == Acad::eKeyNotFound)
		{
			pKeyDict = new AcDbDictionary;
			pNamedDict->setAt(key.c_str(), pKeyDict, keyDictId);
		}
		else
		{
			keyDictId = pKeyDict->id();
		}

		pNamedDict->close();
		pKeyDict->close();

		// New objects to add to the new dictionary, then close them.
		LineDBEntity* pLineEntry = LineDBEntity::cast(pNameObj);

		if( pLineEntry )
		{
			Acad::ErrorStatus es = acdbOpenObject(pKeyDict, keyDictId, AcDb::kForWrite);

			if (es != Acad::eOk)
			{
				acutPrintf(L"\n重新打开词典添加失败！");
				rxErrorMsg(es);
			}
			else
			{
				acutPrintf(L"\n添加管线【%s】到命名词典",pLineEntry->pImplemention->m_LineName.c_str());
				es = pKeyDict->setAt(pLineEntry->pImplemention->m_LineName.c_str(), pNameObj, newNameObjId);

				if (es != Acad::eOk)
				{
					acutPrintf(L"\n添加失败！");
					rxErrorMsg(es);
				}

				pKeyDict->close();
				pNameObj->close();
			}
		}
	}
	catch(const Acad::ErrorStatus es)
	{
		acutPrintf(L"\n操作词典发生异常！");
		rxErrorMsg(es);
	}

	UnLockCurDoc();

	return newNameObjId;
}

/**
 * 将对象放置从命名词典中删除
 **/
bool ArxWrapper::DeleteFromNameObjectsDict(AcDbObjectId objToRemoveId,const wstring& key )
{
	LockCurDoc();

	try
	{
		AcDbDictionary *pNamedDict,*pKeyDict;
		acdbHostApplicationServices()->workingDatabase()->
			getNamedObjectsDictionary(pNamedDict, AcDb::kForWrite);

		// Check to see if the dictionary we want to create is
		// already present. If not, create it and add
		// it to the named object dictionary.
		//
		if (pNamedDict->getAt(key.c_str(), (AcDbObject*&) pKeyDict,
			AcDb::kForWrite) == Acad::eKeyNotFound)
		{
			acutPrintf(L"\n命名词典没有集合【%s】",key.c_str());
			pNamedDict->close();
			UnLockCurDoc();
			return false;
		}

		pNamedDict->close();

		if( objToRemoveId.isValid() )
		{
			// Get an iterator for the ASDK_DICT dictionary.
			AcDbDictionaryIterator* pDictIter= pKeyDict->newIterator();

			LineDBEntity *pLineEntry = NULL;
			for (; !pDictIter->done(); pDictIter->next()) 
			{
				// Get the current record, open it for read, and
				Acad::ErrorStatus es = pDictIter->getObject((AcDbObject*&)pLineEntry, AcDb::kForWrite);

				// if ok
				if (es == Acad::eOk && pLineEntry && pLineEntry->id() == objToRemoveId )
				{
					acutPrintf(L"\n从命名词典删除对象【%s】",pLineEntry->pImplemention->m_LineName.c_str());
					
					pLineEntry->erase();
					pLineEntry->close();

					break;
				}
			}

			delete pDictIter;
		}

		pKeyDict->close();
	}
	catch(const Acad::ErrorStatus es)
	{
		acutPrintf(L"\n操作词典发生异常！");
		rxErrorMsg(es);
	}

	UnLockCurDoc();

	return true;
}

/**

/**
 * 将对象放置从命名词典中读出来
 **/
void ArxWrapper::PullFromNameObjectsDict()
{
	AcDbDictionaryPointer pNamedobj;
	// use a smart pointer to access the objects, the destructor will close them automatically
	pNamedobj.open(acdbHostApplicationServices()->workingDatabase()->namedObjectsDictionaryId(), AcDb::kForRead);
	// if ok
	if (pNamedobj.openStatus() == Acad::eOk)
	{
		AcDbObjectId dictId;
		// get at the dictionary entry itself
		Acad::ErrorStatus es = pNamedobj->getAt(LineEntity::LINE_ENTRY_LAYER.c_str(), dictId);
		// if ok
		if (es == Acad::eOk)
		{
			// now open it for read
			AcDbDictionaryPointer pDict(dictId, AcDb::kForRead);
			// if ok
			if (pDict.openStatus() == Acad::eOk)
			{
				// Get an iterator for the ASDK_DICT dictionary.
				AcDbDictionaryIterator* pDictIter= pDict->newIterator();

				LineEntity *pLineEntry = NULL;
				for (; !pDictIter->done(); pDictIter->next()) 
				{
					// Get the current record, open it for read, and
					es = pDictIter->getObject((AcDbObject*&)pLineEntry, AcDb::kForRead);
					// if ok
					if (es == Acad::eOk)
					{
						//pLineEntry->m_pDbEntry->close();
					}
				}
				delete pDictIter;
			}

			// no need to close the dicts as we used smart pointers
		}
	}
}

/**
 * 将对象放置在模型空间中的某一层上
 **/
AcDbObjectId ArxWrapper::PostToModelSpace(AcDbEntity* pEnt,const wstring& layerName )
{
	AcDbObjectId entId;
	Acad::ErrorStatus es;

	if( !pEnt )
		return 0;

	AcDbBlockTable *pBlockTable(NULL);
	AcDbBlockTableRecord *pBlockTableRecord(NULL);

	LockCurDoc();

	try
	{
		//打开块表数据库
		es = acdbHostApplicationServices()->workingDatabase()->getBlockTable(pBlockTable, AcDb::kForWrite);
		if( es != Acad::eOk )
			throw ErrorException(L"打开块表记录失败");

		//得到模型空间
		es = pBlockTable->getAt(ACDB_MODEL_SPACE, pBlockTableRecord, AcDb::kForWrite);
		if( es != Acad::eOk )
			throw ErrorException(L"打开模块空间记录失败");

		//加入实体
		es = pBlockTableRecord->appendAcDbEntity(entId, pEnt);
		if( es != Acad::eOk )
			throw ErrorException(L"添加实体记录失败");

		es = pEnt->setLayer(layerName.c_str());
		if( es != Acad::eOk )
			throw ErrorException(L"设置实体所在的图层失败");

		//关闭实体
		pEnt->close();

		acutPrintf(L"\n成功添加到图层【%s】",layerName.c_str());
	}
	catch(const Acad::ErrorStatus es)
	{
		acutPrintf(L"\n操作数据库发生异常！");
		rxErrorMsg(es);
	}
	catch( const ErrorException& e)
	{
		acutPrintf(L"\n创建新的实体记录失败【%s】",e.errMsg.c_str());
		rxErrorMsg(es);
	}

	if( pBlockTableRecord != NULL )
		pBlockTableRecord->close();

	if( pBlockTable != NULL )
		pBlockTable->close();

	UnLockCurDoc();

	return entId;
}

Acad::ErrorStatus ArxWrapper::RemoveDbObject(AcDbObjectId id)
{
	Acad::ErrorStatus es = Acad::eOk;

	if( id.isValid())
	{
		AcDbEntity* ent;
        es = acdbOpenAcDbEntity(ent, id, AcDb::kForWrite);

        if (es == Acad::eOk) 
		{
            ent->erase();
            ent->close();
			acutPrintf(L"\n成功删除.");
        }
		else
		{
			acutPrintf(L"\n打开数据库对象失败.");
			rxErrorMsg(es);
		}
    }
	else
	{
		acutPrintf(L"\n数据库对象不合法.");
	}

	return es;
}

Adesk::Boolean ArxWrapper::ShowDbObject( AcDbObjectId& objectId, AcDb::Visibility show )
{
	Acad::ErrorStatus es = Acad::eOk;

	if( objectId.isValid())
	{
		AcDbEntity* ent;
        es = acdbOpenAcDbEntity(ent, objectId, AcDb::kForWrite);

        if (es == Acad::eOk) 
		{
			ent->setVisibility(show);
			ent->setColorIndex(2);
			ent->close();

			acutPrintf(L"\n成功%s.", show == AcDb::kVisible ? L"显示" : L"隐藏" );
        }
		else
		{
			acutPrintf(L"\n在设置对象可见性时，打开数据库对象失败.");
			rxErrorMsg(es);
		}
    }
	else
	{
		acutPrintf(L"\n在设置对象可见性时，数据库对象不合法.");
	}

	return Adesk::kTrue;
}

Acad::ErrorStatus ArxWrapper::LockCurDoc()
{
	if (acDocManager->curDocument() == NULL)
		return Acad::eOk;	// this wasn't associated with a document

	Acad::ErrorStatus es = Acad::eOk;

	if ((acDocManager->curDocument()->myLockMode() & AcAp::kWrite) == 0) {
		es = acDocManager->lockDocument(acDocManager->curDocument(), AcAp::kWrite);
		if (es == Acad::eOk)
			es = Acad::eOk;
		else {
			ASSERT(0);
			rxErrorMsg(es);
			return es;
		}
	}

	return es;
}

Acad::ErrorStatus ArxWrapper::UnLockCurDoc()
{
	if (acDocManager->curDocument() == NULL)
		return Acad::eOk;	// this wasn't associated with a document

	Acad::ErrorStatus es = Acad::eOk;

	if ((acDocManager->curDocument()->myLockMode() & AcAp::kWrite) != 0) {
		es = acDocManager->unlockDocument(acDocManager->curDocument());
		if (es == Acad::eOk)
			es = Acad::eOk;
		else {
			ASSERT(0);
			rxErrorMsg(es);
			return es;
		}
	}

	return es;
}

//得到两个实体是否有交集
AcDb3dSolid* ArxWrapper::GetInterset( AcDbEntity* pEntityA, AcDbEntity* pEntityB )
{
	if( pEntityA == NULL || pEntityB == NULL )
		return false;

	AcDb3dSolid* pCheck3dSolid = AcDb3dSolid::cast(pEntityA);
	AcDb3dSolid* pCompare3dSolid = AcDb3dSolid::cast(pEntityB);

	if( pCheck3dSolid == NULL || pCompare3dSolid == NULL )
	{
		acutPrintf(L"\n得到交集的对象不是合法的3D对象!");
		return NULL;
	}

	//判断是否有相交的部分
	Adesk::Boolean boolInter;
	AcDb3dSolid *commonVolumeSolid;
	Acad::ErrorStatus es = pCheck3dSolid->checkInterference(pCompare3dSolid,Adesk::kTrue,boolInter,commonVolumeSolid);

	if( es == Acad::eOk)
	{
		if(boolInter == Adesk::kFalse)
		{
			acutPrintf(L"\n两个3D对象无交集!");
			return NULL;
		}
		else
		{
			acutPrintf(L"\n两个3D对象存在交集!");
			if( commonVolumeSolid )
			{
				return commonVolumeSolid;
			}
			else
			{
				acutPrintf(L"\n没有创建交集对象，可能出错了!");
				return NULL;
			}
		}
	}
	else
	{
		acutPrintf(L"\n得到两个3D对象的交集失败!");
		rxErrorMsg(es);

		return NULL;
	}
}


/**
 * 将对象从模型空间中的某一层上删除
 **/
Acad::ErrorStatus ArxWrapper::RemoveFromModelSpace(const wstring& layerName )
{
#ifdef DEBUG
	acutPrintf(L"\n删除图层【%s】上的所有对象！",layerName.c_str());
#endif

	try
	{
		if( layerName.length() == 0 )
			return Acad::eOk;

		LockCurDoc();

		//打开块表数据库
		AcDbBlockTable *pBlockTable;
		acdbHostApplicationServices()->workingDatabase()
			->getBlockTable(pBlockTable, AcDb::kForWrite);

		//得到模型空间
		AcDbBlockTableRecord *pBlockTableRecord;
		pBlockTable->getAt(ACDB_MODEL_SPACE, pBlockTableRecord,
		AcDb::kForWrite);

		//遍历数据库
		AcDbBlockTableRecordIterator* iter;
		Acad::ErrorStatus es = pBlockTableRecord->newIterator(iter);
        if (es != Acad::eOk) 
		{
			acutPrintf(L"\n打开数据库失败了");
            rxErrorMsg(es);
            pBlockTableRecord->close();
        }
        else 
		{
            AcDbEntity* ent;
            for (; !iter->done(); iter->step()) 
			{
                if (iter->getEntity(ent, AcDb::kForWrite) == Acad::eOk) 
				{
#ifdef DEBUG
					acutPrintf(L"\n找到对象【%p】,所在图层为【%s】",ent,ent->layer());
#endif

					if( wstring(ent->layer()) == layerName || wstring(ent->layer()).length() == 0 )
					{
#ifdef DEBUG
						acutPrintf(L"\n删除并关闭掉");
#endif
						ent->erase();
						ent->close();
					}
                }
				else
				{
					acutPrintf(L"\n对象失败");
					rxErrorMsg(es);
				}
            }

			//把迭代器删了，防内存泄露
            delete iter;
        }

		//关闭实体
		if( pBlockTable )
			pBlockTable->close();

		if( pBlockTableRecord )
			pBlockTableRecord->close();

		UnLockCurDoc();

		return Acad::eOk;
	}
	catch(const Acad::ErrorStatus es)
	{
		acutPrintf(L"\n操作数据库发生异常！");
		rxErrorMsg(es);

		UnLockCurDoc();

		return Acad::eOutOfMemory;
	}
}

/**
 * 将对象从模型空间中的某一层上删除
 **/
Acad::ErrorStatus ArxWrapper::RemoveFromModelSpace(const AcDbHandle& handle,const wstring& layerName )
{
	AcDbObjectId entId;

	try
	{
		if( handle.isNull() )
			return Acad::eOk;

		LockCurDoc();

		//打开块表数据库
		AcDbBlockTable *pBlockTable;
		acdbHostApplicationServices()->workingDatabase()
			->getBlockTable(pBlockTable, AcDb::kForWrite);

		//得到模型空间
		AcDbBlockTableRecord *pBlockTableRecord;
		pBlockTable->getAt(ACDB_MODEL_SPACE, pBlockTableRecord,
		AcDb::kForWrite);

		//遍历数据库
		AcDbBlockTableRecordIterator* iter;
		Acad::ErrorStatus es = pBlockTableRecord->newIterator(iter);
        if (es != Acad::eOk) 
		{
			acutPrintf(L"\n打开数据库失败了");
            rxErrorMsg(es);
            pBlockTableRecord->close();
        }
        else 
		{
            AcDbEntity* ent;
            for (; !iter->done(); iter->step()) 
			{
                if (iter->getEntity(ent, AcDb::kForWrite) == Acad::eOk) 
				{
					AcDbHandle entHandle;
					ent->getAcDbHandle(entHandle);

					if( !entHandle.isNull() )
					{
						if( entHandle == handle )
						{
#ifdef DEBUG
							acutPrintf(L"\n对象找到了，删除并关闭掉");
#endif						
							ent->erase();
							ent->close();
							break;
						}
					}
					else
					{
						acutPrintf(L"\n根据Handle删除对象时出错！");
					}
                }
            }

			//把迭代器删了，防内存泄露
            delete iter;
        }

		//关闭实体
		pBlockTable->close();
		pBlockTableRecord->close();

		UnLockCurDoc();

		return Acad::eOk;
	}
	catch(const Acad::ErrorStatus es)
	{
		acutPrintf(L"\n操作数据库发生异常！");
		rxErrorMsg(es);

		UnLockCurDoc();

		return Acad::eOutOfMemory;
	}
}

/**
 * 将对象从模型空间中的某一层上删除
 **/
Acad::ErrorStatus ArxWrapper::RemoveFromModelSpace(AcDbEntity* pEnt,const wstring& layerName )
{
	AcDbObjectId entId;

	try
	{
		if( !pEnt )
			return Acad::eOk;

		LockCurDoc();

		//打开块表数据库
		AcDbBlockTable *pBlockTable;
		acdbHostApplicationServices()->workingDatabase()
			->getBlockTable(pBlockTable, AcDb::kForWrite);

		//得到模型空间
		AcDbBlockTableRecord *pBlockTableRecord;
		pBlockTable->getAt(ACDB_MODEL_SPACE, pBlockTableRecord,
		AcDb::kForWrite);

		//遍历数据库
		AcDbBlockTableRecordIterator* iter;
		Acad::ErrorStatus es = pBlockTableRecord->newIterator(iter);
        if (es != Acad::eOk) 
		{
			acutPrintf(L"\n打开数据库失败了");
            rxErrorMsg(es);
            pBlockTableRecord->close();
        }
        else 
		{
            AcDbEntity* ent;
            for (; !iter->done(); iter->step()) 
			{
                if (iter->getEntity(ent, AcDb::kForWrite) == Acad::eOk) 
				{
					if( ent == pEnt )
					{
#ifdef DEBUG
						acutPrintf(L"\n对象找到了，删除并关闭掉");
#endif						
						ent->erase();
						ent->close();
						break;
					}
                }
            }

			//把迭代器删了，防内存泄露
            delete iter;
        }

		//关闭实体
		pBlockTable->close();
		pBlockTableRecord->close();

		UnLockCurDoc();

		return Acad::eOk;
	}
	catch(const Acad::ErrorStatus es)
	{
		acutPrintf(L"\n操作数据库发生异常！");
		rxErrorMsg(es);

		UnLockCurDoc();

		return Acad::eOutOfMemory;
	}
}

/**
 * 创建特定名称的层
 **/
bool ArxWrapper::createNewLayer(const wstring& rLayerName)
{
	bool result = false;
	int option = 0;

	if( option == 0 )
	{
		Acad::ErrorStatus es;
		
		AcDbLayerTable *pLayerTable;
		AcDbLayerTableRecord *pLayerTableRecord = NULL;

		LockCurDoc();

		try
		{
			//打开层表数据库
			es = acdbHostApplicationServices()->workingDatabase()
				->getSymbolTable(pLayerTable, AcDb::kForWrite);

			if( es != Acad::eOk )
				throw ErrorException(L"打开层表记录失败");

			//层表不存在，则创建
			if( pLayerTable->has(rLayerName.c_str()) )
			{
				acutPrintf(L"\n图层【%s】已存在,无需创建",rLayerName.c_str());
				pLayerTable->close();

				return true;
			}

			//构建新的层表记录
			acutPrintf(L"\n创建图层【%s】",rLayerName.c_str());
			pLayerTableRecord = new AcDbLayerTableRecord;
			es = pLayerTableRecord->setName(rLayerName.c_str());
			if( es != Acad::eOk )
				throw ErrorException(L"设置新层表记录的名字失败");

			// Defaults are used for other properties of 
			// the layer if they are not otherwise specified.
			AcDbObjectId layerTblRcdId;
			es = pLayerTable->add(layerTblRcdId,pLayerTableRecord);
			if( es != Acad::eOk )
				throw ErrorException(L"加入新的层表记录失败");

			/*
			pLayerTableRecord->downgradeOpen();
			es = acdbHostApplicationServices()->workingDatabase()->setClayer(layerTblRcdId);
			if( es != Acad::eOk )
				throw ErrorException(L"设置当前图层为CLAYER时出错");
			*/

			result = true; 
		}
		catch( const ErrorException& e)
		{
			acutPrintf(L"\n创建新的层表记录失败，【%s】！",e.errMsg.c_str());
			rxErrorMsg(es);
		}

		if( pLayerTableRecord != NULL )
			pLayerTableRecord->close();

		if( pLayerTable != NULL )
			pLayerTable->close();

		UnLockCurDoc();
	}
	else
	{
		// 提示用户输入新建图层的名称
		ACHAR layerName[100];
		wmemset(layerName,0,100);
		wsprintf(layerName,L"%s",rLayerName.c_str());

		// 获得当前图形的层表
		AcDbLayerTable *pLayerTbl;
		acdbHostApplicationServices()->workingDatabase()
			->getLayerTable(pLayerTbl, AcDb::kForWrite);

		// 是否已经包含指定的层表记录
		if (pLayerTbl->has(layerName))
		{
			pLayerTbl->close();
			return true;
		}

		// 创建新的层表记录
		AcDbLayerTableRecord *pLayerTblRcd;
		pLayerTblRcd = new AcDbLayerTableRecord();
		pLayerTblRcd->setName(layerName);

		// 将新建的层表记录添加到层表中
		AcDbObjectId layerTblRcdId;
		pLayerTbl->add(layerTblRcdId, pLayerTblRcd);
		acdbHostApplicationServices()->workingDatabase()->setClayer(layerTblRcdId);

		pLayerTblRcd->close();
		pLayerTbl->close();
	}

	return result;
}

/**
 * 删除特定图层
 **/
bool ArxWrapper::DeleteLayer(const wstring& layerName, bool deleteChildren)
{
	bool result = false;

	LockCurDoc();

	//打开层表数据库
	AcDbLayerTable *pLayerTable;
	Acad::ErrorStatus es = acdbHostApplicationServices()->workingDatabase()
        ->getSymbolTable(pLayerTable, AcDb::kForWrite);

	if( pLayerTable )
	{
		AcDbLayerTableRecord *pLayerTableRecord = NULL;

		//层表不存在，则创建
		if(pLayerTable->has(layerName.c_str()))
		{
			acutPrintf(L"\n删除图层【%s】",layerName.c_str());

			//构建新的层表记录
			AcDbLayerTableRecord *pLayerRecord;

			// Defaults are used for other properties of 
			// the layer if they are not otherwise specified.
			Acad::ErrorStatus es = pLayerTable->getAt(layerName.c_str(),pLayerRecord,AcDb::kForWrite);
			pLayerTable->close();

			if( pLayerRecord )
			{
				pLayerRecord->erase();
				pLayerRecord->close();

				result = true;
			}
			else
			{
				acutPrintf(L"\n打开层表记录失败");
				rxErrorMsg(es);
			}
		}
		else
		{
			acutPrintf(L"\n图层【%s】不存在，无法删除",layerName.c_str());
		}
	}
	else
	{
		acutPrintf(L"\n打开层表记录失败！");
		rxErrorMsg(es);
	}

	UnLockCurDoc();

	return result;
}

/**
 * 隐藏其他图层
 **/
bool ArxWrapper::ShowLayer(const wstring& layerName)
{
	bool result = true;

	LockCurDoc();

	//打开层表数据库
	AcDbLayerTable *pLayerTable;
    acdbHostApplicationServices()->workingDatabase()
        ->getSymbolTable(pLayerTable, AcDb::kForWrite);

	if( pLayerTable )
	{
		AcDbLayerTableIterator *pLayerIter = NULL;
		Acad::ErrorStatus es = pLayerTable->newIterator(pLayerIter);

		if( es == Acad::eOk )
		{
			for( ;!pLayerIter->done();pLayerIter->step() )
			{
				AcDbLayerTableRecord* pLayerTableRecord = NULL;

				pLayerIter->getRecord(pLayerTableRecord,AcDb::kForWrite);

				if( pLayerTableRecord )
				{
					const TCHAR *pLayerName; 
					pLayerTableRecord->getName(pLayerName);
#ifdef DEBUG
					acutPrintf(L"\n找到层【%s】",pLayerName);
#endif
					if( wstring(pLayerName) != wstring(L"0") )
					{
						if( layerName.length() == 0
							|| wstring(pLayerName) == layerName 
						)
						{
							pLayerTableRecord->setIsOff(false);
#ifdef DEBUG
							acutPrintf(L"，显示该层",pLayerName);
#endif					
						}
						else
						{
							pLayerTableRecord->setIsOff(true);
#ifdef DEBUG
							acutPrintf(L"，隐藏该层",pLayerName);
#endif					
						}
					}
					else
					{
						acutPrintf(L"\n0号图层不处理");
					}

					pLayerTableRecord->close();
				}
			}

			delete pLayerIter;
		}
		else
		{
			acutPrintf(L"\n打开图层表失败");
			rxErrorMsg(es);

			result = false;
		}

		pLayerTable->close();
	}

	UnLockCurDoc();

	return result;
}

/**
 * 根据起始点队列（向量列表），并放置在特定的层上
 **/
AcDbEntity* ArxWrapper::MoveToBottom(AcDbEntity* pEntry)
{
	if( !pEntry )
		return NULL;

	AcGeVector3d vec(-8,10,0);

	AcGeMatrix3d moveMatrix;
	moveMatrix.setToTranslation(vec);

	pEntry->transformBy(moveMatrix);

	return pEntry;
}

AcDbEntity* ArxWrapper::GetDbObject( const AcDbObjectId& objId, bool openWrite )
{
	if( !objId.isValid() )
	{
		acutPrintf(L"\n打开数据库对象时，对象ID非法!");
		return NULL;
	}

	AcDbEntity *pObject;
	AcDb::OpenMode openMode = AcDb::kForRead;
	if( openWrite )
		openMode = AcDb::kForWrite;

	Acad::ErrorStatus es = acdbOpenAcDbEntity(pObject, objId, openMode );
	if( es != Acad::eOk )
	{
		acutPrintf(L"\n打开数据库对象是出错");
		rxErrorMsg(es);
		return NULL;
	}

	return pObject;
}

/**
 * 切换当前视图到与WCS的viewDirection相一致的视图
 **/
void ArxWrapper::ChangeView(int viewDirection)
{
#ifdef DEBUG
	acutPrintf(L"\n设置当前视图为前视,中心点在50");
#endif

	wstring cmdViewDir;

	if( viewDirection == 1 )
		cmdViewDir = L"LEFT";
	else if ( viewDirection == 2 )
		cmdViewDir =  L"FRONT";
	else if  ( viewDirection == 3 )
		cmdViewDir =  L"TOP";
	else if( viewDirection == 4 )
		cmdViewDir = L"RIGHT";
	else if ( viewDirection == 5 )
		cmdViewDir =  L"BACK";
	else if  ( viewDirection == 6 )
		cmdViewDir =  L"BOTTOM";

#ifdef DEBUG
	acutPrintf(L"\n设置当前视图为前视为【%s】",cmdViewDir.c_str());
#endif

	acedCommand(RTSTR, _T("._-VIEW"), RTSTR, cmdViewDir.c_str(), 0);
}

void TestViewPort()
{
#define VIEW_NAME L"VIEW_TEST"

	AcDbViewportTable* pVPortTable;
	acdbHostApplicationServices()->workingDatabase()->getViewportTable(pVPortTable,AcDb::kForWrite);
	AcGePoint2d ptLL, ptUR;

	//Generate viewport table 1
	AcDbViewportTableRecord* pVPortTRcd1 = new AcDbViewportTableRecord;
	ptLL.set(0, 0);
	ptUR.set(0.75, 0.5);
	pVPortTRcd1->setLowerLeftCorner(ptLL);
	pVPortTRcd1->setUpperRightCorner(ptUR);
	pVPortTRcd1->setName(VIEW_NAME);

	//Generate viewport table 2
	AcDbViewportTableRecord* pVPortTRcd2 = new AcDbViewportTableRecord;
	ptLL.set(0.75, 0);
	ptUR.set(1, 0.5);
	pVPortTRcd2->setLowerLeftCorner(ptLL);
	pVPortTRcd2->setUpperRightCorner(ptUR);
	pVPortTRcd2->setName(VIEW_NAME);

	//Generate viewport table 3
	AcDbViewportTableRecord* pVPortTRcd3 = new AcDbViewportTableRecord;
	ptLL.set(0, 0.5);
	ptUR.set(0.5, 1);
	pVPortTRcd3->setLowerLeftCorner(ptLL);
	pVPortTRcd3->setUpperRightCorner(ptUR);
	pVPortTRcd3->setName(VIEW_NAME);

	//Generate viewport table 4
	AcDbViewportTableRecord* pVPortTRcd4 = new AcDbViewportTableRecord;
	ptLL.set(0.5, 0.5);
	ptUR.set(1, 1);
	pVPortTRcd4->setLowerLeftCorner(ptLL);
	pVPortTRcd4->setUpperRightCorner(ptUR);
	pVPortTRcd4->setName(VIEW_NAME);

	pVPortTable->add( pVPortTRcd1);
	pVPortTable->add( pVPortTRcd2);
	pVPortTable->add( pVPortTRcd3);
	pVPortTable->add( pVPortTRcd4);

	pVPortTable->close();
	pVPortTRcd1->close();
	pVPortTRcd2->close();
	pVPortTRcd3->close();
	pVPortTRcd4->close();

	struct resbuf rb;
	acedGetVar(L"TILEMODE", &rb);

	if (rb.resval.rint == 1) // 当前工作空间是模型空间
	{
		acedCommand(RTSTR, L".-VPORTS", RTSTR, L"R", RTSTR, VIEW_NAME, RTNONE);
	}
	else // 当前工作空间是图纸空间
	{
		acedCommand(RTSTR, L".-VPORTS", RTSTR, L"R", RTSTR, VIEW_NAME, RTSTR, L"", RTNONE);
	}
}

void ArxWrapper::TestFunction()
{
	//TestHatch();

	//TestText();

	//TestLeader();

	//TestMLeader();

	//TestLayer();

	//TestArxPath();

	TestViewPort();
}

void TestArxPath()
{
	void *appPaths = acrxLoadedApps();
	AcDbVoidPtrArray *appPathArray = (AcDbVoidPtrArray*)appPaths;

	for (int i=0; i <= appPathArray->length(); i++)
	{
		wstring s;
		wchar_t *path =(wchar_t*)appPathArray->at(i);

		acutPrintf(L"\n的ARX加载路径【%s】",path);
	}

	delete appPaths;
}

void TestLayer()
{
	// 提示用户输入新建图层的名称
	ACHAR layerName[100];
	if (acedGetString(Adesk::kFalse, L"\n输入新图层的名称：", layerName) != RTNORM)
	{
		return;
	}

	// 获得当前图形的层表
	AcDbLayerTable *pLayerTbl;
	acdbHostApplicationServices()->workingDatabase()
		->getLayerTable(pLayerTbl, AcDb::kForWrite);

	// 是否已经包含指定的层表记录
	if (pLayerTbl->has(layerName))
	{
		pLayerTbl->close();
		return;
	}

	// 创建新的层表记录
	AcDbLayerTableRecord *pLayerTblRcd;
	pLayerTblRcd = new AcDbLayerTableRecord();
	pLayerTblRcd->setName(layerName);

	// 将新建的层表记录添加到层表中
	AcDbObjectId layerTblRcdId;
	pLayerTbl->add(layerTblRcdId, pLayerTblRcd);
	acdbHostApplicationServices()->workingDatabase()->setClayer(layerTblRcdId);

	pLayerTblRcd->close();
	pLayerTbl->close();
}

void TestHatch()
{
	// 提示用户选择填充边界
	ads_name ss;
	int rt = acedSSGet(NULL, NULL, NULL, NULL, ss);

	AcDbObjectIdArray objIds;
	// 初始化填充边界的ID数组
	if (rt == RTNORM)
	{
		long length;
		acedSSLength(ss, &length);
		for (int i = 0; i < length; i++)
		{
			ads_name ent;
			acedSSName(ss, i, ent);
			AcDbObjectId objId;
			acdbGetObjectId(objId, ent);
			objIds.append(objId);
		}
	}

	acedSSFree(ss); // 释放选择集

	//ArxWrapper::CreateHatch(objIds, L"JIS_LC_20", true, L"0", AcGeVector3d(0,0,1), 0);
}

static AcDbObjectId CreateText(const AcGePoint3d& ptInsert,
		const wchar_t* text, AcDbObjectId style = AcDbObjectId::kNull,
			double height = 2.5, double rotation = 0);

// 实现部分
AcDbObjectId CreateText(const AcGePoint3d& ptInsert,
						const wchar_t* text, AcDbObjectId style,
						double height, double rotation)
{
	AcDbText *pText = new AcDbText(ptInsert, text, style, height, rotation);
	return ArxWrapper::PostToModelSpace(pText,L"0");
}

void TestText()
{
	// Hide the dialog and give control to the editor
    ads_point pt;

    // Get a point
    if (acedGetPoint(NULL, _T("\n选取输入点: "), pt) == RTNORM) {
		// 创建单行文字
		AcGePoint3d ptInsert(pt[X], pt[Y], pt[Z]);
		CreateText(ptInsert, L"测试文字");
	}
}

void TestMLeader()
{
	AcGePoint3dArray vertices;
	AcGePoint3d startPoint,endPoint;

	{
		ads_point pt;

		if (acedGetPoint(NULL, _T("\n选取MLeader开始点: "), pt) != RTNORM) 
			return;

		startPoint.x = pt[X];
		startPoint.y = pt[Y];
		startPoint.z = pt[Z];

		//if (acedGetPoint(NULL, _T("\n选取MLeader结束点: "), pt) != RTNORM) 
		//	return;

		endPoint.x = pt[X] - 9;
		endPoint.y = pt[Y] - 9;
		endPoint.z = pt[Z];
	}

    AcDbMLeader* leader = new AcDbMLeader;

	int index = 0;
	leader->addLeaderLine(startPoint,index);

	leader->addLastVertex(index,endPoint);
	leader->setLastVertex(index,endPoint);

	leader->setContentType(AcDbMLeaderStyle::kMTextContent);

	AcDbMText* mtext = new AcDbMText;
	mtext->setContents(L"动力线1#【半径10】");

	leader->setMText(mtext);
	ArxWrapper::PostToModelSpace(leader,L"0");
}

void TestLeader()
{
    AcGePoint3dArray vertices;
	AcGePoint3d startPoint;

	{
		ads_point pt;

		if (acedGetPoint(NULL, _T("\n选取输入点: "), pt) != RTNORM) 
			return;

		startPoint.x = pt[X];
		startPoint.y = pt[Y];
		startPoint.z = pt[Z];
	}
		
	AcGePoint3d endPoint(startPoint.x + 1,startPoint.y-2,0);
	AcGePoint3d lastPoint(startPoint.x + 5,startPoint.y-2,0);

	vertices.append(startPoint);
	vertices.append(endPoint);
	vertices.append(lastPoint);

    AcDbLeader* leader = new AcDbLeader;

    for (int i=0;i<3;i++)
        leader->appendVertex(vertices[i]);

    leader->setToSplineLeader();    // set to spline just for fun
    leader->setDatabaseDefaults();    // pick up default dimstyle, etc.

	AcDbMText* mtext = new AcDbMText;
    //mtext->setLocation(prInsertPt.value());
    //mtext->setWidth(fabs(prInsertPt.value().x - prCorner.value().x));

	mtext->setContents(L"1#动力线，半径10");
	mtext->setDirection(AcGeVector3d(1,0,0));
	mtext->setLocation(endPoint);
	mtext->setHeight(0.5);

	ArxWrapper::PostToModelSpace(mtext,L"0");

	//leader->setAnnotationObjId(mtext->objectId());
	ArxWrapper::PostToModelSpace(leader,L"0");
}
