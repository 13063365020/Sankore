#include "UBTeacherBarPreviewWidget.h"
#include "web/UBWebController.h"
#include "core/UBApplication.h"
#include "globals/UBGlobals.h"
#include "board/UBBoardController.h"
#include "frameworks/UBFileSystemUtils.h"


UBTeacherBarPreviewMedia::UBTeacherBarPreviewMedia(QWidget* parent, const char* name) : QWidget(parent)
{
    setObjectName(name);
    mWidget = new UBWidgetList(parent);
    mLayout.addWidget(mWidget);
    setLayout(&mLayout);
    mWidgetList.clear();
}

UBTeacherBarPreviewMedia::~UBTeacherBarPreviewMedia()
{
    DELETEPTR(mWidget);
}

void UBTeacherBarPreviewMedia::cleanMedia()
{
    foreach(QWidget* eachWidget, mWidgetList.keys()){
        if(QString(eachWidget->metaObject()->className()).contains("UBDraggable")){
            mWidget->removeWidget(eachWidget);
            delete eachWidget;
            eachWidget = NULL;
        }
        else{
            mWidget->removeWidget(eachWidget);
        }
    }
    mWidgetList.clear();
}

void UBTeacherBarPreviewMedia::loadWidgets(QList<QWidget*> pWidgetsList, bool isResizable)
{
    Q_UNUSED(isResizable);
    foreach(QWidget*eachWidget, pWidgetsList){
        mWidget->addWidget(eachWidget);
        mWidgetList[eachWidget]="DRAG UNAVAILABLE";
    }
}


//int UBTeacherBarPreviewMedia::loadMedia(QStringList pMedias)
//{
//    int addedMedia = 0;
////    foreach(QString eachString, pMedias){
////        if(!eachString.isEmpty()){
////            QString mimeType = UBFileSystemUtils::mimeTypeFromFileName(eachString);
////            if(mimeType.contains("image")){
////                UBDraggableLabel* label = new UBDraggableLabel();
////                label->loadImage(eachString);
////                mWidget->addWidget(label);
////                mWidgetList[label]=eachString;
////                addedMedia += 1;
////            }
////            else if(mimeType.contains("video") || mimeType.contains("audio")){
////                UBDraggableMediaPlayer* mediaPlayer = new UBDraggableMediaPlayer();
////                mediaPlayer->setFile(eachString);
////                mWidget->addWidget(mediaPlayer);
////                mWidgetList[mediaPlayer] = eachString;
////                addedMedia += 1;
////            }
////            else{
////                qWarning() << "pMediaPath" << eachString;
////                qWarning() << "bad idea to come here";
////            }
////        }
////    }
//    return addedMedia;
//}

// -----------------------------------------------------------------------------------------------
UBActionPreview::UBActionPreview(QWidget *parent, const char *name):QWidget(parent)
  , mpOwner(NULL)
  , mpContent(NULL)
{
    setObjectName(name);
    setLayout(&mLayout);
    mpOwner = new QLabel(this);
    mpOwner->setObjectName("UBActionPreviewOwner");
    mOwnerLayout.addWidget(mpOwner, 0);
    mOwnerLayout.addStretch(1);
    mLayout.addLayout(&mOwnerLayout);
    mpContent = new QTextEdit(this);
    mpContent->setReadOnly(true);
    mpContent->setObjectName("UBActionPreviewContent");
    //mpContent->setWordWrap(true);
    mLayout.addWidget(mpContent);
    setContentsMargins(-9, -9, -9, -9);
}

UBActionPreview::~UBActionPreview()
{
    if(NULL != mpOwner){
        delete mpOwner;
        mpOwner = NULL;
    }
    if(NULL != mpContent){
        delete mpContent;
        mpContent = NULL;
    }
}

void UBActionPreview::setOwner(int owner)
{
    if(NULL != mpOwner && NULL != mpContent){
        switch(owner){
            case eActionOwner_Teacher:
                mpOwner->setText(tr("Teacher"));
                mpContent->setStyleSheet("background:lightblue; border:lightblue;");
                break;

            case eActionOwner_Student:
                mpOwner->setText(tr("Student"));
                mpContent->setStyleSheet("background:lightgreen; border:lightgreen;");
                break;
        }
    }
}

void UBActionPreview::setContent(const QString &content)
{
    if(NULL != mpContent){
        mpContent->setText(content);
        setMinimumHeight(mpOwner->height() + mpContent->document()->documentLayout()->documentSize().toSize().height());
    }
}

// -------------------------------------------------------------------------------------------------------------------
UBTBPreviewContainer::UBTBPreviewContainer(QWidget *parent, const char *name):UBWidgetList(parent)
{
    setObjectName(name);
    mCanRemove = false;
}

UBTBPreviewContainer::~UBTBPreviewContainer()
{

}

