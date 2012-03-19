/*
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "UBPersistenceManager.h"
#include "gui/UBMainWindow.h"

#include <QtXml>

#include "frameworks/UBPlatformUtils.h"
#include "frameworks/UBFileSystemUtils.h"

#include "core/UBApplication.h"
#include "core/UBSettings.h"
#include "core/UBSetting.h"

#include "document/UBDocumentProxy.h"

#include "adaptors/UBExportPDF.h"
#include "adaptors/UBSvgSubsetAdaptor.h"
#include "adaptors/UBThumbnailAdaptor.h"
#include "adaptors/UBMetadataDcSubsetAdaptor.h"

#include "board/UBBoardController.h"
#include "board/UBBoardPaletteManager.h"

#include "core/memcheck.h"

const QString UBPersistenceManager::imageDirectory = "images"; // added to UBPersistenceManager::mAllDirectories
const QString UBPersistenceManager::objectDirectory = "objects"; // added to UBPersistenceManager::mAllDirectories
const QString UBPersistenceManager::widgetDirectory = "widgets"; // added to UBPersistenceManager::mAllDirectories
const QString UBPersistenceManager::videoDirectory = "videos"; // added to UBPersistenceManager::mAllDirectories
const QString UBPersistenceManager::audioDirectory = "audios"; // added to
const QString UBPersistenceManager::teacherBarDirectory = "teacherBarObjects";


const QString UBPersistenceManager::gId = "id";
const QString UBPersistenceManager::gSeparator = "$$#@@!@#{}{{]][";

UBPersistenceManager * UBPersistenceManager::sSingleton = 0;

UBPersistenceManager::UBPersistenceManager(QObject *pParent)
    : QObject(pParent)
    , mHasPurgedDocuments(false)
{

    mDocumentSubDirectories << imageDirectory;
    mDocumentSubDirectories << objectDirectory;
    mDocumentSubDirectories << widgetDirectory;
    mDocumentSubDirectories << videoDirectory;
    mDocumentSubDirectories << audioDirectory;

    documentProxies = allDocumentProxies();
    emit proxyListChanged();
}

UBPersistenceManager* UBPersistenceManager::persistenceManager()
{
    if (!sSingleton)
    {
        sSingleton = new UBPersistenceManager(UBApplication::staticMemoryCleaner);
    }

    return sSingleton;
}

void UBPersistenceManager::destroy()
{
    if (sSingleton)
        delete sSingleton;
    sSingleton = NULL;
}

UBPersistenceManager::~UBPersistenceManager()
{
    foreach(QPointer<UBDocumentProxy> proxyGuard, documentProxies)
    {
        if (!proxyGuard.isNull())
            delete proxyGuard.data();
    }
}


QList<QPointer<UBDocumentProxy> > UBPersistenceManager::allDocumentProxies()
{
    mDocumentRepositoryPath = UBSettings::userDocumentDirectory();

    QDir rootDir(mDocumentRepositoryPath);
    rootDir.mkpath(rootDir.path());


    QFileSystemWatcher* watcher = new QFileSystemWatcher(this);
    watcher->addPath(mDocumentRepositoryPath);

    connect(watcher, SIGNAL(directoryChanged(const QString&)), this, SLOT(documentRepositoryChanged(const QString&)));

    QList<QPointer<UBDocumentProxy> > proxies;

    foreach(QString path, rootDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot,
            QDir::Time | QDir::Reversed))
    {
        QString fullPath = rootDir.path() + "/" + path;

        QDir dir(fullPath);

        if (dir.entryList(QDir::Files | QDir::NoDotAndDotDot).size() > 0)
        {
            UBDocumentProxy* proxy = new UBDocumentProxy(fullPath); // deleted in UBPersistenceManager::destructor

            QMap<QString, QVariant> metadatas = UBMetadataDcSubsetAdaptor::load(fullPath);

            foreach(QString key, metadatas.keys())
            {
                proxy->setMetaData(key, metadatas.value(key));
            }

            proxy->setPageCount(sceneCount(proxy));

            proxies << QPointer<UBDocumentProxy>(proxy);

        }
    }

    return proxies;
}


QStringList UBPersistenceManager::allShapes()
{
    QString shapeLibraryPath = UBSettings::settings()->applicationShapeLibraryDirectory();

    QDir dir(shapeLibraryPath);

    if (!dir.exists())
        dir.mkpath(shapeLibraryPath);

    QStringList files = dir.entryList(QDir::Files | QDir::NoDotAndDotDot, QDir::Name);
    QStringList paths;

    foreach(QString file, files)
    {
        paths.append(shapeLibraryPath + QString("/") + file);
    }

    return paths;
}

QStringList UBPersistenceManager::allGips()
{
    QString gipLibraryPath = UBSettings::settings()->applicationGipLibraryDirectory();

    QDir dir(gipLibraryPath);

    QStringList files = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name);
    QStringList paths;

    foreach(QString file, files)
    {
        QFileInfo fi(file);

        if (UBSettings::settings()->widgetFileExtensions.contains(fi.suffix()))
            paths.append(dir.path() + QString("/") + file);
    }

    return paths;
}

QStringList UBPersistenceManager::allImages(const QDir& dir)
{
    if (!dir.exists())
        dir.mkpath(dir.path());

    QStringList files = dir.entryList(QDir::Files | QDir::Hidden | QDir::NoDotAndDotDot, QDir::Name);
    QStringList paths;

    foreach(QString file, files)
    {
        paths.append(dir.path() + QString("/") + file);
    }

    return paths;
}


QStringList UBPersistenceManager::allVideos(const QDir& dir)
{
    if (!dir.exists())
        dir.mkpath(dir.path());

    QStringList files = dir.entryList(QDir::Files | QDir::NoDotAndDotDot, QDir::Name);
    QStringList paths;

    foreach(QString file, files)
    {
        paths.append(dir.path() + QString("/") + file);
    }

    return paths;
}


QStringList UBPersistenceManager::allWidgets(const QDir& dir)
{
    if (!dir.exists())
        dir.mkpath(dir.path());

    QStringList files = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name);
    QStringList paths;

    foreach(QString file, files)
    {
        QFileInfo fi(file);

        if (UBSettings::settings()->widgetFileExtensions.contains(fi.suffix()))
            paths.append(dir.path() + QString("/") + file);
    }

    return paths;
}


UBDocumentProxy* UBPersistenceManager::createDocument(const QString& pGroupName, const QString& pName, bool withEmptyPage)
{
    checkIfDocumentRepositoryExists();

    UBDocumentProxy *doc = new UBDocumentProxy(); // deleted in UBPersistenceManager::destructor

    if (pGroupName.length() > 0)
    {
        doc->setMetaData(UBSettings::documentGroupName, pGroupName);
    }

    if (pName.length() > 0)
    {
        doc->setMetaData(UBSettings::documentName, pName);
    }

    doc->setMetaData(UBSettings::documentVersion, UBSettings::currentFileVersion);
    doc->setMetaData(UBSettings::documentUpdatedAt, UBStringUtils::toUtcIsoDateTime(QDateTime::currentDateTime()));

    if (withEmptyPage) createDocumentSceneAt(doc, 0);

    documentProxies.insert(0, QPointer<UBDocumentProxy>(doc));

    emit proxyListChanged();

    emit documentCreated(doc);

    mDocumentCreatedDuringSession << doc;

    return doc;
}


UBDocumentProxy* UBPersistenceManager::createDocumentFromDir(const QString& pDocumentDirectory)
{
    checkIfDocumentRepositoryExists();

    UBDocumentProxy* doc = new UBDocumentProxy(pDocumentDirectory); // deleted in UBPersistenceManager::destructor

    QMap<QString, QVariant> metadatas = UBMetadataDcSubsetAdaptor::load(pDocumentDirectory);

    foreach(QString key, metadatas.keys())
    {
        doc->setMetaData(key, metadatas.value(key));
    }

    doc->setUuid(QUuid::createUuid());
    doc->setPageCount(sceneCount(doc));

    UBMetadataDcSubsetAdaptor::persist(doc);

    for(int i = 0; i < doc->pageCount(); i++)
    {
        UBSvgSubsetAdaptor::setSceneUuid(doc, i, QUuid::createUuid());
    }

    documentProxies << QPointer<UBDocumentProxy>(doc);

    emit proxyListChanged();

    emit documentCreated(doc);

    return doc;
}


void UBPersistenceManager::deleteDocument(UBDocumentProxy* pDocumentProxy)
{
    checkIfDocumentRepositoryExists();

    emit documentWillBeDeleted(pDocumentProxy);

    UBFileSystemUtils::deleteDir(pDocumentProxy->persistencePath());

    documentProxies.removeAll(QPointer<UBDocumentProxy>(pDocumentProxy));
    mDocumentCreatedDuringSession.removeAll(pDocumentProxy);

    mSceneCache.removeAllScenes(pDocumentProxy);

    pDocumentProxy->deleteLater();

    emit proxyListChanged();

}


UBDocumentProxy* UBPersistenceManager::duplicateDocument(UBDocumentProxy* pDocumentProxy)
{
    checkIfDocumentRepositoryExists();

    UBDocumentProxy *copy = new UBDocumentProxy(); // deleted in UBPersistenceManager::destructor

    generatePathIfNeeded(copy);

    UBFileSystemUtils::copyDir(pDocumentProxy->persistencePath(), copy->persistencePath());

    // regenerate scenes UUIDs
    for(int i = 0; i < pDocumentProxy->pageCount(); i++)
    {
        UBSvgSubsetAdaptor::setSceneUuid(pDocumentProxy, i, QUuid::createUuid());
    }

    foreach(QString key, pDocumentProxy->metaDatas().keys())
    {
        copy->setMetaData(key, pDocumentProxy->metaDatas().value(key));
    }

    copy->setMetaData(UBSettings::documentName,
            pDocumentProxy->metaData(UBSettings::documentName).toString() + " " + tr("(copy)"));

    copy->setUuid(QUuid::createUuid());

    persistDocumentMetadata(copy);

    copy->setPageCount(sceneCount(copy));

    documentProxies << QPointer<UBDocumentProxy>(copy);

    emit proxyListChanged();

    emit documentCreated(copy);

    return copy;

}


void UBPersistenceManager::deleteDocumentScenes(UBDocumentProxy* proxy, const QList<int>& indexes)
{
	checkIfDocumentRepositoryExists();

    int pageCount = UBPersistenceManager::persistenceManager()->sceneCount(proxy);

    QList<int> compactedIndexes;

    foreach(int index, indexes)
    {
        if (!compactedIndexes.contains(index))
            compactedIndexes.append(index);
    }

    if (compactedIndexes.size() == pageCount)
    {
        deleteDocument(proxy);
        return;
    }

    if (compactedIndexes.size() == 0)
        return;

    foreach(int index, compactedIndexes)
    {
        emit documentSceneWillBeDeleted(proxy, index);
    }

    QString sourceGroupName = proxy->metaData(UBSettings::documentGroupName).toString();
    QString sourceName = proxy->metaData(UBSettings::documentName).toString();
    UBDocumentProxy *trashDocProxy = createDocument(UBSettings::trashedDocumentGroupNamePrefix + sourceGroupName, sourceName, false);

    foreach(int index, compactedIndexes)
    {
        UBGraphicsScene *scene = loadDocumentScene(proxy, index);
        if (scene)
        {
            //scene is about to move into new document
            foreach (QUrl relativeFile, scene->relativeDependencies())
            {
                QString source = scene->document()->persistencePath() + "/" + relativeFile.toString();
                QString target = trashDocProxy->persistencePath() + "/" + relativeFile.toString();

                QFileInfo fi(target);
                QDir d = fi.dir();

                d.mkpath(d.absolutePath());
                QFile::copy(source, target);
            }

            insertDocumentSceneAt(trashDocProxy, scene, trashDocProxy->pageCount());
        }
    }

    for (int i = 1; i < pageCount; i++)
    {
        renamePage(trashDocProxy, i , i - 1);
    }

    foreach(int index, compactedIndexes)
    {
        QString svgFileName = proxy->persistencePath() +
            UBFileSystemUtils::digitFileFormat("/page%1.svg", index + 1);

        QFile::remove(svgFileName);

        QString thumbFileName = proxy->persistencePath() +
            UBFileSystemUtils::digitFileFormat("/page%1.thumbnail.jpg", index + 1);

        QFile::remove(thumbFileName);

		mSceneCache.removeScene(proxy, index);

        proxy->decPageCount();

    }

    qSort(compactedIndexes);

    int offset = 1;

    for (int i = compactedIndexes.at(0) + 1; i < pageCount; i++)
    {
        if(compactedIndexes.contains(i))
        {
            offset++;
        }
        else
        {
            renamePage(proxy, i , i - offset);

            mSceneCache.moveScene(proxy, i, i - offset);

        }
    }

    foreach(int index, compactedIndexes)
    {
         emit documentSceneDeleted(proxy, index);
    }
}


void UBPersistenceManager::duplicateDocumentScene(UBDocumentProxy* proxy, int index)
{
    checkIfDocumentRepositoryExists();

    int pageCount = UBPersistenceManager::persistenceManager()->sceneCount(proxy);

    for (int i = pageCount; i > index + 1; i--)
    {
        renamePage(proxy, i - 1 , i);

        mSceneCache.moveScene(proxy, i - 1, i);

    }

    copyPage(proxy, index , index + 1);

    proxy->incPageCount();

	//due to architectural peculiarity we need to save teacher bar info, otherwise we'll see not exactly what we expect
	sTeacherBarInfos properInfo = getTeacherBarInfos(proxy, index/* + 1*/);
	//after the call below
	emit documentSceneCreated(proxy, index + 1);
	//restoring info
	persistTeacherBar(proxy, index + 1, properInfo);
}


