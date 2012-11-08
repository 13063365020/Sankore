/*
 * UBActionPalette.h
 *
 *  Created on: 8 oct. 2009
 *      Author: Luc
 */

#ifndef UBACTIONPALETTE_H_
#define UBACTIONPALETTE_H_

#include <QtGui>
#include <QPoint>
#include <QButtonGroup>
#include "UBFloatingPalette.h"

class UBActionPaletteButton;
class UBActionPaletteMultiStateButton;

class UBActionPalette : public UBFloatingPalette
{
    Q_OBJECT;

    public:
        UBActionPalette(QList<QAction*> actions, Qt::Orientation orientation = Qt::Vertical, QWidget* parent = 0);
        UBActionPalette(Qt::Orientation orientation, QWidget* parent = 0);
        UBActionPalette(Qt::Corner corner, QWidget* parent = 0, Qt::Orientation orient = Qt::Vertical);
        UBActionPalette(QWidget* parent = 0);

        virtual ~UBActionPalette();

        void setButtonIconSize(const QSize& size);
        void setToolButtonStyle(Qt::ToolButtonStyle);

        QList<QAction*> actions();
        void setActions(QList<QAction*> actions);
        void groupActions();
        void addAction(QAction* action, bool addToGroup = true);
        void addActions(QList<QAction*> actions, bool addToGroup = true);

        void setClosable(bool closable);
        void setAutoClose(bool autoClose)
        {
            mAutoClose = autoClose;
        }

        void setCustomCloseProcessing(bool customCloseProcessing)
        {
            m_customCloseProcessing = customCloseProcessing;
        }
        bool m_customCloseProcessing;

        virtual int border();
        void clearLayout();
        QSize buttonSize();

        UBActionPaletteButton* getButtonFromAction(QAction* action);

        QButtonGroup *buttonGroup() { return mButtonGroup; }

    public slots:
        void close();

    signals:
        void closed();
        void buttonGroupClicked(int id);
        void customMouseReleased();

    protected:
        virtual void paintEvent(QPaintEvent *event);
        virtual void mouseReleaseEvent(QMouseEvent * event);
        virtual void init(Qt::Orientation orientation);

        void updateLayout();

        QList<UBActionPaletteButton*> mButtons;
        QButtonGroup* mButtonGroup;
        QList<QAction*> mActions;
        QMap<QAction*, UBActionPaletteButton*> mMapActionToButton;

        bool mIsClosable;
        Qt::ToolButtonStyle mToolButtonStyle;
        bool mAutoClose;
        QSize mButtonSize;
        QPoint mMousePos;
        UBActionPaletteMultiStateButton *createPaletteButton(QList<QAction*> actions, QWidget *parent);
        UBActionPaletteButton *createPaletteButton(QAction* action, QWidget *parent);

    private slots:
        void buttonClicked(int id);
        void buttonClicked();
        void actionChanged();
};

class UBActionPaletteButton : public QToolButton
{
    Q_OBJECT

    public:
        UBActionPaletteButton(QAction* action, QWidget * parent = 0);
        virtual ~UBActionPaletteButton();

    signals:
        void doubleClicked();

    protected:
        virtual void mouseDoubleClickEvent(QMouseEvent *event);
        virtual bool hitButton(const QPoint &pos) const;

};


class UBActionPaletteMultiStateButton : public UBActionPaletteButton
{
    Q_OBJECT

public:
    UBActionPaletteMultiStateButton(QList<QAction*> actions, QWidget *parent = 0);
    virtual ~UBActionPaletteMultiStateButton();

private:
    virtual void nextCheckState();

private slots:
    void setActiveAction(QAction* action);

private:
    int mCurrentAction;
    QList<QAction *> mActions;
    QActionGroup *mActionsGroup;
};

#endif /* UBACTIONPALETTE_H_ */
