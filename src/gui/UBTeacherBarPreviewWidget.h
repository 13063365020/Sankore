#ifndef UBTEACHERBARPREVIEWWIDGET_H
#define UBTEACHERBARPREVIEWWIDGET_H

#include <QLabel>
#include <QVBoxLayout>
#include <QPushButton>

#include "core/UBPersistenceManager.h"
#include "customWidgets/UBWidgetList.h"
#include "customWidgets/UBMediaWidget.h"
#include "customWidgets/UBDraggableMedia.h"
#include "customWidgets/UBDraggableLabel.h"
#include "UBTeacherBarDataMgr.h"

class UBTeacherBarPreviewMedia : public QWidget
{
    Q_OBJECT
public:
    UBTeacherBarPreviewMedia(QWidget* parent=0, const char* name="UBTeacherBarPreviewMedia");
    ~UBTeacherBarPreviewMedia();
//    int loadMedia(QStringList pMedias);
    void loadWidgets(QList<QWidget*> pWidgetList, bool isResizable = true);
    void cleanMedia();

private:
    UBWidgetList* mWidget;
    QVBoxLayout mLayout;
    QMap<QWidget*,QString>mWidgetList;
};

class UBActionPreview : public QWidget
{
public:
    UBActionPreview(QWidget* parent=0, const char* name="UBActionPreview");
    ~UBActionPreview();
    void setOwner(int owner);
    void setContent(const QString& content);

private:
    QLabel* mpOwner;
    QTextEdit* mpContent;

    QVBoxLayout mLayout;
    QHBoxLayout mOwnerLayout;
};

class UBTBPreviewContainer : public UBWidgetList
{
public:
    UBTBPreviewContainer(QWidget* parent=0, const char* name="UBTBPreviewContainer");
    ~UBTBPreviewContainer();
};

class UBTeacherBarPreviewWidget : public QWidget
{
    Q_OBJECT
public:
    UBTeacherBarPreviewWidget(UBTeacherBarDataMgr* pDataMgr, QWidget* parent=0, const char* name="UBTeacherBarPreviewWidget");
    ~UBTeacherBarPreviewWidget();
    void updateFields();
    void clearFields();

signals:
    void showEditMode();
    void showDocumentPreview();

protected:
    void showEvent(QShowEvent* ev);

private slots:
    void onEdit();
    void onActiveSceneChanged();
    void onDocumentClicked();
    void onToggleMedias();
    void onLinkActivated(const QString &url);

private:
    void generateActions();
    void generateMedias();
    void generateLinks();
    void generateComments();

    QVBoxLayout mLayout;
    QVBoxLayout mContainerLayout;
    QHBoxLayout mEditLayout;
    QPushButton* mpEditButton;
    QPushButton* mpDocumentButton;

    QWidget* mpContainer;

    // Titles
    QVBoxLayout mTitleLayout;
    UBTBSeparator mTitleSeparator;
    QWidget* mpTitleContainer;
    QLabel* mpSessionTitle;
    QLabel* mpTitle;
    QLabel* mpTitleLabel;
    QLabel* mpPageNbrLabel;
    UBTBPreviewContainer* mpContentContainer;

    // Schedule
    QLabel* mpScheduleLabel;

    // License
    UBTBSeparator mLicenseSeparator;
    UBTBLicenseWidget* mpLicenseLabel;


    /** Pointer to the datas */
    UBTeacherBarDataMgr* mpDataMgr;
    /** The list of stored widgets */
    QList<QWidget*> mStoredWidgets;
    /** A temporary action widget */
    UBActionPreview* mpTmpAction;
    /** A temporary media widget */
    UBTeacherBarPreviewMedia* mpTmpMedia;
    /** A temporary link */
    QLabel* mpTmpLink;
    /** A temporary comments field */
    QTextEdit* mpTmpComment;
    /** A temporary media object */
    UBDraggableMedia* mTmpMedia;
    /** A temporary label object */
    UBDraggableLabel* mpTmpLabel;
    /** A media expander */
    UBMediaExpander* mpExpander;

    QList<UBDraggableMedia*> mMedias;
    QList<QLabel*> mLinks;
};

#endif // UBTEACHERBARPREVIEWWIDGET_H