UBGraphicsScene* UBPersistenceManager::createDocumentSceneAt(UBDocumentProxy* proxy, int index)
{
    int count = sceneCount(proxy);

    for(int i = count - 1; i >= index; i--)
    {
        renamePage(proxy, i , i + 1);
    }

    mSceneCache.shiftUpScenes(proxy, index, count -1);

    UBGraphicsScene *newScene = mSceneCache.createScene(proxy, index);

    newScene->setBackground(UBSettings::settings()->isDarkBackground(),
            UBSettings::settings()->UBSettings::isCrossedBackground());

    persistDocumentScene(proxy, newScene, index);

    proxy->incPageCount();

    emit documentSceneCreated(proxy, index);

    return newScene;
}


void UBPersistenceManager::insertDocumentSceneAt(UBDocumentProxy* proxy, UBGraphicsScene* scene, int index)
{
    scene->setDocument(proxy);

    int count = sceneCount(proxy);

    for(int i = count - 1; i >= index; i--)
    {
        renamePage(proxy, i , i + 1);
    }

    mSceneCache.shiftUpScenes(proxy, index, count -1);

    mSceneCache.insert(proxy, index, scene);

    persistDocumentScene(proxy, scene, index);

    proxy->incPageCount();

    emit documentSceneCreated(proxy, index);

}


