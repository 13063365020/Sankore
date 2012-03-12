#include "globals/UBGlobals.h"
#include "core/UBApplication.h"
#include "frameworks/UBFileSystemUtils.h"
#include "gui/UBMediaPlayer.h"
#include "customWidgets/UBMediaWidget.h"

#include "UBTBPageEditWidget.h"

UBTBPageEditWidget::UBTBPageEditWidget(UBTeacherBarDataMgr *pDataMgr, QWidget *parent, const char *name):QWidget(parent)
  , mpDataMgr(NULL)
  , mpTitleLabel(NULL)
  , mpTitle(NULL)
  , mpMediaLabel(NULL)
  , mpActionLabel(NULL)
  , mpActions(NULL)
  , mpActionButton(NULL)
  , mpLinkLabel(NULL)
  , mpLinks(NULL)
  , mpLinkButton(NULL)
  , mpCommentLabel(NULL)
  , mpComments(NULL)
  , mpDocumentEditbutton(NULL)
  , mpPagePreviewButton(NULL)
  , mpContainer(NULL)
{
    Q_UNUSED(name);
    mpDataMgr = pDataMgr;
    mActions.clear();
    mUrls.clear();
    setAttribute(Qt::WA_StyledBackground, true);
    setStyleSheet(UBApplication::globalStyleSheet());
    mClearingFields = false;
    mLayout.setContentsMargins(0, 0, 0, 0);
    setLayout(&mLayout);

    mpContainer = new QWidget(this);
    mpContainer->setObjectName("DockPaletteWidgetBox");
    mpContainer->setLayout(&mContainerLayout);
    mLayout.addWidget(mpContainer, 1);

    // Title
    mpTitleLabel = new QLabel(tr("Title"), mpContainer);
    mpTitle = new QLineEdit(mpContainer);
    mpTitle->setObjectName("DockPaletteWidgetLineEdit");
    mContainerLayout.addWidget(mpTitleLabel, 0);
    mContainerLayout.addWidget(mpTitle, 0);

    // Comments
    mpCommentLabel = new QLabel(tr("Comments"), mpContainer);
    mContainerLayout.addWidget(mpCommentLabel, 0);
    mpComments = new QTextEdit(mpContainer);
    mpComments->setObjectName("DockPaletteWidgetBox");
    mpComments->setStyleSheet("background:white;");
    mContainerLayout.addWidget(mpComments, 1);

    // Actions
    mpActionLabel = new QLabel(tr("Actions"), mpContainer);
    mContainerLayout.addWidget(mpActionLabel, 0);
    mpActions = new UBWidgetList(mpContainer);
    mpActions->setEmptyText(tr("Add actions"));
    mContainerLayout.addWidget(mpActions, 1);
    mpActionButton = new QPushButton(mpContainer);
    mpActionButton->setObjectName("DockPaletteWidgetButton");
    mpActionButton->setText(tr("Add action"));
    mActionLayout.addWidget(mpActionButton, 0);
    mActionLayout.addStretch(1);
    mContainerLayout.addLayout(&mActionLayout, 0);

    // Media
    mpMediaLabel = new QLabel(tr("Medias"), mpContainer);
    mContainerLayout.addWidget(mpMediaLabel, 0);
    mpMediaContainer = new UBTBMediaContainer(mpContainer);
    mpMediaContainer->setEmptyText(tr("Drop media here"));
    mContainerLayout.addWidget(mpMediaContainer, 1);

    // Links
    mpLinkLabel = new QLabel(tr("Links"), mpContainer);
    mContainerLayout.addWidget(mpLinkLabel, 0);
    mpLinks = new UBWidgetList(mpContainer);
    mContainerLayout.addWidget(mpLinks, 1);
    mpLinkButton = new QPushButton(tr("Add link"), mpContainer);
    mpLinkButton->setObjectName("DockPaletteWidgetButton");
    mLinkLayout.addWidget(mpLinkButton, 0);
    mLinkLayout.addStretch(1);
    mContainerLayout.addLayout(&mLinkLayout, 0);

    mpPagePreviewButton = new QPushButton(tr("Preview"), this);
    mpPagePreviewButton->setObjectName("DockPaletteWidgetButton");
    mpDocumentEditbutton = new QPushButton(tr("Document View"), this);
    mpDocumentEditbutton->setObjectName("DockPaletteWidgetButton");
    mPagePreviewLayout.addWidget(mpDocumentEditbutton, 0);
    mPagePreviewLayout.addWidget(mpPagePreviewButton, 0);
    mPagePreviewLayout.addStretch(1);
    mLayout.addLayout(&mPagePreviewLayout, 0);

    connect(mpTitle, SIGNAL(textChanged(QString)), this, SLOT(onTitleChanged()));
    connect(mpComments, SIGNAL(textChanged()), this, SLOT(onCommentsChanged()));
    connect(mpActionButton, SIGNAL(clicked()), this, SLOT(onActionButton()));
    connect(mpLinkButton, SIGNAL(clicked()), this, SLOT(onLinkButton()));
    connect(mpDocumentEditbutton, SIGNAL(clicked()), this, SLOT(onDocumentEditClicked()));
    connect(mpPagePreviewButton, SIGNAL(clicked()), this, SLOT(onPagePreviewClicked()));
    connect(mpMediaContainer, SIGNAL(mediaDropped(QString)), this, SLOT(onMediaDropped(QString)));
    connect(mpActions, SIGNAL(closeWidget(QWidget*)), this, SLOT(onCloseWidget(QWidget*)));
    connect(mpLinks, SIGNAL(closeWidget(QWidget*)), this, SLOT(onCloseWidget(QWidget*)));
    connect(mpMediaContainer, SIGNAL(closeWidget(QWidget*)), this, SLOT(onCloseWidget(QWidget*)));

    mpDocumentEditbutton->hide();
}

