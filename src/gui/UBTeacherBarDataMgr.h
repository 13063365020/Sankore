#ifndef UBTEACHERBARDATAMGR_H
#define UBTEACHERBARDATAMGR_H

#include <QString>
#include <QWidget>
#include <QTextEdit>
#include <QLineEdit>
#include <QLabel>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QComboBox>
#include <QFrame>

#include "customWidgets/UBMediaWidget.h"

typedef enum{
    eTeacherBarState_DocumentEdit,
    eTeacherBarState_DocumentPreview,
    eTeacherBarState_PageEdit,
    eTeacherBarState_PagePreview
}eTeacherBarState;

typedef enum{
    eActionOwner_Teacher,
    eActionOwner_Student
}eActionOwner;

typedef struct{
    int type;
    QString content;
}sAction;

typedef struct{
    QString title;
    QString link;
}sLink;

typedef enum{
    eLicense_CCBY,
    eLicense_CCBYND,
    eLicense_CCBYNCSA,
    eLicense_CCBYSA,
    eLicense_CCBYNC,
    eLicense_CCBYNCND
}eLicense;

typedef struct{
    UBMediaWidget* widget;
    QString url;
    QString title;
}sMedia;

class UBTBSeparator : public QFrame
{
public:
    UBTBSeparator(QWidget* parent=0, const char* name="UBTBSeparator");
    ~UBTBSeparator();
};

class UBTBLicenseWidget : public QWidget
{
public:
    UBTBLicenseWidget(QWidget* parent=0, const char* name="UBTBLicenseWidget");
    ~UBTBLicenseWidget();
    void setLicense(eLicense lic);

private:
    QHBoxLayout mLayout;
    QLabel* mpIcon;
    QLabel* mpText;
};

class UBTeacherBarDataMgr
{
public:
    UBTeacherBarDataMgr();
    ~UBTeacherBarDataMgr();
    bool couldSaveContent;

    // Session Title
    void setSessionTitle(const QString& title){mSessionTitle = title;}
    QString sessionTitle(){return mSessionTitle;}

    // Session Target
    void setSessionTarget(const QString& target){mSessionTarget = target;}
    QString sessionTarget(){return mSessionTarget;}

    // Licence
    void setSessionLicence(eLicense licence){mSessionLicence = licence;}
    eLicense sessionLicence(){return mSessionLicence;}

    // Page Title
    void setPageTitle(const QString& title){mPageTitle = title;}
    QString pageTitle(){return mPageTitle;}

    // Actions
    QVector<sAction>* actions(){return &mActionList;}

    // Medias
    QVector<sMedia>* medias(){return &mMediaList;}

    // Urls
    QVector<sLink>* urls(){return &mUrlList;}

    // Comments
    void setComments(const QString& c){mComments = c;}
    QString comments(){return mComments;}

    // Keywords
    void setKeywords(const QString& kw){mKeywords = kw;}
    QString keywords(){return mKeywords;}

    // Level
    void setLevel(const QString& level){mLevel = level;}
    QString level(){return mLevel;}

    // Topic
    void setTopic(const QString& topic){mTopic = topic;}
    QString topic(){return mTopic;}

    // Authors
    void setAuthors(const QString& authors){mAuthors = authors;}
    QString authors(){return mAuthors;}

    // Creation Date
    void setCreationDate(const QString& date){mCreationDate = date;}
    QString creationDate(){return mCreationDate;}

    // Last Modification
    void setLastModificationDate(const QString& date){mLastModif = date;}
    QString lastModificationDate(){return mLastModif;}

    // Others
    void clearLists();
    void saveContent();
    void loadContent(bool docChanged = false);

private:
    QString mSessionTitle;
    QString mSessionTarget;
    eLicense mSessionLicence;
    QString mPageTitle;
    QString mComments;
    QString mKeywords;
    QString mLevel;
    QString mTopic;
    QString mAuthors;
    QString mCreationDate;
    QString mLastModif;

    QVector<sAction> mActionList;
    QVector<sLink> mUrlList;
    QVector<sMedia> mMediaList;

};

#endif // UBTEACHERBARDATAMGR_H