void UBPersistenceManager::moveSceneToIndex(UBDocumentProxy* proxy, int source, int target)
{
    checkIfDocumentRepositoryExists();

    if (source == target)
        return;

    QFile svgTmp(proxy->persistencePath() + UBFileSystemUtils::digitFileFormat("/page%1.svg", source + 1));
    svgTmp.rename(proxy->persistencePath() + UBFileSystemUtils::digitFileFormat("/page%1.tmp", target + 1));

    QFile thumbTmp(proxy->persistencePath() + UBFileSystemUtils::digitFileFormat("/page%1.thumbnail.jpg", source + 1));
    thumbTmp.rename(proxy->persistencePath() + UBFileSystemUtils::digitFileFormat("/page%1.thumbnail.tmp", target + 1));

    if (source < target)
    {
        for (int i = source + 1; i <= target; i++)
        {
            renamePage(proxy, i , i - 1);
        }
    }
    else
    {
        for (int i = source - 1; i >= target; i--)
        {
            renamePage(proxy, i , i + 1);
        }
    }

    QFile svg(proxy->persistencePath() + UBFileSystemUtils::digitFileFormat("/page%1.tmp", target + 1));
    svg.rename(proxy->persistencePath() + UBFileSystemUtils::digitFileFormat("/page%1.svg", target + 1));

    QFile thumb(proxy->persistencePath() + UBFileSystemUtils::digitFileFormat("/page%1.thumbnail.tmp", target + 1));
    thumb.rename(proxy->persistencePath() + UBFileSystemUtils::digitFileFormat("/page%1.thumbnail.jpg", target + 1));

    mSceneCache.moveScene(proxy, source, target);

    emit documentSceneMoved(proxy, target);
}


