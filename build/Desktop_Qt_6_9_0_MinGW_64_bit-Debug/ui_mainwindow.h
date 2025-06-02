/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created by: Qt User Interface Compiler version 6.9.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QSlider>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QToolBar>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QAction *actionSelectTool;
    QAction *actionDrawLine;
    QAction *actionDrawRectangle;
    QAction *actionChangeColor;
    QAction *actionDrawFreehand;
    QAction *actionStrokeEraser;
    QAction *actionDraggingStrokeEraser;
    QAction *actionNormalEraser;
    QAction *actionChangeFillColor;
    QAction *actionUndo;
    QAction *actionRedo;
    QAction *actionExportImage;
    QAction *actionClearCanvas;
    QAction *actionOpenImage;
    QAction *actionDrawEllipse;
    QAction *actionDrawStar;
    QWidget *centralwidget;
    QWidget *penOptionsContainer;
    QHBoxLayout *horizontalLayout;
    QLabel *labelCurrentPenWidth;
    QSlider *sliderPenWidth;
    QMenuBar *menubar;
    QStatusBar *statusbar;
    QToolBar *ToolBar;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName("MainWindow");
        MainWindow->resize(800, 600);
        actionSelectTool = new QAction(MainWindow);
        actionSelectTool->setObjectName("actionSelectTool");
        actionSelectTool->setCheckable(true);
        QIcon icon;
        icon.addFile(QString::fromUtf8(":/icons/icons/arrow_selector.svg"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
        actionSelectTool->setIcon(icon);
        actionSelectTool->setMenuRole(QAction::MenuRole::NoRole);
        actionDrawLine = new QAction(MainWindow);
        actionDrawLine->setObjectName("actionDrawLine");
        actionDrawLine->setCheckable(true);
        QIcon icon1;
        icon1.addFile(QString::fromUtf8(":/icons/icons/line.svg"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
        actionDrawLine->setIcon(icon1);
        actionDrawLine->setMenuRole(QAction::MenuRole::NoRole);
        actionDrawRectangle = new QAction(MainWindow);
        actionDrawRectangle->setObjectName("actionDrawRectangle");
        actionDrawRectangle->setCheckable(true);
        QIcon icon2;
        icon2.addFile(QString::fromUtf8(":/icons/icons/square.svg"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
        actionDrawRectangle->setIcon(icon2);
        actionDrawRectangle->setMenuRole(QAction::MenuRole::NoRole);
        actionChangeColor = new QAction(MainWindow);
        actionChangeColor->setObjectName("actionChangeColor");
        QIcon icon3;
        icon3.addFile(QString::fromUtf8(":/icons/icons/color.svg"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
        actionChangeColor->setIcon(icon3);
        actionChangeColor->setMenuRole(QAction::MenuRole::NoRole);
        actionDrawFreehand = new QAction(MainWindow);
        actionDrawFreehand->setObjectName("actionDrawFreehand");
        actionDrawFreehand->setCheckable(true);
        QIcon icon4;
        icon4.addFile(QString::fromUtf8(":/icons/icons/freehand_pen.svg"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
        actionDrawFreehand->setIcon(icon4);
        actionDrawFreehand->setMenuRole(QAction::MenuRole::NoRole);
        actionStrokeEraser = new QAction(MainWindow);
        actionStrokeEraser->setObjectName("actionStrokeEraser");
        actionStrokeEraser->setCheckable(true);
        QIcon icon5;
        icon5.addFile(QString::fromUtf8(":/icons/icons/clicking_eraser.svg"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
        actionStrokeEraser->setIcon(icon5);
        actionStrokeEraser->setMenuRole(QAction::MenuRole::NoRole);
        actionDraggingStrokeEraser = new QAction(MainWindow);
        actionDraggingStrokeEraser->setObjectName("actionDraggingStrokeEraser");
        actionDraggingStrokeEraser->setCheckable(true);
        QIcon icon6;
        icon6.addFile(QString::fromUtf8(":/icons/icons/dragging_eraser.svg"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
        actionDraggingStrokeEraser->setIcon(icon6);
        actionDraggingStrokeEraser->setMenuRole(QAction::MenuRole::NoRole);
        actionNormalEraser = new QAction(MainWindow);
        actionNormalEraser->setObjectName("actionNormalEraser");
        actionNormalEraser->setCheckable(true);
        QIcon icon7;
        icon7.addFile(QString::fromUtf8(":/icons/icons/normal_eraser.svg"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
        actionNormalEraser->setIcon(icon7);
        actionNormalEraser->setMenuRole(QAction::MenuRole::NoRole);
        actionChangeFillColor = new QAction(MainWindow);
        actionChangeFillColor->setObjectName("actionChangeFillColor");
        actionChangeFillColor->setCheckable(false);
        QIcon icon8;
        icon8.addFile(QString::fromUtf8(":/icons/icons/inner_color.svg"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
        actionChangeFillColor->setIcon(icon8);
        actionChangeFillColor->setMenuRole(QAction::MenuRole::NoRole);
        actionUndo = new QAction(MainWindow);
        actionUndo->setObjectName("actionUndo");
        actionUndo->setEnabled(false);
        QIcon icon9;
        icon9.addFile(QString::fromUtf8(":/icons/icons/undo.svg"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
        actionUndo->setIcon(icon9);
        actionUndo->setMenuRole(QAction::MenuRole::NoRole);
        actionRedo = new QAction(MainWindow);
        actionRedo->setObjectName("actionRedo");
        actionRedo->setEnabled(false);
        QIcon icon10;
        icon10.addFile(QString::fromUtf8(":/icons/icons/redo.svg"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
        actionRedo->setIcon(icon10);
        actionRedo->setMenuRole(QAction::MenuRole::NoRole);
        actionExportImage = new QAction(MainWindow);
        actionExportImage->setObjectName("actionExportImage");
        QIcon icon11;
        icon11.addFile(QString::fromUtf8(":/icons/icons/save.svg"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
        actionExportImage->setIcon(icon11);
        actionExportImage->setMenuRole(QAction::MenuRole::NoRole);
        actionClearCanvas = new QAction(MainWindow);
        actionClearCanvas->setObjectName("actionClearCanvas");
        QIcon icon12;
        icon12.addFile(QString::fromUtf8(":/icons/icons/clear.svg"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
        actionClearCanvas->setIcon(icon12);
        actionClearCanvas->setMenuRole(QAction::MenuRole::NoRole);
        actionOpenImage = new QAction(MainWindow);
        actionOpenImage->setObjectName("actionOpenImage");
        QIcon icon13;
        icon13.addFile(QString::fromUtf8(":/icons/icons/file_open.svg"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
        actionOpenImage->setIcon(icon13);
        actionOpenImage->setMenuRole(QAction::MenuRole::NoRole);
        actionDrawEllipse = new QAction(MainWindow);
        actionDrawEllipse->setObjectName("actionDrawEllipse");
        actionDrawEllipse->setCheckable(true);
        QIcon icon14;
        icon14.addFile(QString::fromUtf8(":/icons/icons/ellipse.svg"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
        actionDrawEllipse->setIcon(icon14);
        actionDrawEllipse->setMenuRole(QAction::MenuRole::NoRole);
        actionDrawStar = new QAction(MainWindow);
        actionDrawStar->setObjectName("actionDrawStar");
        actionDrawStar->setCheckable(true);
        QIcon icon15;
        icon15.addFile(QString::fromUtf8(":/icons/icons/star.svg"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
        actionDrawStar->setIcon(icon15);
        actionDrawStar->setMenuRole(QAction::MenuRole::NoRole);
        centralwidget = new QWidget(MainWindow);
        centralwidget->setObjectName("centralwidget");
        penOptionsContainer = new QWidget(centralwidget);
        penOptionsContainer->setObjectName("penOptionsContainer");
        penOptionsContainer->setGeometry(QRect(0, 0, 162, 34));
        horizontalLayout = new QHBoxLayout(penOptionsContainer);
        horizontalLayout->setObjectName("horizontalLayout");
        labelCurrentPenWidth = new QLabel(penOptionsContainer);
        labelCurrentPenWidth->setObjectName("labelCurrentPenWidth");

        horizontalLayout->addWidget(labelCurrentPenWidth);

        sliderPenWidth = new QSlider(penOptionsContainer);
        sliderPenWidth->setObjectName("sliderPenWidth");
        sliderPenWidth->setMinimum(1);
        sliderPenWidth->setMaximum(20);
        sliderPenWidth->setValue(2);
        sliderPenWidth->setOrientation(Qt::Orientation::Horizontal);

        horizontalLayout->addWidget(sliderPenWidth);

        MainWindow->setCentralWidget(centralwidget);
        menubar = new QMenuBar(MainWindow);
        menubar->setObjectName("menubar");
        menubar->setGeometry(QRect(0, 0, 800, 21));
        MainWindow->setMenuBar(menubar);
        statusbar = new QStatusBar(MainWindow);
        statusbar->setObjectName("statusbar");
        MainWindow->setStatusBar(statusbar);
        ToolBar = new QToolBar(MainWindow);
        ToolBar->setObjectName("ToolBar");
        MainWindow->addToolBar(Qt::ToolBarArea::TopToolBarArea, ToolBar);

        ToolBar->addAction(actionOpenImage);
        ToolBar->addAction(actionExportImage);
        ToolBar->addSeparator();
        ToolBar->addAction(actionSelectTool);
        ToolBar->addAction(actionDrawFreehand);
        ToolBar->addAction(actionDrawLine);
        ToolBar->addAction(actionDrawRectangle);
        ToolBar->addAction(actionDrawEllipse);
        ToolBar->addAction(actionDrawStar);
        ToolBar->addSeparator();
        ToolBar->addAction(actionChangeColor);
        ToolBar->addAction(actionChangeFillColor);
        ToolBar->addSeparator();
        ToolBar->addAction(actionNormalEraser);
        ToolBar->addAction(actionStrokeEraser);
        ToolBar->addAction(actionDraggingStrokeEraser);
        ToolBar->addSeparator();
        ToolBar->addAction(actionUndo);
        ToolBar->addAction(actionRedo);
        ToolBar->addSeparator();
        ToolBar->addAction(actionClearCanvas);

        retranslateUi(MainWindow);

        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QCoreApplication::translate("MainWindow", "MainWindow", nullptr));
        actionSelectTool->setText(QCoreApplication::translate("MainWindow", "\351\200\211\346\213\251", nullptr));
#if QT_CONFIG(tooltip)
        actionSelectTool->setToolTip(QCoreApplication::translate("MainWindow", "<html><head/><body><p><span style=\" font-weight:700;\">\351\200\211\346\213\251\345\267\245\345\205\267 (Esc)</span></p><p>\347\202\271\345\207\273\344\273\245\351\200\211\344\270\255\347\224\273\345\270\203\344\270\212\347\232\204\345\233\276\345\275\242\357\274\214\346\210\226\346\213\226\345\212\250\344\273\245\347\247\273\345\212\250\351\200\211\344\270\255\347\232\204\345\233\276\345\275\242\343\200\202 </p></body></html>", nullptr));
#endif // QT_CONFIG(tooltip)
#if QT_CONFIG(shortcut)
        actionSelectTool->setShortcut(QCoreApplication::translate("MainWindow", "Esc", nullptr));
#endif // QT_CONFIG(shortcut)
        actionDrawLine->setText(QCoreApplication::translate("MainWindow", "\347\233\264\347\272\277", nullptr));
#if QT_CONFIG(tooltip)
        actionDrawLine->setToolTip(QCoreApplication::translate("MainWindow", "<html><head/><body><p><span style=\" font-weight:700;\">\347\273\230\345\210\266\347\233\264\347\272\277 (L)</span></p><p>\345\234\250\347\224\273\345\270\203\344\270\212\347\202\271\345\207\273\345\271\266\346\213\226\345\212\250\344\273\245\347\224\273\345\207\272\344\270\200\346\235\241\347\233\264\347\272\277\346\256\265\343\200\202</p></body></html>", nullptr));
#endif // QT_CONFIG(tooltip)
#if QT_CONFIG(shortcut)
        actionDrawLine->setShortcut(QCoreApplication::translate("MainWindow", "L", nullptr));
#endif // QT_CONFIG(shortcut)
        actionDrawRectangle->setText(QCoreApplication::translate("MainWindow", "\347\237\251\345\275\242", nullptr));
#if QT_CONFIG(tooltip)
        actionDrawRectangle->setToolTip(QCoreApplication::translate("MainWindow", "<html><head/><body><p><span style=\" font-weight:700;\">\347\273\230\345\210\266\347\237\251\345\275\242 (R)</span></p><p>\345\234\250\347\224\273\345\270\203\344\270\212\347\202\271\345\207\273\345\271\266\346\213\226\345\212\250\344\273\245\347\224\273\345\207\272\344\270\200\344\270\252\347\237\251\345\275\242\343\200\202</p></body></html>", nullptr));
#endif // QT_CONFIG(tooltip)
#if QT_CONFIG(shortcut)
        actionDrawRectangle->setShortcut(QCoreApplication::translate("MainWindow", "R", nullptr));
#endif // QT_CONFIG(shortcut)
        actionChangeColor->setText(QCoreApplication::translate("MainWindow", "\351\242\234\350\211\262", nullptr));
#if QT_CONFIG(tooltip)
        actionChangeColor->setToolTip(QCoreApplication::translate("MainWindow", "<html><head/><body><p><span style=\" font-weight:700;\">\346\233\264\346\224\271\350\276\271\346\241\206\351\242\234\350\211\262</span></p><p>\347\202\271\345\207\273\351\200\211\346\213\251\345\220\216\347\273\255\347\273\230\345\210\266\345\233\276\345\275\242\347\232\204\350\276\271\346\241\206\351\242\234\350\211\262\343\200\202</p></body></html>", nullptr));
#endif // QT_CONFIG(tooltip)
        actionDrawFreehand->setText(QCoreApplication::translate("MainWindow", "\350\207\252\347\224\261\347\224\273\347\254\224", nullptr));
#if QT_CONFIG(tooltip)
        actionDrawFreehand->setToolTip(QCoreApplication::translate("MainWindow", "<html><head/><body><p><span style=\" font-weight:700;\">\350\207\252\347\224\261\347\224\273\347\254\224 (P)</span></p><p>\345\234\250\347\224\273\345\270\203\344\270\212\346\214\211\344\275\217\345\271\266\346\213\226\345\212\250\344\273\245\350\207\252\347\224\261\347\273\230\345\210\266\346\233\262\347\272\277\343\200\202</p></body></html>", nullptr));
#endif // QT_CONFIG(tooltip)
        actionStrokeEraser->setText(QCoreApplication::translate("MainWindow", "\347\202\271\345\207\273\347\254\224\347\224\273\346\251\241\347\232\256\346\223\246", nullptr));
#if QT_CONFIG(tooltip)
        actionStrokeEraser->setToolTip(QCoreApplication::translate("MainWindow", "<html><head/><body><p><span style=\" font-weight:700;\">\347\254\224\347\224\273\346\223\246\351\231\244\357\274\210\347\202\271\345\207\273\357\274\211</span></p><p>\347\202\271\345\207\273\345\233\276\345\275\242\347\232\204\344\273\273\346\204\217\351\203\250\345\210\206\344\273\245\345\210\240\351\231\244\346\225\264\344\270\252\345\233\276\345\275\242\343\200\202</p></body></html>", nullptr));
#endif // QT_CONFIG(tooltip)
        actionDraggingStrokeEraser->setText(QCoreApplication::translate("MainWindow", "\346\213\226\345\212\250\347\254\224\347\224\273\346\251\241\347\232\256\346\223\246", nullptr));
#if QT_CONFIG(tooltip)
        actionDraggingStrokeEraser->setToolTip(QCoreApplication::translate("MainWindow", "<html><head/><body><p><span style=\" font-weight:700;\">\347\254\224\347\224\273\346\223\246\351\231\244\357\274\210\346\213\226\345\212\250\357\274\211</span></p><p>\346\214\211\344\275\217\345\271\266\346\213\226\345\212\250\357\274\214\351\274\240\346\240\207\345\210\222\350\277\207\347\232\204\346\211\200\346\234\211\345\233\276\345\275\242\351\203\275\345\260\206\350\242\253\345\210\240\351\231\244\343\200\202</p></body></html>", nullptr));
#endif // QT_CONFIG(tooltip)
        actionNormalEraser->setText(QCoreApplication::translate("MainWindow", "\346\231\256\351\200\232\346\251\241\347\232\256\346\223\246", nullptr));
#if QT_CONFIG(tooltip)
        actionNormalEraser->setToolTip(QCoreApplication::translate("MainWindow", "<html><head/><body><p><span style=\" font-weight:700;\">\346\251\241\347\232\256\346\223\246</span></p><p>\346\214\211\344\275\217\345\271\266\346\213\226\345\212\250\344\273\245\346\223\246\351\231\244\347\224\273\345\270\203\345\206\205\345\256\271\357\274\210\344\275\277\347\224\250\350\203\214\346\231\257\350\211\262\347\273\230\345\210\266\357\274\211\357\274\214\347\262\227\347\273\206\345\217\257\350\260\203\343\200\202</p></body></html>", nullptr));
#endif // QT_CONFIG(tooltip)
        actionChangeFillColor->setText(QCoreApplication::translate("MainWindow", "\345\241\253\345\205\205\351\242\234\350\211\262", nullptr));
#if QT_CONFIG(tooltip)
        actionChangeFillColor->setToolTip(QCoreApplication::translate("MainWindow", "<html><head/><body><p><span style=\" font-weight:700;\">\346\233\264\346\224\271\345\241\253\345\205\205\351\242\234\350\211\262</span></p><p>\347\202\271\345\207\273\351\200\211\346\213\251\345\220\216\347\273\255\347\273\230\345\210\266\345\233\276\345\275\242\347\232\204\345\241\253\345\205\205\351\242\234\350\211\262\357\274\210\345\257\271\351\227\255\345\220\210\345\233\276\345\275\242\346\234\211\346\225\210\357\274\211\343\200\202</p></body></html>", nullptr));
#endif // QT_CONFIG(tooltip)
        actionUndo->setText(QCoreApplication::translate("MainWindow", "\346\222\244\351\224\200", nullptr));
#if QT_CONFIG(tooltip)
        actionUndo->setToolTip(QCoreApplication::translate("MainWindow", "<html><head/><body><p><span style=\" font-weight:700;\">\346\222\244\351\224\200 (Ctrl+Z)</span></p><p>\345\217\226\346\266\210\344\270\212\344\270\200\346\255\245\346\223\215\344\275\234\343\200\202</p></body></html>", nullptr));
#endif // QT_CONFIG(tooltip)
#if QT_CONFIG(shortcut)
        actionUndo->setShortcut(QCoreApplication::translate("MainWindow", "Ctrl+Z", nullptr));
#endif // QT_CONFIG(shortcut)
        actionRedo->setText(QCoreApplication::translate("MainWindow", "\351\207\215\345\201\232", nullptr));
#if QT_CONFIG(tooltip)
        actionRedo->setToolTip(QCoreApplication::translate("MainWindow", "<html><head/><body><p><span style=\" font-weight:700;\">\351\207\215\345\201\232 (Ctrl+Shift+Z)</span></p><p>\346\201\242\345\244\215\344\270\212\344\270\200\346\255\245\345\267\262\346\222\244\351\224\200\347\232\204\346\223\215\344\275\234\343\200\202</p></body></html>", nullptr));
#endif // QT_CONFIG(tooltip)
#if QT_CONFIG(shortcut)
        actionRedo->setShortcut(QCoreApplication::translate("MainWindow", "Ctrl+Shift+Z", nullptr));
#endif // QT_CONFIG(shortcut)
        actionExportImage->setText(QCoreApplication::translate("MainWindow", "\345\217\246\345\255\230\344\270\272...", nullptr));
#if QT_CONFIG(tooltip)
        actionExportImage->setToolTip(QCoreApplication::translate("MainWindow", "<html><head/><body><p><span style=\" font-weight:700;\">\345\217\246\345\255\230\344\270\272... (Ctrl + S)</span></p><p>\345\260\206\345\275\223\345\211\215\347\224\273\345\270\203\345\206\205\345\256\271\344\277\235\345\255\230\344\270\272\345\233\276\345\203\217\346\226\207\344\273\266\343\200\202</p></body></html>", nullptr));
#endif // QT_CONFIG(tooltip)
#if QT_CONFIG(shortcut)
        actionExportImage->setShortcut(QCoreApplication::translate("MainWindow", "Ctrl+S", nullptr));
#endif // QT_CONFIG(shortcut)
        actionClearCanvas->setText(QCoreApplication::translate("MainWindow", "\346\270\205\347\251\272\347\224\273\345\270\203", nullptr));
#if QT_CONFIG(tooltip)
        actionClearCanvas->setToolTip(QCoreApplication::translate("MainWindow", "<html><head/><body><p><span style=\" font-weight:700;\">\346\270\205\347\251\272\347\224\273\345\270\203</span></p><p>\346\270\205\351\231\244\347\224\273\345\270\203\344\270\212\346\211\200\346\234\211\345\267\262\347\273\230\345\210\266\347\232\204\345\233\276\345\275\242\357\274\214\346\255\244\346\223\215\344\275\234\345\217\257\346\222\244\351\224\200\343\200\202</p></body></html>", nullptr));
#endif // QT_CONFIG(tooltip)
        actionOpenImage->setText(QCoreApplication::translate("MainWindow", "\346\211\223\345\274\200\345\233\276\347\211\207...", nullptr));
#if QT_CONFIG(tooltip)
        actionOpenImage->setToolTip(QCoreApplication::translate("MainWindow", "<html><head/><body><p><span style=\" font-weight:700;\">\346\211\223\345\274\200\345\233\276\347\211\207 (Ctrl + O)</span></p><p>\345\212\240\350\275\275\344\270\200\345\274\240\345\233\276\347\211\207\344\275\234\344\270\272\347\224\273\345\270\203\350\203\214\346\231\257\343\200\202</p></body></html>", nullptr));
#endif // QT_CONFIG(tooltip)
#if QT_CONFIG(shortcut)
        actionOpenImage->setShortcut(QCoreApplication::translate("MainWindow", "Ctrl+O", nullptr));
#endif // QT_CONFIG(shortcut)
        actionDrawEllipse->setText(QCoreApplication::translate("MainWindow", "\346\244\255\345\234\206", nullptr));
#if QT_CONFIG(tooltip)
        actionDrawEllipse->setToolTip(QCoreApplication::translate("MainWindow", "<html><head/><body><p><span style=\" font-weight:700;\">\347\273\230\345\210\266\346\244\255\345\234\206 (E)</span></p><p>\345\234\250\347\224\273\345\270\203\344\270\212\347\202\271\345\207\273\345\271\266\346\213\226\345\212\250\344\273\245\347\224\273\345\207\272\344\270\200\344\270\252\346\244\255\345\234\206\343\200\202</p></body></html>", nullptr));
#endif // QT_CONFIG(tooltip)
#if QT_CONFIG(shortcut)
        actionDrawEllipse->setShortcut(QCoreApplication::translate("MainWindow", "E", nullptr));
#endif // QT_CONFIG(shortcut)
        actionDrawStar->setText(QCoreApplication::translate("MainWindow", "\344\272\224\350\247\222\346\230\237", nullptr));
#if QT_CONFIG(tooltip)
        actionDrawStar->setToolTip(QCoreApplication::translate("MainWindow", "<html><head/><body><p><span style=\" font-weight:700;\">\347\273\230\345\210\266\344\272\224\350\247\222\346\230\237(S)</span></p><p>\345\234\250\347\224\273\345\270\203\344\270\212\347\202\271\345\207\273\345\271\266\346\213\226\345\212\250\344\273\245\347\224\273\345\207\272\344\270\200\344\270\252\344\272\224\350\247\222\346\230\237\343\200\202</p></body></html>", nullptr));
#endif // QT_CONFIG(tooltip)
#if QT_CONFIG(shortcut)
        actionDrawStar->setShortcut(QCoreApplication::translate("MainWindow", "S", nullptr));
#endif // QT_CONFIG(shortcut)
        labelCurrentPenWidth->setText(QCoreApplication::translate("MainWindow", "\347\272\277\345\256\275\357\274\2322", nullptr));
        ToolBar->setWindowTitle(QCoreApplication::translate("MainWindow", "toolBar_2", nullptr));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