UBTBPageEditWidget::~UBTBPageEditWidget()
{
    DELETEPTR(mpDocumentEditbutton);
    DELETEPTR(mpPagePreviewButton);
    DELETEPTR(mpComments);
    DELETEPTR(mpCommentLabel);
    DELETEPTR(mpLinks);
    DELETEPTR(mpLinkLabel);
    DELETEPTR(mpLinkButton);
    DELETEPTR(mpMediaLabel);
    DELETEPTR(mpActionButton);
    DELETEPTR(mpActionLabel);
    DELETEPTR(mpTitleLabel);
    DELETEPTR(mpTitle);
}

void UBTBPageEditWidget::onTitleChanged()
{
    if(!mClearingFields){
        mpDataMgr->setPageTitle(mpTitle->text());
        emit valueChanged();
    }
}

void UBTBPageEditWidget::onCommentsChanged()
{
    if(!mClearingFields){
        mpDataMgr->setComments(mpComments->document()->toPlainText());
        emit valueChanged();
    }
}

void UBTBPageEditWidget::onActionButton()
{
    UBTeacherStudentAction* pAction = new UBTeacherStudentAction(this);
    mActions << pAction;
    mpActions->addWidget(pAction);
    connectActions(pAction);
    emit valueChanged();
}

void UBTBPageEditWidget::onLinkButton()
{
    UBUrlWidget* pUrl = new UBUrlWidget(this);
    mUrls << pUrl;
    mpLinks->addWidget(pUrl);
    connectActions(pUrl);
    emit valueChanged();
}

void UBTBPageEditWidget::onMediaDropped(const QString &url)
{
    if("" != url){
        UBMediaWidget* pMedia = mpMediaContainer->generateMediaWidget(url);
        if(NULL != pMedia){
            sMedia media;
            media.widget = pMedia;
            media.title = pMedia->title();
            media.url = url;
            mMedias << media;
            //mMedias << pWidget;
            mpMediaContainer->addWidget(media.widget);
            connectActions(media.widget);
            emit valueChanged();
        }
    }
}

void UBTBPageEditWidget::connectActions(QWidget* w)
{
    UBActionableWidget* pActionable = dynamic_cast<UBActionableWidget*>(w);
    if(NULL != pActionable){
        connect(pActionable, SIGNAL(close(QWidget*)), this, SLOT(onCloseWidget(QWidget*)));
    }
}

void UBTBPageEditWidget::onDocumentEditClicked()
{
    emit changeTBState(eTeacherBarState_DocumentEdit);
}

void UBTBPageEditWidget::onPagePreviewClicked()
{
//    foreach(sMedia media, *mpDataMgr->medias()){
//        if(NULL != media.widget){
//            media.widget->setVizualisationMode(eVizualisationMode_Half);
//        }
//    }
    emit changeTBState(eTeacherBarState_PagePreview);
}

void UBTBPageEditWidget::saveFields()
{
    mpDataMgr->actions()->clear();
    mpDataMgr->urls()->clear();
    mpDataMgr->medias()->clear();

    foreach(UBTeacherStudentAction* pAct, mActions){
        sAction action;
        action.type = pAct->comboValue().toInt();
        action.content = pAct->text();
        mpDataMgr->actions()->append(action);
    }
    foreach(UBUrlWidget* pUrl, mUrls){
        sLink link;
        link.title = pUrl->title();
        link.link = pUrl->url();
        mpDataMgr->urls()->append(link);
    }
    foreach(sMedia media, mMedias){
        media.title = media.widget->title();
        // Update the title in the preview title
        media.widget->setTitle(media.title);
        media.url = media.widget->url();
        //media.widget->setVizualisationMode(eVizualisationMode_Half);
        mpDataMgr->medias()->append(media);
    }
}