// ------------------------------------------------------------------------------------
UBTeacherBarPreviewWidget::UBTeacherBarPreviewWidget(UBTeacherBarDataMgr* pDataMgr, QWidget *parent, const char *name):QWidget(parent)
  , mpEditButton(NULL)
  , mpDocumentButton(NULL)
  , mpSessionTitle(NULL)
  , mpTitle(NULL)
  , mpTitleLabel(NULL)
  , mpPageNbrLabel(NULL)
  , mpContentContainer(NULL)
  , mpScheduleLabel(NULL)
  , mpLicenseLabel(NULL)
  , mpExpander(NULL)
{
    setObjectName(name);
    mpDataMgr = pDataMgr;
    setLayout(&mLayout);
    mLayout.setContentsMargins(0, 0, 0, 0);

    setAttribute(Qt::WA_StyledBackground, true);
    setStyleSheet(UBApplication::globalStyleSheet());

    // Build the Preview widget
    mpContainer = new QWidget(this);
    mpContainer->setObjectName("UBTBPreviewWidget");
    mpContainer->setLayout(&mContainerLayout);
    mLayout.addWidget(mpContainer, 1);


    // Session Title
    mpTitleContainer = new QWidget(mpContainer);
    mpTitleContainer->setLayout(&mTitleLayout);
    mpSessionTitle = new QLabel(mpContainer);
    mpSessionTitle->setText(tr("Session: "));
    mpSessionTitle->setWordWrap(true);
    mpSessionTitle->setAlignment(Qt::AlignRight);
    mpSessionTitle->setObjectName("UBTBPreviewSessionTitle");
    mContainerLayout.addWidget(mpSessionTitle);

    // Title
    mpTitleContainer->setLayout(&mTitleLayout);
    mTitleLayout.setContentsMargins(0, 0, 0, 0);
    mpTitleLabel = new QLabel(mpTitleContainer);
    mpTitleLabel->setText(tr("Activity"));
    mpTitleLabel->setObjectName("UBTeacherBarPreviewSubtitle");
    mTitleLayout.addWidget(mpTitleLabel, 0);
    mpTitle = new QLabel(mpTitleContainer);
    mpTitle->setObjectName("UBTeacherBarPreviewTitle");
    mpTitle->setWordWrap(true);
    mpTitle->setAlignment(Qt::AlignLeft);
    mTitleLayout.addWidget(mpTitle, 1);
    mpPageNbrLabel = new QLabel(tr("Page n° "), mpTitleContainer);
    mpPageNbrLabel->setAlignment(Qt::AlignRight);
    mpPageNbrLabel->setObjectName("UBTBPreviewSessionTitle");
    mTitleLayout.addWidget(mpPageNbrLabel);
    mTitleLayout.addWidget(&mTitleSeparator);
    mContainerLayout.addWidget(mpTitleContainer);

    // Content
    mpContentContainer = new UBTBPreviewContainer(mpContainer);
    mContainerLayout.addWidget(mpContentContainer, 1);

    // License
    mContainerLayout.addWidget(&mLicenseSeparator);
    mpLicenseLabel = new UBTBLicenseWidget(mpContainer);
    mContainerLayout.addWidget(mpLicenseLabel);

    // Document Button
    mpDocumentButton = new QPushButton(tr("Document View"), this);
    mpDocumentButton->setObjectName("DockPaletteWidgetButton");

    // Edit Button
    mpEditButton = new QPushButton(tr("Edit"), this);
    mpEditButton->setObjectName("DockPaletteWidgetButton");
    mEditLayout.addWidget(mpDocumentButton, 0);
    mEditLayout.addWidget(mpEditButton, 0);
    mEditLayout.addStretch(1);
    mLayout.addLayout(&mEditLayout, 0);

    mpExpander = new UBMediaExpander(this);

    connect(mpExpander, SIGNAL(toggleMedias()), this, SLOT(onToggleMedias()));
    connect(mpEditButton, SIGNAL(clicked()), this, SLOT(onEdit()));
    connect(mpDocumentButton, SIGNAL(clicked()), this, SLOT(onDocumentClicked()));
    connect(UBApplication::boardController, SIGNAL(activeSceneChanged()), this, SLOT(onActiveSceneChanged()));

    mpDocumentButton->hide();
}

UBTeacherBarPreviewWidget::~UBTeacherBarPreviewWidget()
{
}

void UBTeacherBarPreviewWidget::onActiveSceneChanged()
{
    mpPageNbrLabel->setText(tr("Page n° %0").arg(UBApplication::boardController->activeSceneIndex()));
}

void UBTeacherBarPreviewWidget::onEdit()
{
    foreach(sMedia media, *mpDataMgr->medias()){
        media.widget->setVizualisationMode(eVizualisationMode_Edit);
    }

    emit showEditMode();
}

void UBTeacherBarPreviewWidget::onDocumentClicked()
{
    emit showDocumentPreview();
}

void UBTeacherBarPreviewWidget::updateFields()
{
    // Session Title
    if("" != mpDataMgr->sessionTitle()){
        mpSessionTitle->setText(mpDataMgr->sessionTitle());
        mpSessionTitle->setVisible(true);
    }else{
        mpSessionTitle->setVisible(false);
    }

    // Page Title
    if("" != mpDataMgr->pageTitle()){
        mpTitle->setText(mpDataMgr->pageTitle());
        mpPageNbrLabel->setText(tr("Page n° %0").arg(UBApplication::boardController->activeSceneIndex()));
        mpTitleContainer->setVisible(true);
    }else{
        mpTitleContainer->setVisible(false);
    }

    // Comments
    generateComments();

    // Actions
    generateActions();

    // The media expander
    mpContentContainer->addWidget(mpExpander);
    // Media
    //generateMedias();
    onToggleMedias();

    // Links
    //generateLinks();

    // License
    mpLicenseLabel->setLicense(mpDataMgr->sessionLicence());

}