UBGraphicsScene* UBPersistenceManager::loadDocumentScene(UBDocumentProxy* proxy, int sceneIndex)
{
    if (mSceneCache.contains(proxy, sceneIndex))
    {
        //qDebug() << "scene" << sceneIndex << "retrieved from cache ...";
        return mSceneCache.value(proxy, sceneIndex);
    }
    else
    {
        UBGraphicsScene* scene = UBSvgSubsetAdaptor::loadScene(proxy, sceneIndex);

        if (scene)
            mSceneCache.insert(proxy, sceneIndex, scene);

        return scene;
    }
}


void UBPersistenceManager::persistDocumentScene(UBDocumentProxy* pDocumentProxy, UBGraphicsScene* pScene, const int pSceneIndex)
{
    checkIfDocumentRepositoryExists();

    pScene->deselectAllItems();

    generatePathIfNeeded(pDocumentProxy);

    QDir dir(pDocumentProxy->persistencePath());
    dir.mkpath(pDocumentProxy->persistencePath());

    if (pDocumentProxy->isModified())
        UBMetadataDcSubsetAdaptor::persist(pDocumentProxy);

    if (pScene->isModified())
    {
        UBThumbnailAdaptor::persistScene(pDocumentProxy->persistencePath(), pScene, pSceneIndex);

        UBSvgSubsetAdaptor::persistScene(pDocumentProxy, pScene, pSceneIndex);

        pScene->setModified(false);
    }

    mSceneCache.insert(pDocumentProxy, pSceneIndex, pScene);

    emit documentCommitted(pDocumentProxy);

}


UBDocumentProxy* UBPersistenceManager::persistDocumentMetadata(UBDocumentProxy* pDocumentProxy)
{
    UBMetadataDcSubsetAdaptor::persist(pDocumentProxy);

    emit documentMetadataChanged(pDocumentProxy);

    return pDocumentProxy;
}


void UBPersistenceManager::renamePage(UBDocumentProxy* pDocumentProxy, const int sourceIndex, const int targetIndex)
{
    QFile svg(pDocumentProxy->persistencePath() + UBFileSystemUtils::digitFileFormat("/page%1.svg", sourceIndex + 1));
    svg.rename(pDocumentProxy->persistencePath() + UBFileSystemUtils::digitFileFormat("/page%1.svg", targetIndex + 1));

    QFile thumb(pDocumentProxy->persistencePath() + UBFileSystemUtils::digitFileFormat("/page%1.thumbnail.jpg", sourceIndex + 1));
    thumb.rename(pDocumentProxy->persistencePath() + UBFileSystemUtils::digitFileFormat("/page%1.thumbnail.jpg", targetIndex + 1));
}