void UBTBPageEditWidget::updateFields()
{
    // Title
    mpTitle->setText(mpDataMgr->pageTitle());
    // Actions
    foreach(sAction action, *mpDataMgr->actions()){
        UBTeacherStudentAction* pAction = new UBTeacherStudentAction(this);
        pAction->setComboValue(action.type);
        pAction->setText(action.content);
        mActions << pAction;
        mpActions->addWidget(pAction);
        connectActions(pAction);
    }
    // Medias
    foreach(sMedia media, *mpDataMgr->medias()){
        QString url = media.url;
        if(!url.isEmpty()){
            UBMediaWidget* pWidget = mpMediaContainer->generateMediaWidget(url);
            if(pWidget != NULL){
                media.widget = pWidget;
                media.widget->setVizualisationMode(eVizualisationMode_Edit);
                media.widget->setTitle(media.title);
                mMedias << media;
                mpMediaContainer->addWidget(media.widget);
                connectActions(media.widget);
            }
        }
    }

    // Links
    foreach(sLink link, *mpDataMgr->urls()){
        UBUrlWidget* urlWidget = new UBUrlWidget(this);
        urlWidget->setTitle(link.title);
        urlWidget->setUrl(link.link);
        mUrls << urlWidget;
        mpLinks->addWidget(urlWidget);
        connectActions(urlWidget);
    }
    // Comments
    mpComments->document()->setPlainText(mpDataMgr->comments());
}

void UBTBPageEditWidget::clearFields()
{
    mClearingFields = true;
    // Title
    mpTitle->setText("");
    // Actions
    foreach(UBTeacherStudentAction* pAction, mActions){
        mpActions->removeWidget(pAction);
        DELETEPTR(pAction);
    }
    mActions.clear();
    // Medias
    foreach(sMedia media, mMedias){
        QWidget* pMedia = media.widget;
        if(NULL != pMedia){
            mpMediaContainer->removeWidget(pMedia);
            DELETEPTR(pMedia);
        }
    }
    mMedias.clear();

    // Links
    foreach(UBUrlWidget* pLink, mUrls){
        mpLinks->removeWidget(pLink);
        DELETEPTR(pLink);
    }
    mUrls.clear();
    // Comments
    mpComments->setText("");

    mClearingFields = false;
}

void UBTBPageEditWidget::onCloseWidget(QWidget *w)
{
    if(NULL != w){
        if("UBTeacherStudentAction" == w->objectName()){
            UBTeacherStudentAction* pW = dynamic_cast<UBTeacherStudentAction*>(w);
            mpActions->removeWidget(pW);
            mActions.remove(mActions.indexOf(pW));
            DELETEPTR(w);
        }else if("UBUrlWidget" == w->objectName()){
            UBUrlWidget* pW = dynamic_cast<UBUrlWidget*>(w);
            mpLinks->removeWidget(pW);
            mUrls.remove(mUrls.indexOf(pW));
            DELETEPTR(w);
        }else if("UBTBMediaPicture" == w->objectName()){
            mpMediaContainer->removeWidget(w);
            removeMediaWidget(w);
        }else if("UBMediaWidget" == w->objectName()){
            mpMediaContainer->removeWidget(w);
            removeMediaWidget(w);
        }
    }
}

void UBTBPageEditWidget::removeMediaWidget(QWidget *w)
{
    for(int i=0; i<mMedias.size(); i++){
        sMedia media = mMedias.at(i);
        if(media.widget == w){
            mMedias.remove(i);
            DELETEPTR(w);
            break;
        }
    }
}