void UBTeacherBarPreviewWidget::clearFields()
{
    // Session Title
    mpSessionTitle->setText("");

    // Page Title
    mpTitle->setText("");

    // Medias
    if(!mStoredWidgets.empty()){
        foreach(QWidget* pW, mStoredWidgets){
            mpContentContainer->removeWidget(pW);
            DELETEPTR(pW);
        }
        mStoredWidgets.clear();
    }

    // License
    mpLicenseLabel->setLicense(eLicense_CCBY);
}

void UBTeacherBarPreviewWidget::generateActions()
{
    if(!mpDataMgr->actions()->empty()){
        foreach(sAction act, *mpDataMgr->actions()){
            mpTmpAction = new UBActionPreview(this);
            mpTmpAction->setOwner(act.type);
            mpTmpAction->setContent(act.content);
            mpContentContainer->addWidget(mpTmpAction);
            mStoredWidgets << mpTmpAction;
        }
    }
}

void UBTeacherBarPreviewWidget::generateMedias()
{
    if(isVisible()){
        foreach(sMedia media, *mpDataMgr->medias()){
            QString mediaUrl = media.url;
            QString mimeType = UBFileSystemUtils::mimeTypeFromFileName(mediaUrl);
            if(mimeType.contains("image")){
                UBDraggableMedia* mediaPlayer = new UBDraggableMedia(eMediaType_Picture);
                mediaPlayer->setUrl(mediaUrl);
                mediaPlayer->setFile(mediaUrl);
                mediaPlayer->createMediaPlayer();
                mediaPlayer->setTitle(media.title);
                mediaPlayer->setVizualisationMode(eVizualisationMode_Full);
                mStoredWidgets << mediaPlayer;
                mMedias << mediaPlayer;
                mpContentContainer->addWidget(mediaPlayer);
            }
            else if(mimeType.contains("video") || mimeType.contains("audio")){
                UBDraggableMedia* mediaPlayer = new UBDraggableMedia(mimeType.contains("audio")?eMediaType_Audio:eMediaType_Video);
                mediaPlayer->setUrl(mediaUrl);
                mediaPlayer->setFile(mediaUrl);
                mediaPlayer->createMediaPlayer();
                mediaPlayer->setTitle(media.title);
                mediaPlayer->setVizualisationMode(eVizualisationMode_Full);
                mStoredWidgets << mediaPlayer;
                mMedias << mediaPlayer;
                mpContentContainer->addWidget(mediaPlayer);
            }
        }
    }
}

void UBTeacherBarPreviewWidget::onLinkActivated(const QString& url)
{
    qDebug() << url;
    UBApplication::webController->loadUrl(QUrl(url));
}

void UBTeacherBarPreviewWidget::generateLinks()
{
    if(!mpDataMgr->urls()->empty()){
        foreach(sLink link, *mpDataMgr->urls()){
            mpTmpLink = new QLabel(QString("<a href='%0'>%1</a>").arg(link.link).arg(link.title), this);
            mpTmpLink->setObjectName("UBLinkPreview");
            mpTmpLink->setOpenExternalLinks(false);
            mpTmpLink->setTextInteractionFlags(Qt::LinksAccessibleByMouse);
            connect(mpTmpLink,SIGNAL(linkActivated(const QString &)), this,SLOT(onLinkActivated(const QString &)));
            mpContentContainer->addWidget(mpTmpLink);
            mStoredWidgets << mpTmpLink;
            mLinks << mpTmpLink;
        }
    }
}

void UBTeacherBarPreviewWidget::generateComments()
{
    if("" != mpDataMgr->comments()){
        mpTmpComment = new QTextEdit(this);
        mpTmpComment->setObjectName("UBCommentPreview");
        mpTmpComment->setPlainText(mpDataMgr->comments());
        mpTmpComment->setReadOnly(true);
        mpContentContainer->addWidget(mpTmpComment);
        mStoredWidgets << mpTmpComment;
    }
}

void UBTeacherBarPreviewWidget::showEvent(QShowEvent* ev)
{
    Q_UNUSED(ev);
    updateFields();
}

void UBTeacherBarPreviewWidget::onToggleMedias()
{
    if(mpExpander->isExpanded()){
        generateMedias();
        generateLinks();
    }else{
        // Remove the media
        foreach(UBDraggableMedia* media, mMedias){
            mpContentContainer->removeWidget(media);
            mStoredWidgets.removeOne(media);
            DELETEPTR(media);
        }
        mMedias.clear();

        // Remove the links
        foreach(QLabel* link, mLinks){
            mpContentContainer->removeWidget(link);
            mStoredWidgets.removeOne(link);
            DELETEPTR(link);
        }
        mLinks.clear();
    }
}