void UBPersistenceManager::copyPage(UBDocumentProxy* pDocumentProxy, const int sourceIndex, const int targetIndex)
{
    QFile svg(pDocumentProxy->persistencePath() + UBFileSystemUtils::digitFileFormat("/page%1.svg", sourceIndex + 1));
    svg.copy(pDocumentProxy->persistencePath() + UBFileSystemUtils::digitFileFormat("/page%1.svg", targetIndex + 1));

    UBSvgSubsetAdaptor::setSceneUuid(pDocumentProxy, targetIndex, QUuid::createUuid());

    QFile thumb(pDocumentProxy->persistencePath() + UBFileSystemUtils::digitFileFormat("/page%1.thumbnail.jpg", sourceIndex + 1));
    thumb.copy(pDocumentProxy->persistencePath() + UBFileSystemUtils::digitFileFormat("/page%1.thumbnail.jpg", targetIndex + 1));
}


int UBPersistenceManager::sceneCount(const UBDocumentProxy* proxy)
{
    return sceneCountInDir(proxy->persistencePath());
}

int UBPersistenceManager::sceneCountInDir(const QString& pPath)
{
    int pageIndex = 0;
    bool moreToProcess = true;

    while (moreToProcess)
    {
        QString fileName = pPath + UBFileSystemUtils::digitFileFormat("/page%1.svg", pageIndex + 1);

        QFile file(fileName);

        if (file.exists())
        {
            pageIndex++;
        }
        else
        {
            moreToProcess = false;
        }
    }

    return pageIndex;
}


QString UBPersistenceManager::generateUniqueDocumentPath()
{
    QString ubPath = UBSettings::userDocumentDirectory();

    QDateTime now = QDateTime::currentDateTime();
    QString dirName = now.toString("yyyy-MM-dd hh-mm-ss.zzz");

    return ubPath + QString("/Sankore Document %1").arg(dirName);
}


void UBPersistenceManager::generatePathIfNeeded(UBDocumentProxy* pDocumentProxy)
{
    if (pDocumentProxy->persistencePath().length() == 0)
    {
        pDocumentProxy->setPersistencePath(generateUniqueDocumentPath());
    }
}


void UBPersistenceManager::addDirectoryContentToDocument(const QString& documentRootFolder, UBDocumentProxy* pDocument)
{
    int sourcePageCount = sceneCountInDir(documentRootFolder);

    int targetPageCount = pDocument->pageCount();

    for(int sourceIndex = 0 ; sourceIndex < sourcePageCount; sourceIndex++)
    {
        int targetIndex = targetPageCount + sourceIndex;

        QFile svg(documentRootFolder + UBFileSystemUtils::digitFileFormat("/page%1.svg", sourceIndex + 1));
        svg.copy(pDocument->persistencePath() + UBFileSystemUtils::digitFileFormat("/page%1.svg", targetIndex + 1));

        UBSvgSubsetAdaptor::setSceneUuid(pDocument, targetIndex, QUuid::createUuid());

        QFile thumb(documentRootFolder + UBFileSystemUtils::digitFileFormat("/page%1.thumbnail.jpg", sourceIndex + 1));
        thumb.copy(pDocument->persistencePath() + UBFileSystemUtils::digitFileFormat("/page%1.thumbnail.jpg", targetIndex + 1));
    }

    foreach(QString dir, mDocumentSubDirectories)
    {
        qDebug() << "copying " << documentRootFolder << "/" << dir << " to " << pDocument->persistencePath() << "/" + dir;

        UBFileSystemUtils::copyDir(documentRootFolder + "/" + dir, pDocument->persistencePath() + "/" + dir);
    }

    pDocument->setPageCount(sceneCount(pDocument));

}


void UBPersistenceManager::upgradeDocumentIfNeeded(UBDocumentProxy* pDocumentProxy)
{
    int pageCount = pDocumentProxy->pageCount();

    for(int index = 0 ; index < pageCount; index++)
    {
        UBSvgSubsetAdaptor::upgradeScene(pDocumentProxy, index);
    }

    pDocumentProxy->setMetaData(UBSettings::documentVersion, UBSettings::currentFileVersion);

    UBMetadataDcSubsetAdaptor::persist(pDocumentProxy);
}


void UBPersistenceManager::upgradeAllDocumentsIfNeeded()
{
    foreach(QPointer<UBDocumentProxy> proxy, documentProxies)
    {
        upgradeDocumentIfNeeded(proxy);
    }
}