// ---------------------------------------------------------------------------------------------
UBUrlWidget::UBUrlWidget(QWidget *parent, const char *name):UBActionableWidget(parent, name)
  , mpLayout(NULL)
  , mpUrlLabel(NULL)
  , mpUrl(NULL)
{
    setAttribute(Qt::WA_StyledBackground, true);
    setStyleSheet(UBApplication::globalStyleSheet());
    addAction(eAction_Close);

    mpLayout = new QVBoxLayout(this);
    setLayout(mpLayout);

    mpLabelLayout = new QHBoxLayout(0);
    mpUrlLabel = new QLabel(tr("Url"), this);
    mpLabelLayout->addWidget(mpUrlLabel, 0);
    mpUrl = new QLineEdit(this);
    mpUrl->setObjectName("DockPaletteWidgetLineEdit");
    mpUrl->setMinimumHeight(20);
    mpLabelLayout->addWidget(mpUrl, 1);

    mpTitleLayout = new QHBoxLayout(0);
    mpTitleLabel = new QLabel(tr("Title"),this);
    mpTitleLayout->addWidget(mpTitleLabel,0);
    mpTitle = new QLineEdit(this);
    mpTitle->setObjectName("DockPaletteWidgetLineEdit");
    mpTitle->setMinimumHeight(20);
    mpTitleLayout->addWidget(mpTitle,1);

    mpLayout->addLayout(mpTitleLayout);
    mpLayout->addLayout(mpLabelLayout);
    setActionsParent(this);
}

UBUrlWidget::~UBUrlWidget()
{
    unsetActionsParent();
    DELETEPTR(mpTitle);
    DELETEPTR(mpTitleLabel);
    DELETEPTR(mpUrlLabel);
    DELETEPTR(mpUrl);
    DELETEPTR(mpTitleLayout);
    DELETEPTR(mpLabelLayout);
    DELETEPTR(mpLayout);
}

QString UBUrlWidget::url()
{
    QString str;

    if(NULL != mpUrl)
        str = mpUrl->text().startsWith("http://") ? mpUrl->text() : QString("http://" + mpUrl->text());
    return str;
}

void UBUrlWidget::setUrl(const QString &url)
{
    if(NULL != mpUrl){
        mpUrl->setText(url);
    }
}

//  ------------------------------------------------------------------------------------------------------------------------------------
UBTBMediaContainer::UBTBMediaContainer(QWidget *parent, const char *name) : UBWidgetList(parent)
{
    setObjectName(name);
    setAcceptDrops(true);
}

UBTBMediaContainer::~UBTBMediaContainer()
{

}

void UBTBMediaContainer::dropEvent(QDropEvent* pEvent)
{
    QPixmap pixFromDropEvent;
    QString mimeType;
    QString resourcePath;
    if(pEvent->mimeData()->hasText()){
        resourcePath = pEvent->mimeData()->text();
    }
    else if(pEvent->mimeData()->hasUrls()){
        resourcePath = pEvent->mimeData()->urls().at(0).toLocalFile();
    }
    else if(pEvent->mimeData()->hasImage()){
        pixFromDropEvent.loadFromData(pEvent->mimeData()->imageData().toByteArray());
        if(!pixFromDropEvent.isNull())
            mimeType = "image";
    }

    if (mimeType.isEmpty() && resourcePath.isEmpty()){
        pEvent->acceptProposedAction();
        return;
    }
    if(!resourcePath.isEmpty()){
        emit mediaDropped(resourcePath);
        pEvent->acceptProposedAction();
    }
}

void UBTBMediaContainer::dragEnterEvent(QDragEnterEvent* pEvent)
{
    pEvent->acceptProposedAction();
}

void UBTBMediaContainer::dragMoveEvent(QDragMoveEvent* pEvent)
{
    pEvent->acceptProposedAction();
}

void UBTBMediaContainer::dragLeaveEvent(QDragLeaveEvent* pEvent)
{
    pEvent->accept();
}

void UBTBMediaContainer::addMedia(const QString& mediaPath)
{
    if(!mediaPath.isEmpty())
        mMediaList.append(mediaPath);

    QString mimeType = UBFileSystemUtils::mimeTypeFromFileName(mediaPath);
    if(mimeType.contains("image")){
        QPixmap pix = QPixmap(mediaPath);
        QLabel* label = new QLabel();
        label->setPixmap(pix);
        label->setScaledContents(true);
        addWidget(label);
    }
    else if(mimeType.contains("video") || mimeType.contains("audio")){
        UBMediaPlayer* mediaPlayer = new UBMediaPlayer();
        mediaPlayer->setFile(mediaPath);
        addWidget(mediaPlayer);
    }
    else{
        qWarning() << "pMediaPath" << mediaPath;
        qWarning() << "bad idea to come here";
    }
}

QStringList UBTBMediaContainer::mediaUrls()
{
    return mMediaList;
}

void UBTBMediaContainer::cleanMedias()
{
    mMediaList.clear();
}