UBDocumentProxy* UBPersistenceManager::documentByUuid(const QUuid& pUuid)
{
    for(int i = 0 ; i < documentProxies.length(); i++)
    {
        UBDocumentProxy* proxy = documentProxies.at(i);

        if (proxy && proxy->uuid() == pUuid)
        {
            return proxy;
        }
    }

    return 0;

}


bool UBPersistenceManager::isEmpty(UBDocumentProxy* pDocumentProxy)
{
    if(!pDocumentProxy)
        return true;

    if (pDocumentProxy->pageCount() > 1)
        return false;

	bool isTeacherBarEmpty = UBApplication::boardController->paletteManager()->teacherBarWidget()->isEmpty();
	if(!isTeacherBarEmpty)
		return false;

    UBGraphicsScene *theSoleScene = UBSvgSubsetAdaptor::loadScene(pDocumentProxy, 0);

    bool empty = false;

    if (theSoleScene)
    {
        empty = theSoleScene->isEmpty();
        delete theSoleScene;
    }
    else
    {
        empty = true;
    }

    return empty;
}


void UBPersistenceManager::purgeEmptyDocuments()
{
    if(!mHasPurgedDocuments) // hack to workaround the fact that app closing is called twice :-(
    {
        QList<UBDocumentProxy*> toBeDeleted;

        foreach(UBDocumentProxy* docProxy, mDocumentCreatedDuringSession)
        {
            if (isEmpty(docProxy))
            {
                toBeDeleted << docProxy;
            }
        }

        foreach(UBDocumentProxy* docProxy, toBeDeleted)
        {
            deleteDocument(docProxy);
        }

        mHasPurgedDocuments = true;
    }
}

QString UBPersistenceManager::addObjectToTeacherBarDirectory(UBDocumentProxy* pDocumentProxy, QString pPath)
{
    QFileInfo fi(pPath);
    QString uuid = QUuid::createUuid();

    if (!fi.exists() || !pDocumentProxy)
        return "";

    QString fileName = UBPersistenceManager::teacherBarDirectory + "/" + uuid + "." + fi.suffix();

    QString destPath = pDocumentProxy->persistencePath() + "/" + fileName;

    if (!QFile::exists(destPath))
    {
        QDir dir;
        dir.mkdir(pDocumentProxy->persistencePath() + "/" + UBPersistenceManager::teacherBarDirectory);

        QFile source(pPath);

        source.copy(destPath);
    }

    //return fileName;
    return destPath;
}


QString UBPersistenceManager::addVideoFileToDocument(UBDocumentProxy* pDocumentProxy, QString path, QUuid objectUuid)
{
    QFileInfo fi(path);

    if (!fi.exists() || !pDocumentProxy || objectUuid.isNull())
        return "";

    QString fileName = UBPersistenceManager::videoDirectory + "/" + objectUuid.toString() + "." + fi.suffix();

    QString destPath = pDocumentProxy->persistencePath() + "/" + fileName;

    if (!QFile::exists(destPath))
    {
        QDir dir;
        dir.mkdir(pDocumentProxy->persistencePath() + "/" + UBPersistenceManager::videoDirectory);

        QFile source(path);

        source.copy(destPath);

    }

    return fileName;

}


QString UBPersistenceManager::addVideoFileToDocument(UBDocumentProxy* pDocumentProxy, QUrl sourceUrl, QByteArray pPayload, QUuid objectUuid)
{
    if (!pDocumentProxy || objectUuid.isNull())
        return "";

    QString urlPath = sourceUrl.path();
    int lastDot = urlPath.lastIndexOf(".");
    QString suffix = urlPath.right(urlPath.length() - lastDot - 1);

    QString fileName = UBPersistenceManager::videoDirectory + "/" + objectUuid.toString() + "." + suffix;
    QString destPath = pDocumentProxy->persistencePath() + "/" + fileName;

    if (!QFile::exists(destPath))
    {
        QDir dir;
        dir.mkdir(pDocumentProxy->persistencePath() + "/" + UBPersistenceManager::videoDirectory);

        QFile newFile(destPath);

        if (newFile.open(QIODevice::WriteOnly))
        {
            newFile.write(pPayload);
            newFile.flush();
            newFile.close();
        }
    }

    return fileName;

}



QString UBPersistenceManager::addAudioFileToDocument(UBDocumentProxy* pDocumentProxy, QString path, QUuid objectUuid)
{
    QFileInfo fi(path);

    if (!fi.exists() || !pDocumentProxy || objectUuid.isNull())
        return "";

    QString fileName = UBPersistenceManager::audioDirectory + "/" + objectUuid.toString() + "." + fi.suffix();

    QString destPath = pDocumentProxy->persistencePath() + "/" + fileName;

    if (!QFile::exists(destPath))
    {
        QDir dir;
        dir.mkdir(pDocumentProxy->persistencePath() + "/" + UBPersistenceManager::audioDirectory);

        QFile source(path);

        source.copy(destPath);

    }

    return fileName;

}