UBMediaWidget* UBTBMediaContainer::generateMediaWidget(const QString& url)
{
    UBMediaWidget* pW = NULL;

    if(!url.isEmpty())
        mMediaList.append(url);

    QString mimeType = UBFileSystemUtils::mimeTypeFromFileName(url);
    if(mimeType.contains("image")){
        UBMediaWidget* mediaPicture = new UBMediaWidget(eMediaType_Picture);
        mediaPicture->setUrl(url);
        mediaPicture->createMediaPlayer();
        pW = mediaPicture;
    }
    else if(mimeType.contains("video") || mimeType.contains("audio")){
        UBMediaWidget* mediaPlayer = new UBMediaWidget(mimeType.contains("audio")?eMediaType_Audio:eMediaType_Video);
        mediaPlayer->setFile(url);
        mediaPlayer->setUrl(url);
        mediaPlayer->createMediaPlayer();
        pW = mediaPlayer;
    }
    else{
        qWarning() << "pMediaPath" << url;
        qWarning() << "bad idea to come here";
    }

    return pW;
}

UBTeacherStudentAction::UBTeacherStudentAction(QWidget *parent, const char *name):UBActionableWidget(parent, name)
  , mpText(NULL)
  , mpLayout(NULL)
  , mpComboLayout(NULL)
  , mpCombo(NULL)
{
    setAttribute(Qt::WA_StyledBackground, true);
    setStyleSheet(UBApplication::globalStyleSheet());
    addAction(eAction_Close);

    // Create the GUI
    mpLayout = new QHBoxLayout(this);
    setLayout(mpLayout);

    mpComboLayout = new QVBoxLayout();

    mpCombo = new QComboBox(this);
    mpCombo->setObjectName("DockPaletteWidgetComboBox");
    mpCombo->setMinimumWidth(80);
    mpCombo->addItem(tr("Teacher"));
    mpCombo->addItem(tr("Student"));
    mpComboLayout->addWidget(mpCombo, 0);
    mpComboLayout->addStretch(1);

    mpLayout->addLayout(mpComboLayout, 0);

    mpText = new QTextEdit(this);
    mpText->setObjectName("DockPaletteWidgetBox");
    mpText->setStyleSheet("background:white;");

    mpLayout->addWidget(mpText, 1);
    setActionsParent(this);
}

UBTeacherStudentAction::~UBTeacherStudentAction()
{
    unsetActionsParent();
    DELETEPTR(mpCombo);
    DELETEPTR(mpText);
    DELETEPTR(mpComboLayout);
    DELETEPTR(mpLayout);
}

QString UBTeacherStudentAction::text()
{
    QString str;
    if(NULL != mpText){
        str = mpText->document()->toPlainText();
    }
    return str;
}

QString UBTeacherStudentAction::comboValue()
{
    QString str;

    if(NULL != mpCombo){
        str = QString("%0").arg(mpCombo->currentIndex());
    }

    return str;
}

void UBTeacherStudentAction::setComboValue(int value)
{
    if(NULL != mpCombo){
        mpCombo->setCurrentIndex(value);
    }
}

void UBTeacherStudentAction::setText(const QString& text)
{
    if(NULL != mpText){
        mpText->document()->setPlainText(text);
    }
}

// -------------------------------------------------------------
UBPictureWidget::UBPictureWidget(QWidget *parent, const char *name):UBActionableWidget(parent, name)
  , mpLayout(NULL)
  , mpLabel(NULL)
  , mpTitleLabel(NULL)
  , mpTitle(NULL)
{
    addAction(eAction_Close);
    mpLayout = new QVBoxLayout(this);
    setLayout(mpLayout);
    mpLayout->setContentsMargins(10, 0, 10, 0);
    mpTitleLabel = new QLabel(tr("Title"), this);
    mpLayout->addWidget(mpTitleLabel, 0);
    mpTitle = new QLineEdit(this);
    mpTitle->setObjectName("DockPaletteWidgetLineEdit");
    mpLayout->addWidget(mpTitle, 0);
    mpLabel = new QLabel(this);
    mpLayout->addWidget(mpLabel, 1);
    mpLabel->setGeometry( 10, 10, width()-2*10, height() - mpTitleLabel->height() - mpTitle->height() - 20);
    setActionsParent(mpLabel);
}

UBPictureWidget::~UBPictureWidget()
{
    unsetActionsParent();
    DELETEPTR(mpTitle);
    DELETEPTR(mpTitleLabel);
    DELETEPTR(mpLabel);
    DELETEPTR(mpLayout);
}

void UBPictureWidget::resizeEvent(QResizeEvent *ev)
{
    Q_UNUSED(ev);
    mpLabel->setGeometry( 10, 10, width()-2*10, height() - mpTitleLabel->height() - mpTitle->height() - 20);
}