QString UBPersistenceManager::addAudioFileToDocument(UBDocumentProxy* pDocumentProxy, QUrl sourceUrl, QByteArray pPayload, QUuid objectUuid)
{
    if (!pDocumentProxy || objectUuid.isNull())
        return "";

    QString urlPath = sourceUrl.path();
    int lastDot = urlPath.lastIndexOf(".");
    QString suffix = urlPath.right(urlPath.length() - lastDot - 1);

    QString fileName = UBPersistenceManager::audioDirectory + "/" + objectUuid.toString() + "." + suffix;
    QString destPath = pDocumentProxy->persistencePath() + "/" + fileName;

    if (!QFile::exists(destPath))
    {
        QDir dir;
        dir.mkdir(pDocumentProxy->persistencePath() + "/" + UBPersistenceManager::audioDirectory);

        QFile newFile(destPath);

        if (newFile.open(QIODevice::WriteOnly))
        {
            newFile.write(pPayload);
            newFile.flush();
            newFile.close();
        }
    }

    return fileName;

}


QString UBPersistenceManager::addPdfFileToDocument(UBDocumentProxy* pDocumentProxy, QString path, QUuid objectUuid)
{
    QFileInfo fi(path);

    if (!fi.exists() || !pDocumentProxy || objectUuid.isNull())
        return "";

    QString fileName = UBPersistenceManager::objectDirectory + "/" + objectUuid.toString() + "." + fi.suffix();
    QString destPath = pDocumentProxy->persistencePath() + "/" + fileName;

    if (!QFile::exists(destPath))
    {
        QDir dir;
        dir.mkpath(pDocumentProxy->persistencePath() + "/" + UBPersistenceManager::objectDirectory);

        QFile source(path);
        source.copy(destPath);
    }

    return fileName;
}
QString UBPersistenceManager::addGraphicsWidgteToDocument(UBDocumentProxy *pDocumentProxy, QString path, QUuid objectUuid)
{
    QFileInfo fi(path);

    if (!fi.exists() || !pDocumentProxy || objectUuid.isNull())
        return "";

    QString widgetRootDir = path;
    QString extension = QFileInfo(widgetRootDir).suffix();

    QString widgetTargetDir = pDocumentProxy->persistencePath() + "/" + widgetDirectory +  "/" + objectUuid.toString() + "." + extension;

    if (!QFile::exists(widgetTargetDir)) {
        QDir dir;
        dir.mkpath(widgetTargetDir);
        UBFileSystemUtils::copyDir(widgetRootDir, widgetTargetDir);
    }

    if (!QFile::exists(widgetTargetDir))
        widgetTargetDir = QString();

    return widgetTargetDir;
}


void UBPersistenceManager::documentRepositoryChanged(const QString& path)
{
    Q_UNUSED(path);
    checkIfDocumentRepositoryExists();
}


void UBPersistenceManager::checkIfDocumentRepositoryExists()
{
    QDir rp(mDocumentRepositoryPath);

    if (!rp.exists())
    {
        // we have lost the document repository ..

        QString humanPath = QDir::cleanPath(mDocumentRepositoryPath);
        humanPath = QDir::toNativeSeparators(humanPath);

        UBApplication::mainWindow->warning(tr("Document Repository Loss"),tr("Sankore has lost access to the document repository '%1'. Unfortunately the application must shut down to avoid data corruption. Latest changes may be lost as well.").arg(humanPath));

        UBApplication::quit();
    }
}


bool UBPersistenceManager::mayHaveVideo(UBDocumentProxy* pDocumentProxy)
{
    QDir videoDir(pDocumentProxy->persistencePath() + "/" + UBPersistenceManager::videoDirectory);

    return videoDir.exists() && videoDir.entryInfoList().length() > 0;
}

bool UBPersistenceManager::mayHaveAudio(UBDocumentProxy* pDocumentProxy)
{
    QDir audioDir(pDocumentProxy->persistencePath() + "/" + UBPersistenceManager::audioDirectory);

    return audioDir.exists() && audioDir.entryInfoList().length() > 0;
}

bool UBPersistenceManager::mayHavePDF(UBDocumentProxy* pDocumentProxy)
{
    QDir objectDir(pDocumentProxy->persistencePath() + "/" + UBPersistenceManager::objectDirectory);

    QStringList filters;
    filters << "*.pdf";

    return objectDir.exists() && objectDir.entryInfoList(filters).length() > 0;
}


bool UBPersistenceManager::mayHaveSVGImages(UBDocumentProxy* pDocumentProxy)
{
    QDir imageDir(pDocumentProxy->persistencePath() + "/" + UBPersistenceManager::imageDirectory);

    QStringList filters;
    filters << "*.svg";

    return imageDir.exists() && imageDir.entryInfoList(filters).length() > 0;
}


bool UBPersistenceManager::mayHaveWidget(UBDocumentProxy* pDocumentProxy)
{
    QDir widgetDir(pDocumentProxy->persistencePath() + "/" + UBPersistenceManager::widgetDirectory);

    return widgetDir.exists() && widgetDir.entryInfoList(QDir::Dirs).length() > 0;
}

void UBPersistenceManager::persistTeacherBar(UBDocumentProxy* pDocumentProxy, int page, sTeacherBarInfos infos)
{
    if(NULL != pDocumentProxy)
    {
        QFile f(pDocumentProxy->persistencePath() + UBFileSystemUtils::digitFileFormat("/page%1.svg", page + 1));
        if(f.exists())
        {
            if(f.open(QIODevice::ReadOnly))
            {
                QDomDocument domDoc;
                QByteArray read = f.readAll();
                if(domDoc.setContent(read))
                {
                    f.close();
                    if(f.open(QIODevice::WriteOnly))
                    {
                        QDomElement rootElem = domDoc.documentElement();
                        QDomNode teacherBarNode = rootElem.namedItem("teacherBar");
                        QDomElement teacherBarElem;
                        if(!teacherBarNode.isNull()) {
                            rootElem.removeChild(teacherBarNode);
                        }

                        teacherBarElem = domDoc.createElement("teacherBar");
                        rootElem.appendChild(teacherBarElem);
                        teacherBarElem.setAttribute("version","1.40");

                        QDomElement title = domDoc.createElement("title");
                        title.setAttribute("value",infos.title);
                        title.setAttribute(gId,0);
                        teacherBarElem.appendChild(title);

                        QDomElement comment = domDoc.createElement("comment");
                        comment.setAttribute("value", infos.comments);
                        comment.setAttribute(gId,1);
                        teacherBarElem.appendChild(comment);
                        int counter = 2;

                        for(int i=0; i<infos.actions.size(); i++){
                            QDomElement action = domDoc.createElement("action");
                            QStringList attribute = infos.actions.at(i).split(gSeparator);
                            action.setAttribute("owner", attribute.at(0).toInt());
                            action.setAttribute("task", attribute.at(1));
                            action.setAttribute(gId,counter++);
                            teacherBarElem.appendChild(action);
                        }


                        for(int j=0; j<infos.medias.size(); j++){
                            QDomElement media = domDoc.createElement("media");
                            QStringList attribute = infos.medias.at(j).split(gSeparator);
                            media.setAttribute("title", attribute.at(0));
                            media.setAttribute("relativePath", attribute.at(1));
                            media.setAttribute(gId,counter++);
                            teacherBarElem.appendChild(media);
                        }


                        for(int k=0; k<infos.urls.size(); k++){
                            QDomElement link = domDoc.createElement("link");
                            QStringList attribute = infos.urls.at(k).split(gSeparator);
                            link.setAttribute("title", attribute.at(0));
                            link.setAttribute("url", attribute.at(1));
                            link.setAttribute(gId,counter++);
                            teacherBarElem.appendChild(link);
                        }
                        // Save the file
                        f.write(domDoc.toString().toAscii());
                        f.close();
                    }
                }
                f.close();
            }
        }
    }
}

sTeacherBarInfos UBPersistenceManager::getTeacherBarInfos(UBDocumentProxy* pDocumentProxy, int page)
{
    sTeacherBarInfos infos;

    if(NULL != pDocumentProxy)
    {
        QFile f(pDocumentProxy->persistencePath() + UBFileSystemUtils::digitFileFormat("/page%1.svg", page + 1));
        if(f.exists())
        {
            if(f.open(QIODevice::ReadWrite))
            {
                QDomDocument domDoc;
                if(domDoc.setContent(f.readAll()))
                {
                    QDomElement rootElem = domDoc.documentElement();
                    QDomNode teacherBarNode = rootElem.namedItem("teacherBar");
                    QDomNodeList childList = teacherBarNode.childNodes();
                    QStringList medias;
                    QStringList actions;
                    QStringList urls;
                    for(int i = 0; i < childList.count() ; i +=1 ){
                        QDomElement element = childList.at(i).toElement();
                        QString name = element.nodeName();
                        qDebug() << name;
                        if(name == "title")
                            infos.title = element.attribute("value");
                        else if (name == "comment")
                            infos.comments = element.attribute("value");
                        else if (name == "media"){
                            QString media;
                            media = element.attribute("title") + gSeparator + element.attribute("relativePath");
                            medias << media;
                        }
                        else if (name == "action"){
                            QString action;
                            action = element.attribute("owner") + gSeparator + element.attribute("task");
                            actions << action;
                        }
                        else if (name == "link"){
                            QString url;
                            url = element.attribute("title") + gSeparator + element.attribute("url");
                            urls << url;
                        }
                        else
                            qCritical() << "Error reading the ubz file";
                    }
                    infos.medias = medias;
                    infos.actions = actions;
                    infos.urls = urls;
                }
                f.close();
            }
        }
    }

    return infos;
}
