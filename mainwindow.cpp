#include "mainwindow.h"
#include "./ui_mainwindow.h" // 包含由 Qt Designer (uic) 生成的 UI 类定义
#include <QActionGroup>      // 用于创建互斥的动作组 (例如，绘图工具单选)
#include <QDebug>            // 用于调试输出
#include <QColorDialog>      // 用于弹出颜色选择对话框
#include <QToolBar>          // 用于在代码中添加或操作工具栏
#include <QFileDialog>       // 用于文件打开/保存对话框 (如导出图片、打开图片)
#include <QImage>            // 用于加载和处理图像数据 (如背景图片、导出)
#include <QPixmap>           // (QPixmap 也可用于图像操作，但 QImage 更适合底层像素操作)
#include <QMessageBox>       // 用于向用户显示提示或警告信息对话框
#include <QIcon>             // 用于我的程序的左上角的图标
#include <QPalette>

#include <QInputDialog>          // <--- 新增，用于弹出输入框
#include <QNetworkAccessManager> // <--- 新增，网络访问管理器
#include <QNetworkRequest>       // <--- 新增，用于构造网络请求
#include <QNetworkReply>         // <--- 新增，用于处理网络响应
#include <QJsonDocument>         // <--- 新增，处理JSON文档
#include <QJsonObject>         // <--- 新增，处理JSON对象
#include <QJsonArray>          // <--- 新增，处理JSON数组
#include "addmultipleshapescommand.h" // <--- 新增，引入我们之前创建的批量添加命令
#include "aipromptdialog.h"

#include "artboardview.h"    // 确保包含了 ArtboardView 的完整定义
#include "clearallcommand.h" // 包含了清空画布命令的定义
#include "abstractshape.h"
#include "groupshape.h"
#include "groupcommand.h"
#include "ungroupcommand.h"



// 如果其他命令类在 MainWindow 的槽函数中被直接创建 (虽然目前大部分是在 ArtboardView 中)，
// 也需要在这里包含它们的头文件。

/// @brief MainWindow 类的构造函数。
///
/// 负责以下主要的初始化工作：
/// 1. 调用 `ui->setupUi(this)` 来加载和初始化在 Qt Designer 中设计的UI元素。
///    这一步会自动创建 Designer 中定义的所有 QAction、QToolBar、菜单等，并建立
///    遵循 `on_<objectName>_<signalName>()` 命名约定的信号槽自动连接。
/// 2. 创建并设置 `ArtboardView` 实例作为主窗口的中央绘图区域。
/// 3. 设置窗口的基本属性，如标题和初始大小。
/// 4. 创建并配置 `drawingToolGroup` (QActionGroup)，用于管理所有绘图工具和橡皮擦工具
///    的 QAction，实现它们之间的互斥选择（单选效果）。
/// 5. （可选地，如果自动连接不可靠或命名不规范）显式连接某些 QAction 的信号到对应的槽函数。
///    （我们之前的版本已经清理了大部分不必要的显式 connect，依赖 MOC 自动连接）。
/// 6. 设置一个默认选中的工具（例如“选择”工具）。
/// 7. 将 `penOptionsContainer` (包含线宽滑块和标签的QWidget) 添加到一个工具栏上。
/// 8. 初始化线宽滑块的默认值和标签的显示文本。
/// 9. 连接 `ArtboardView` 发射的 `undoAvailabilityChanged` 和 `redoAvailabilityChanged` 信号
///    到 `MainWindow` 中用于更新撤销/重做按钮状态的槽函数。
/// 10. 显式设置撤销和重做按钮的初始禁用状态，确保程序启动时它们不可用。
///
/// @param parent 指向父 QWidget 对象的指针，对于主窗口通常为 nullptr。
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),      // 调用基类 QMainWindow 的构造函数
    ui(new Ui::MainWindow)  // 创建并初始化 Ui::MainWindow 对象 (管理 Designer UI 元素)
{
    // 1. 加载和设置由 Qt Designer 设计的UI。
    //    这行代码必须在构造函数的最前面，因为它会实例化 ui 指针所指向的对象，
    //    并创建所有在 .ui 文件中定义的控件和动作，例如 ui->actionDrawLine 等。
    //    同时，它也会尝试建立基于命名约定的信号槽自动连接。
    ui->setupUi(this);

    // 2. 创建 ArtboardView 实例，并将其设置为主窗口的中央部件。
    //    ArtboardView 是我们自定义的绘图区域。
    myArtboardView = new ArtboardView(this); // 'this' 使 MainWindow 成为 myArtboardView 的父对象，负责其内存管理
    setCentralWidget(myArtboardView);      // 将 myArtboardView 设置为中央区域显示

    // 3. 设置窗口的基本属性。
    setWindowTitle("Fishplate Artboard"); // 设置应用程序主窗口的标题
    resize(800, 600);                      // 设置主窗口的初始宽度和高度

    // 4. 初始化并配置绘图工具 QActionGroup。
    //    QActionGroup 用于将一组可勾选 (checkable) 的 QAction 组织起来，
    //    如果设置了 setExclusive(true)，则该组内的动作表现为单选按钮组。
    drawingToolGroup = new QActionGroup(this); // 'this' 使 MainWindow 成为 drawingToolGroup 的父对象
    drawingToolGroup->setExclusive(true);      // 设置为互斥模式




    // 设置窗口图标
    this->setWindowIcon(QIcon(":/icons/Fishplate_Artboard.png"));

    qDebug() << "MainWindow: Application window icon set.";





    // 5. 将在 Designer 中创建的、代表各种绘图和擦除工具的 QAction 添加到 drawingToolGroup 中。

    if (ui->actionSelectTool) {             // “选择”工具
        ui->actionSelectTool->setCheckable(true);
        drawingToolGroup->addAction(ui->actionSelectTool);
    }
    if (ui->actionDrawLine) {               // “直线”工具
        ui->actionDrawLine->setCheckable(true);
        drawingToolGroup->addAction(ui->actionDrawLine);
    }
    if (ui->actionDrawRectangle) {          // “矩形”工具
        ui->actionDrawRectangle->setCheckable(true);
        drawingToolGroup->addAction(ui->actionDrawRectangle);
    }
    if (ui->actionDrawFreehand) {           // “自由画笔”工具
        ui->actionDrawFreehand->setCheckable(true);
        drawingToolGroup->addAction(ui->actionDrawFreehand);
    }
    if (ui->actionDrawEllipse) {            // “椭圆”工具
        ui->actionDrawEllipse->setCheckable(true);
        drawingToolGroup->addAction(ui->actionDrawEllipse);
    }
    if (ui->actionDrawStar) {               // “五角星”工具
        ui->actionDrawStar->setCheckable(true);
        drawingToolGroup->addAction(ui->actionDrawStar);
    }
    // --- 橡皮擦工具也加入同一个互斥组 ---
    if (ui->actionNormalEraser) {           // “普通橡皮擦”工具
        ui->actionNormalEraser->setCheckable(true);
        drawingToolGroup->addAction(ui->actionNormalEraser);
    }
    if (ui->actionStrokeEraser) {           // “点击式笔画橡皮擦”工具
        ui->actionStrokeEraser->setCheckable(true);
        drawingToolGroup->addAction(ui->actionStrokeEraser);
    }
    if (ui->actionDraggingStrokeEraser) {   // “拖动式笔画橡皮擦”工具
        ui->actionDraggingStrokeEraser->setCheckable(true);
        drawingToolGroup->addAction(ui->actionDraggingStrokeEraser);
    }

    // 6. 设置一个默认选中的工具。
    //    当程序启动时，我们希望“选择”工具是默认激活的。
    if (ui->actionSelectTool) {
        ui->actionSelectTool->setChecked(true); // 将“选择”工具设为勾选状态
    }

    // 7. 处理画笔选项容器 (penOptionsContainer) 的放置。
    //    penOptionsContainer 是一个在 Designer 中创建的 QWidget，包含了线宽滑块和标签。
    if (ui->penOptionsContainer) { // 检查 Designer 中是否存在这个对象
        QToolBar *optionsToolBar = addToolBar(tr("画笔选项")); // 在主窗口添加一个新工具栏，并设置其标题
        optionsToolBar->setObjectName("optionsToolBar");      // 给这个新工具栏一个对象名 (方便代码引用或样式设置)
        optionsToolBar->addWidget(ui->penOptionsContainer);   // 将 penOptionsContainer (QWidget) 添加到这个工具栏中
    } else {
        qWarning() << "MainWindow Constructor: UI file might be missing 'penOptionsContainer'.";
    }

    // 8. 初始化线宽滑块 (sliderPenWidth) 的默认值和标签 (labelCurrentPenWidth) 的显示文本。
    //    确保 Designer 中滑块的 objectName="sliderPenWidth"，标签的 objectName="labelCurrentPenWidth"。
    if (ui->sliderPenWidth && ui->labelCurrentPenWidth && myArtboardView) {
        int initialPenWidth = myArtboardView->getCurrentPenWidth(); // 从 ArtboardView 获取初始线宽 (默认为2)
        ui->sliderPenWidth->setValue(initialPenWidth);              // 设置滑块的当前值为初始线宽
        // 使用滑块的实际值（可能受到min/max限制）来更新标签文本
        ui->labelCurrentPenWidth->setText(QString("线宽: %1").arg(ui->sliderPenWidth->value()));

        // qDebug() 输出用于确认初始化是否正确
        qDebug() << "Constructor: Initial pen width from ArtboardView:" << initialPenWidth;
        qDebug() << "Constructor: Slider value set to:" << ui->sliderPenWidth->value();
        qDebug() << "Constructor: Label text set to:" << ui->labelCurrentPenWidth->text();
        // sliderPenWidth 的 valueChanged(int) 信号会自动连接到 on_sliderPenWidth_valueChanged(int) 槽函数
    } else {
        qWarning() << "MainWindow Constructor: UI file might be missing 'sliderPenWidth' or 'labelCurrentPenWidth', or myArtboardView is null.";
    }

    // 9. 连接 ArtboardView 发射的信号到 MainWindow 的槽函数，用于动态更新撤销/重做按钮的启用状态。
    //     这些是跨对象的连接，必须手动进行。
    if (myArtboardView) {
        connect(myArtboardView, &ArtboardView::undoAvailabilityChanged,
                this, &MainWindow::updateUndoActionState);
        connect(myArtboardView, &ArtboardView::redoAvailabilityChanged,
                this, &MainWindow::updateRedoActionState);
    }

    // 10. 显式设置撤销 (Undo) 和重做 (Redo) QAction 按钮的初始禁用状态。
    //     虽然 ArtboardView 在构造时会发射信号将它们设为禁用，但在这里再次设置可以作为双保险，
    //     确保在界面完全显示前，它们就是禁用的。
    //     前提是这些 QAction (objectName: actionUndo, actionRedo) 在 Designer 中已创建。
    if (ui->actionUndo) {
        ui->actionUndo->setEnabled(false);
    }
    if (ui->actionRedo) {
        ui->actionRedo->setEnabled(false);
    }

    setupAdaptiveIcons();

}

/// @brief MainWindow 类的析构函数。
/// Qt 的父子对象机制会自动管理大部分在 MainWindow 中创建的、以 this 为父对象的 QObject 派生类实例的内存
/// (例如 myArtboardView, drawingToolGroup, 以及通过 addToolBar 创建的工具栏等)。
/// 因此，这里主要需要显式 delete 的是由 `new Ui::MainWindow` 创建的 ui 对象。
MainWindow::~MainWindow()
{
    delete ui;    // 释放由 Qt Designer 生成的 UI 类实例所占用的内存
    ui = nullptr; // 防止悬空指针

    // myArtboardView 由于在创建时指定了 this (MainWindow) 作为其父对象，
    // 当 MainWindow 被销毁时，Qt 的对象树机制会自动销毁 myArtboardView。
    // drawingToolGroup 同样如此。
    // 其他在 Designer 中创建的 QAction 等，也由 ui 对象管理或作为 MainWindow 的子对象管理。
    qDebug() << "MainWindow destroyed.";
}

/// @brief 响应“选择工具”QAction (ui->actionSelectTool) 被触发（点击并勾选）的槽函数。
/// 当用户选择此工具时，将 ArtboardView 的当前绘图模式设置为 ShapeType::None，
/// 表示不进行任何绘图操作，而是进入“选择”或“空闲”模式。
/// @note 此槽函数依赖 Qt 的 MOC 自动连接机制 (基于 on_<objectName>_<signalName>() 命名约定)。
///       前提是 ui->actionSelectTool 在 Designer 中的 objectName 为 "actionSelectTool"，
///       并且它是一个可勾选 (checkable) 的 QAction。
void MainWindow::on_actionSelectTool_triggered()
{
    // 确保动作存在并且是被勾选（激活）时才执行操作。
    // 对于在 QActionGroup(exclusive=true) 中的 checkable QAction，
    // triggered 信号发出时，该 action 必然是 isChecked() == true 的。
    if (ui->actionSelectTool && ui->actionSelectTool->isChecked()) {
        if (myArtboardView) { // 确保 ArtboardView 实例有效
            myArtboardView->setCurrentShape(ShapeType::None);
        }
        qDebug() << "MainWindow: SelectTool triggered. Current shape type set to None.";
    }
}

/// @brief 响应“直线工具”QAction (ui->actionDrawLine) 被触发的槽函数。
/// 将 ArtboardView 的当前绘图模式设置为 ShapeType::Line。
void MainWindow::on_actionDrawLine_triggered()
{
    if (ui->actionDrawLine && ui->actionDrawLine->isChecked()) {
        if (myArtboardView) {
            myArtboardView->setCurrentShape(ShapeType::Line);
        }
        qDebug() << "MainWindow: DrawLine triggered. Current shape type set to Line.";
    }
}

/// @brief 响应“矩形工具”QAction (ui->actionDrawRectangle) 被触发的槽函数。
/// 将 ArtboardView 的当前绘图模式设置为 ShapeType::Rectangle。
void MainWindow::on_actionDrawRectangle_triggered()
{
    if (ui->actionDrawRectangle && ui->actionDrawRectangle->isChecked()) {
        if (myArtboardView) {
            myArtboardView->setCurrentShape(ShapeType::Rectangle);
        }
        qDebug() << "MainWindow: DrawRectangle triggered. Current shape type set to Rectangle.";
    }
}

/// @brief 响应“自由画笔工具”QAction (ui->actionDrawFreehand) 被触发的槽函数。
/// 将 ArtboardView 的当前绘图模式设置为 ShapeType::Freehand。
void MainWindow::on_actionDrawFreehand_triggered()
{
    if (ui->actionDrawFreehand && ui->actionDrawFreehand->isChecked()) {
        if (myArtboardView) {
            myArtboardView->setCurrentShape(ShapeType::Freehand);
        }
        qDebug() << "MainWindow: DrawFreehand triggered. Current shape type set to Freehand.";
    }
}

/// @brief 响应“椭圆工具”QAction (ui->actionDrawEllipse) 被触发的槽函数。
/// 将 ArtboardView 的当前绘图模式设置为 ShapeType::Ellipse。
void MainWindow::on_actionDrawEllipse_triggered()
{
    if (ui->actionDrawEllipse && ui->actionDrawEllipse->isChecked()) {
        if (myArtboardView) {
            myArtboardView->setCurrentShape(ShapeType::Ellipse);
        }
        qDebug() << "MainWindow: DrawEllipse triggered. Current shape type set to Ellipse.";
    }
}

/// @brief 响应“五角星工具”QAction (ui->actionDrawStar) 被触发的槽函数。
/// 将 ArtboardView 的当前绘图模式设置为 ShapeType::Star。
void MainWindow::on_actionDrawStar_triggered()
{
    if (ui->actionDrawStar && ui->actionDrawStar->isChecked()) {
        if (myArtboardView) {
            myArtboardView->setCurrentShape(ShapeType::Star);
        }
        qDebug() << "MainWindow: DrawStar triggered. Current shape type set to Star.";
    }
}

/// @brief 响应“更改边框颜色”QAction (ui->actionChangeColor) 被触发的槽函数。
///
/// 当用户点击“颜色”按钮（通常用于设置边框颜色）时，此函数被调用。
/// 它会：
/// 1. 获取 ArtboardView 当前的边框颜色作为颜色对话框的初始选定颜色。
/// 2. 弹出一个 QColorDialog，让用户选择新的颜色。
/// 3. 如果用户确认选择了一个有效的颜色，则调用 ArtboardView 的 setCurrentDrawingColor() 方法
///    来更新画板当前的边框颜色。
/// @note 此槽函数依赖 Qt 的 MOC 自动连接机制。
///       前提是 ui->actionChangeColor 在 Designer 中的 objectName 为 "actionChangeColor"。
void MainWindow::on_actionChangeColor_triggered()
{
    qDebug() << "MainWindow::on_actionChangeColor_triggered - Slot called!";

    // 1. 确定颜色对话框的初始颜色。
    //    如果 myArtboardView 有效，则使用其当前的绘图边框颜色。否则使用默认的黑色。
    QColor initialBorderColor = Qt::black; // 默认初始颜色
    if (myArtboardView) {
        initialBorderColor = myArtboardView->getCurrentDrawingColor(); // 获取当前边框颜色
    }

    // 2. 弹出标准的颜色选择对话框。
    //    QColorDialog::getColor() 是一个静态辅助函数，方便使用。
    //    参数：初始颜色, 父窗口指针, 对话框标题。
    QColor selectedBorderColor = QColorDialog::getColor(initialBorderColor,
                                                        this,
                                                        tr("选择边框颜色"));

    qDebug() << "MainWindow::on_actionChangeColor_triggered - QColorDialog returned, isValid:" << selectedBorderColor.isValid();

    // 3. 检查用户是否选择了一个有效的颜色（即没有点击“取消”或关闭对话框）。
    if (selectedBorderColor.isValid()) {
        if (myArtboardView) { // 确保 ArtboardView 实例仍然有效
            // 调用 ArtboardView 的方法来设置新的当前边框颜色。
            myArtboardView->setCurrentDrawingColor(selectedBorderColor);
        }
        qDebug() << "MainWindow: Border color selected -" << selectedBorderColor.name();
        /// @note 可以在这里更新UI上某个地方（比如一个色块）来显示新选中的边框颜色，也是可优化的一点。
    }
}

/// @brief 响应“更改填充颜色”QAction (ui->actionChangeFillColor) 被触发的槽函数。
///
/// 当用户点击“填充颜色”按钮时，此函数被调用。
/// 其逻辑与 on_actionChangeColor_triggered() 非常相似，但它操作的是填充颜色：
/// 1. 获取 ArtboardView 当前的填充颜色作为颜色对话框的初始选定颜色。
/// 2. 弹出一个 QColorDialog，让用户选择新的填充颜色。
/// 3. 如果用户确认选择了一个有效的颜色，则调用 ArtboardView 的 setCurrentDrawingFillColor() 方法
///    来更新画板当前的填充颜色（ArtboardView 内部可能会根据颜色透明度更新是否填充的标志）。
/// @note 此槽函数依赖 Qt 的 MOC 自动连接机制。
void MainWindow::on_actionChangeFillColor_triggered()
{
    qDebug() << "MainWindow::on_actionChangeFillColor_triggered - Slot called!";

    // 1. 确定颜色对话框的初始填充颜色。
    QColor initialFillColor = Qt::transparent; // 默认初始填充颜色为透明
    if (myArtboardView) {
        initialFillColor = myArtboardView->getCurrentDrawingFillColor(); // 获取当前填充颜色
    }

    // 2. 弹出颜色选择对话框。
    QColor selectedFillColor = QColorDialog::getColor(initialFillColor,
                                                      this,
                                                      tr("选择填充颜色"));

    qDebug() << "MainWindow::on_actionChangeFillColor_triggered - QColorDialog returned, isValid:" << selectedFillColor.isValid();

    // 3. 检查用户是否选择了一个有效的颜色。
    if (selectedFillColor.isValid()) {
        if (myArtboardView) {
            // 调用 ArtboardView 的方法来设置新的当前填充颜色。
            myArtboardView->setCurrentDrawingFillColor(selectedFillColor);
        }
        qDebug() << "MainWindow: Fill color selected -" << selectedFillColor.name();
        /// @note 可以在这里更新UI上某个地方（比如一个色块）来显示新选中的边框颜色，也是可优化的一点。
    }
}

/// @brief 响应线宽滑块 QSlider (ui->sliderPenWidth) 的 valueChanged(int) 信号的槽函数。
///
/// 当用户拖动线宽滑块，使其值发生改变时，此函数被调用。
/// 它会：
/// 1. 调用 ArtboardView 的 setCurrentPenWidth() 方法，将新的线宽值传递给画板。
/// 2. 更新界面上显示当前线宽值的标签 (ui->labelCurrentPenWidth)。
/// @param value 滑块当前的新整数值，代表新的线宽。
/// @note 此槽函数依赖 Qt 的 MOC 自动连接机制。
///       前提是 ui->sliderPenWidth 在 Designer 中的 objectName 为 "sliderPenWidth"。
void MainWindow::on_sliderPenWidth_valueChanged(int value)
{
    qDebug() << "MainWindow::on_sliderPenWidth_valueChanged - Received value:" << value;

    // 1. 将新的线宽值设置到 ArtboardView 中。
    if (myArtboardView) {
        myArtboardView->setCurrentPenWidth(value);
    }

    // 2. 更新界面上显示线宽数值的 QLabel。
    //    确保 Designer 中该 QLabel 的 objectName 是 "labelCurrentPenWidth"。
    if (ui->labelCurrentPenWidth) {
        ui->labelCurrentPenWidth->setText(QString("线宽: %1").arg(value));
    }
}

/// @brief 响应“点击式笔画橡皮擦”QAction (ui->actionStrokeEraser) 被触发的槽函数。
///
/// 当用户从工具栏或菜单选择“点击式笔画橡皮擦”工具时，此函数被调用。
/// 它会将 ArtboardView 的当前绘图/操作模式设置为 ShapeType::StrokeEraser，
/// 使得后续在 ArtboardView 中的鼠标点击操作会被解释为尝试删除整个图形笔画。
///
/// @note 此槽函数依赖 Qt 的 MOC 自动连接机制。
///       前提是 ui->actionStrokeEraser 在 Designer 中的 objectName 为 "actionStrokeEraser"，
///       并且它是一个可勾选 (checkable) 的 QAction，且已加入到互斥的 drawingToolGroup 中。
void MainWindow::on_actionStrokeEraser_triggered()
{
    // 确保 QAction 指针有效，并且该动作当前是被勾选（激活）的状态。
    // 对于在互斥 QActionGroup 中的动作，当 triggered 信号发出时，发送者通常就是被勾选的那个。
    if (ui->actionStrokeEraser && ui->actionStrokeEraser->isChecked()) {
        if (myArtboardView) { // 确保 ArtboardView 实例有效
            // 调用 ArtboardView 的方法，将其当前操作模式设置为点击式笔画橡皮擦。
            myArtboardView->setCurrentShape(ShapeType::StrokeEraser);
        }
        qDebug() << "MainWindow: StrokeEraser tool triggered. ArtboardView mode set to StrokeEraser.";
    }
}

/// @brief 响应“拖动式笔画橡皮擦”QAction (ui->actionDraggingStrokeEraser) 被触发的槽函数。
///
/// 当用户选择“拖动式笔画橡皮擦”工具时，此函数被调用。
/// 它会将 ArtboardView 的当前绘图/操作模式设置为 ShapeType::DraggingStrokeEraser，
/// 使得后续在 ArtboardView 中的鼠标拖动操作会被解释为尝试删除路径上所有碰触到的图形笔画。
///
/// @note 依赖 MOC 自动连接。ui->actionDraggingStrokeEraser 的 objectName 应为 "actionDraggingStrokeEraser"，
///       且为 checkable 并已加入 drawingToolGroup。
void MainWindow::on_actionDraggingStrokeEraser_triggered()
{
    if (ui->actionDraggingStrokeEraser && ui->actionDraggingStrokeEraser->isChecked()) {
        if (myArtboardView) {
            myArtboardView->setCurrentShape(ShapeType::DraggingStrokeEraser);
        }
        qDebug() << "MainWindow: DraggingStrokeEraser tool triggered. ArtboardView mode set to DraggingStrokeEraser.";
    }
}

/// @brief 响应“普通橡皮擦”QAction (ui->actionNormalEraser) 被触发的槽函数。
///
/// 当用户选择“普通橡皮擦”工具时，此函数被调用。
/// 它会将 ArtboardView 的当前绘图/操作模式设置为 ShapeType::NormalEraser，
/// 使得后续在 ArtboardView 中的鼠标拖动操作会被解释为使用背景色绘制路径，从而实现擦除效果。
/// 橡皮擦的粗细由当前的线宽设置 (currentPenWidth) 控制。
///
/// @note 依赖 MOC 自动连接。ui->actionNormalEraser 的 objectName 应为 "actionNormalEraser"，
///       且为 checkable 并已加入 drawingToolGroup。
void MainWindow::on_actionNormalEraser_triggered()
{
    if (ui->actionNormalEraser && ui->actionNormalEraser->isChecked()) {
        if (myArtboardView) {
            myArtboardView->setCurrentShape(ShapeType::NormalEraser);
        }
        qDebug() << "MainWindow: NormalEraser tool triggered. ArtboardView mode set to NormalEraser.";
        // 关于橡皮擦颜色和粗细的说明性注释：
        // - 颜色：ArtboardView 在创建 EraserPathShape 对象时，会使用画布的背景色
        //   (例如 palette().window().color()) 作为橡皮擦的“绘制”颜色。
        //   因此，MainWindow 在这里不需要特别传递颜色信息。
        // - 粗细：ArtboardView 在创建 EraserPathShape 对象时，会使用当前的
        //   currentPenWidth 作为橡皮擦的粗细。用户通过线宽滑块调整的值会自动生效。
    }
}

/// @brief 响应“撤销”QAction (ui->actionUndo) 被触发的槽函数。
/// 当用户点击“撤销”按钮或使用快捷键 (如 Ctrl+Z) 时，此函数被调用。
/// 它会调用 ArtboardView 的 undo() 方法来执行实际的撤销操作。
/// @note 此槽函数依赖 Qt 的 MOC 自动连接机制。
///       前提是 ui->actionUndo 在 Designer 中的 objectName 为 "actionUndo"。
void MainWindow::on_actionUndo_triggered()
{
    if (myArtboardView) { // 确保 ArtboardView 实例有效
        myArtboardView->undo(); // 调用 ArtboardView 的撤销方法
        qDebug() << "MainWindow: Undo action triggered.";
        // ArtboardView::undo() 内部会调用 updateUndoRedoStatus()，
        // 后者发射信号，由 MainWindow::updateUndoActionState() 和 updateRedoActionState()
        // 槽函数接收并更新撤销/重做按钮的启用状态。
    }
}

/// @brief 响应“重做”QAction (ui->actionRedo) 被触发的槽函数。
/// 当用户点击“重做”按钮或使用快捷键 (如 Ctrl+Y) 时，此函数被调用。
/// 它会调用 ArtboardView 的 redo() 方法来执行实际的重做操作。
/// @note 此槽函数依赖 Qt 的 MOC 自动连接机制。
void MainWindow::on_actionRedo_triggered()
{
    if (myArtboardView) { // 确保 ArtboardView 实例有效
        myArtboardView->redo(); // 调用 ArtboardView 的重做方法
        qDebug() << "MainWindow: Redo action triggered.";
        // ArtboardView::redo() 内部会调用 updateUndoRedoStatus() 来更新按钮状态。
    }
}

/// @brief 响应“清空画布”QAction (ui->actionClearCanvas) 被触发的槽函数。
///
/// 当用户点击“清空画布”按钮时，此函数被调用。它会：
/// 1. 创建一个 ClearAllCommand 对象，该命令封装了清空画布的操作。
/// 2. 调用 ArtboardView 的 executeCommand() 方法来执行此命令，
///    该方法会将命令压入撤销栈并清空重做栈，同时更新UI。
/// @note 此槽函数依赖 Qt 的 MOC 自动连接机制。
void MainWindow::on_actionClearCanvas_triggered()
{
    if (myArtboardView) { // 确保 ArtboardView 实例有效
        qDebug() << "MainWindow: ClearCanvas action triggered.";
        // 1. 创建一个 ClearAllCommand 实例，将 myArtboardView 作为参数传递，
        //    以便命令可以操作 ArtboardView 的图形列表和命令栈。
        ClearAllCommand *clearCmd = new ClearAllCommand(myArtboardView);
        // 2. 通过 ArtboardView 的 executeCommand 方法执行此命令。
        //    executeCommand 会负责调用 clearCmd->execute()，
        //    将 clearCmd 压入撤销栈，并清空重做栈，同时更新撤销/重做按钮状态。
        myArtboardView->executeCommand(clearCmd);
    }
}

void MainWindow::on_actionOpen_triggered()
{
    // 定义文件类型过滤器，让用户可以选择
    const QString filter = tr("所有支持的文件 (*.fpa *.png *.jpg *.jpeg *.bmp);;Fishplate工程文件 (*.fpa);;图片文件 (*.png *.jpg *.jpeg *.bmp)");

    // 弹出文件打开对话框
    QString filePath = QFileDialog::getOpenFileName(
        this,
        tr("打开文件"),
        "",
        filter
        );

    if (filePath.isEmpty()) {
        return; // 用户点击了取消
    }

    // [ 关键修正 ]
    // 不再检查 selectedFilter，而是直接检查文件名的后缀
    if (filePath.endsWith(".fpa", Qt::CaseInsensitive)) { // Qt::CaseInsensitive 表示不区分大小写
        // 如果选择的是工程文件，则调用加载数据库的函数
        if (myArtboardView->loadFromDatabase(filePath)) { //
            QMessageBox::information(this, tr("加载成功"), tr("已成功加载工程。"));
        } else {
            QMessageBox::critical(this, tr("加载失败"), tr("无法从指定文件加载工程。"));
        }
    }
    else // 否则，我们认为用户选择的是图片文件
    {
        QImage backgroundImageToLoad;
        if (backgroundImageToLoad.load(filePath)) {
            // 如果是图片文件，则调用设置背景图的函数
            myArtboardView->setBackgroundImage(backgroundImageToLoad); //
        } else {
            // 如果加载图片也失败了（比如文件损坏），才报这个错
            QMessageBox::warning(this, tr("打开图片失败"), tr("无法加载选定的图像文件。"));
        }
    }
}


void MainWindow::on_actionSaveAs_triggered()
{
    // 定义文件类型过滤器
    const QString filter = tr("Fishplate工程文件 (*.fpa);;PNG图片 (*.png);;JPEG图片 (*.jpg)");

    // 弹出文件保存对话框
    QString filePath = QFileDialog::getSaveFileName(
        this,
        tr("另存为"),
        "",
        filter
        );

    if (filePath.isEmpty()) {
        return; // 用户点击了取消
    }

    // [ 关键修正 ]
    // 同样，我们直接检查最终文件名的后缀
    if (filePath.endsWith(".fpa", Qt::CaseInsensitive)) {
        // 调用保存到数据库的函数
        if (!myArtboardView->saveToDatabase(filePath)) { //
            QMessageBox::critical(this, tr("保存失败"), tr("无法将工程保存到指定文件。"));
        } else {
            QMessageBox::information(this, tr("保存成功"), tr("工程已成功保存。"));
        }
    }
    else // 否则，认为是保存为图片
    {
        // 调用渲染到图片的函数
        QImage imageToSave = myArtboardView->renderToImage(); //
        if (imageToSave.save(filePath)) {
            QMessageBox::information(this, tr("导出成功"), tr("图像已成功导出。"));
        } else {
            QMessageBox::critical(this, tr("导出失败"), tr("无法将图像保存到指定文件。"));
        }
    }
}

/// @brief 响应来自 ArtboardView 的 undoAvailabilityChanged(bool) 信号的槽函数。
///
/// 当 ArtboardView 中的撤销栈状态发生改变（例如，从空变为非空，或从非空变为空）时，
/// ArtboardView 会发射 undoAvailabilityChanged 信号，此槽函数随之被调用。
/// 它根据传入的 `available` 参数来设置“撤销”QAction (ui->actionUndo) 的启用或禁用状态。
///
/// @param available 布尔值，如果为 true，则表示当前有操作可以撤销，撤销按钮应被启用；
///                  如果为 false，则表示撤销栈为空，撤销按钮应被禁用。
/// @note 此槽函数与 ArtboardView 的信号的连接是在 MainWindow 构造函数中手动建立的。
void MainWindow::updateUndoActionState(bool available)
{
    if (ui->actionUndo) { // 确保 Designer 中的 ui->actionUndo 指针有效
        ui->actionUndo->setEnabled(available); // 设置 QAction 的启用/禁用状态
        qDebug() << "MainWindow: Undo action 'enabled' state set to:" << available;
    } else {
        qWarning("MainWindow::updateUndoActionState - ui->actionUndo is null!");
    }
}

/// @brief 响应来自 ArtboardView 的 redoAvailabilityChanged(bool) 信号的槽函数。
///
/// 当 ArtboardView 中的重做栈状态发生改变时，ArtboardView 会发射 redoAvailabilityChanged 信号，
/// 此槽函数随之被调用。它根据传入的 `available` 参数来设置“重做”QAction (ui->actionRedo)
/// 的启用或禁用状态。
///
/// @param available 布尔值，如果为 true，则表示当前有操作可以重做，重做按钮应被启用；
///                  如果为 false，则表示重做栈为空，重做按钮应被禁用。
/// @note 此槽函数与 ArtboardView 的信号的连接是在 MainWindow 构造函数中手动建立的。
void MainWindow::updateRedoActionState(bool available)
{
    if (ui->actionRedo) { // 确保 Designer 中的 ui->actionRedo 指针有效
        ui->actionRedo->setEnabled(available); // 设置 QAction 的启用/禁用状态
        qDebug() << "MainWindow: Redo action 'enabled' state set to:" << available;
    } else {
        qWarning("MainWindow::updateRedoActionState - ui->actionRedo is null!");
    }
}

void MainWindow::on_actionGroup_triggered()
{
    if (!myArtboardView) return;

    const QList<AbstractShape*>& selectedShapes = myArtboardView->getSelectedShapes();

    // 只有当选中了多个图形时，编组才有意义
    if (selectedShapes.count() >= 2) {
        GroupCommand *command = new GroupCommand(selectedShapes, myArtboardView);
        myArtboardView->executeCommand(command);
    } else {
        qDebug() << "Grouping requires at least two selected shapes.";
    }
}

void MainWindow::on_actionUngroup_triggered()
{
    if (!myArtboardView) return;

    const QList<AbstractShape*>& selectedShapes = myArtboardView->getSelectedShapes();

    // 只有当恰好选中一个图形，并且这个图形是一个组时，取消编组才有意义
    if (selectedShapes.count() == 1) {
        // 使用 dynamic_cast 来安全地检查选中的图形是否是 GroupShape 类型
        GroupShape *selectedGroup = dynamic_cast<GroupShape*>(selectedShapes.first());
        if (selectedGroup) {
            UngroupCommand *command = new UngroupCommand(selectedGroup, myArtboardView);
            myArtboardView->executeCommand(command);
        } else {
            qDebug() << "Ungrouping requires a single selected group.";
        }
    } else {
        qDebug() << "Ungrouping requires exactly one selected group.";
    }
}


void MainWindow::setupAdaptiveIcons()
{
    // 1. 判断当前系统主题是深色还是浅色
    // 我们通过检查窗口背景色的亮度来判断。亮度值小于128通常意味着是深色背景。
    bool isDarkMode = this->palette().color(QPalette::Window).lightness() < 128;

    // 2. 根据主题选择对应的图标文件后缀
    QString themeSuffix = isDarkMode ? "_light" : "_dark";

    // 3. 为你所有的QAction设置正确的图标
    //    重要提示：下面的“:/icons/”是图标在Qt资源文件(.qrc)中的路径前缀。
    //    请确保这个前缀和你自己在.qrc文件中设置的一致。如果不一致，请修改它。
    QString iconPrefix = ":/icons/icons/";

    // --- 文件与编辑操作 ---
    if (ui->actionUndo) ui->actionUndo->setIcon(QIcon(iconPrefix + "undo" + themeSuffix + ".svg"));
    if (ui->actionRedo) ui->actionRedo->setIcon(QIcon(iconPrefix + "redo" + themeSuffix + ".svg"));
    if (ui->actionClearCanvas) ui->actionClearCanvas->setIcon(QIcon(iconPrefix + "clear" + themeSuffix + ".svg"));
    if (ui->actionOpen) ui->actionOpen->setIcon(QIcon(iconPrefix + "file_open" + themeSuffix + ".svg"));
    if (ui->actionSaveAs) ui->actionSaveAs->setIcon(QIcon(iconPrefix + "save" + themeSuffix + ".svg"));

    // --- 绘图工具 ---
    if (ui->actionSelectTool) ui->actionSelectTool->setIcon(QIcon(iconPrefix + "arrow_selector" + themeSuffix + ".svg"));
    if (ui->actionDrawLine) ui->actionDrawLine->setIcon(QIcon(iconPrefix + "line" + themeSuffix + ".svg"));
    if (ui->actionDrawRectangle) ui->actionDrawRectangle->setIcon(QIcon(iconPrefix + "square" + themeSuffix + ".svg"));
    if (ui->actionDrawEllipse) ui->actionDrawEllipse->setIcon(QIcon(iconPrefix + "ellipse" + themeSuffix + ".svg"));
    if (ui->actionDrawStar) ui->actionDrawStar->setIcon(QIcon(iconPrefix + "star" + themeSuffix + ".svg"));
    if (ui->actionDrawFreehand) ui->actionDrawFreehand->setIcon(QIcon(iconPrefix + "freehand_pen" + themeSuffix + ".svg"));

    // --- 属性与橡皮擦工具 ---
    if (ui->actionChangeColor) ui->actionChangeColor->setIcon(QIcon(iconPrefix + "color" + themeSuffix + ".svg"));
    // (假设actionChangeFillColor的图标文件是 inner_color)
    if (ui->actionChangeFillColor) ui->actionChangeFillColor->setIcon(QIcon(iconPrefix + "inner_color" + themeSuffix + ".svg"));
    if (ui->actionNormalEraser) ui->actionNormalEraser->setIcon(QIcon(iconPrefix + "normal_eraser" + themeSuffix + ".svg"));
    if (ui->actionStrokeEraser) ui->actionStrokeEraser->setIcon(QIcon(iconPrefix + "clicking_eraser" + themeSuffix + ".svg"));
    if (ui->actionDraggingStrokeEraser) ui->actionDraggingStrokeEraser->setIcon(QIcon(iconPrefix + "dragging_eraser" + themeSuffix + ".svg"));
    if (ui->actionAiDraw) ui->actionAiDraw->setIcon(QIcon(iconPrefix + "ai_draw" + themeSuffix + ".svg"));
    if (ui->actionGroup) ui->actionGroup->setIcon(QIcon(iconPrefix + "group" + themeSuffix + ".svg"));
    if (ui->actionUngroup) ui->actionUngroup->setIcon(QIcon(iconPrefix + "ungroup" + themeSuffix + ".svg"));
}

// ----------------- mainwindow.cpp (请完整替换此函数) -----------------
void MainWindow::on_actionAiDraw_triggered()
{
    // 1. 创建并显示我们自定义的新对话框
    AiPromptDialog dialog(this);
    // dialog.exec() 会显示对话框，并在这里暂停，直到用户点击OK或Cancel
    if (dialog.exec() == QDialog::Accepted) {
        // 2. 从对话框获取所有用户输入的数据
        QString prompt = dialog.getPromptText();
        QString selectedModel = dialog.getSelectedModelName();
        QString apiKey = dialog.getApiKey();

        // 3. 校验用户输入
        if (prompt.isEmpty() || apiKey.isEmpty()) {
            QMessageBox::warning(this, "输入错误", "绘画指令和API密钥均不能为空。");
            return;
        }

        // 4. 根据用户选择，设置不同的API地址和模型名称
        QString apiUrl;
        QString modelName;

        if (selectedModel == "DeepSeek-V3-0324") {
            apiUrl = "https://api.deepseek.com/v1/chat/completions";
            modelName = "deepseek-chat";
        }
        else if (selectedModel == "DeepSeek-R1-0528") { // <-- 新增的模型选项
            apiUrl = "https://api.deepseek.com/v1/chat/completions";
            modelName = "deepseek-reasoner"; // <-- 使用你指定的模型名称
        }
        else if (selectedModel == "OpenAI (GPT-3.5)") {
            apiUrl = "https://api.openai.com/v1/chat/completions";
            modelName = "gpt-3.5-turbo";
        }
        else if (selectedModel == "OpenAI (GPT-4 Turbo)") { // <-- 新增的模型选项
            apiUrl = "https://api.openai.com/v1/chat/completions";
            modelName = "gpt-4-turbo";
        }
        else {
            QMessageBox::warning(this, "错误", "未知的AI模型选择: " + selectedModel);
            return;
        }

        // --- 开始进行网络请求 ---
        QNetworkAccessManager *manager = new QNetworkAccessManager(this);

        // 5. 构造发送给API的JSON请求体
        QJsonObject requestData;
        requestData["model"] = modelName;

        QJsonArray messages;
        // ----------------- mainwindow.cpp (请在 on_actionAiDraw_triggered 函数中替换这部分) -----------------
        QJsonObject systemMessage;
        systemMessage["role"] = "system";
        systemMessage["content"] = R"(You are an expert vector graphic designer assistant for a drawing application.
Your task is to convert the user's request into a structured JSON format that the application can render.

**Rules:**
1. The JSON response MUST contain a single root key 'shapes', which is an array of shape objects.
2. The canvas size is 800x600 pixels. All coordinates MUST be within the [0, 800] range for x and [0, 600] for y.
3. Each shape object must have a 'type' property. Supported types are 'Rectangle', 'Ellipse', 'Star', 'Line', 'Freehand'.
4. **A new container type 'Group' is now supported.** A 'Group' object does not have geometry or color properties. Instead, it MUST have a 'children' property, which is an array of other shape objects (which can even be other groups).
5. For 'Rectangle', 'Ellipse', or 'Star', geometry must contain 'x', 'y', 'width', 'height'.
6. For 'Line', geometry must contain 'p1' and 'p2' as arrays of two numbers.
7. For 'Freehand', geometry must contain a 'points' array, which is an array of [x,y] point arrays.
8. Optional properties are 'pen_width', 'border_color', 'rotation', 'is_filled', 'fill_color'.
9. Decompose complex objects into simpler shapes.
10. ONLY respond with the raw JSON object. Do not include any extra text, explanations, or markdown fences like ```json.

**Example of a Group:**
To draw a face, you could group the head and eyes:
{
  "shapes": [
    {
      "type": "Group",
      "children": [
        { "type": "Ellipse", "geometry": {"x":100, "y":100, "width":200, "height":200}, "fill_color": "#FFFF00" },
        { "type": "Ellipse", "geometry": {"x":150, "y":160, "width":20, "height":20}, "fill_color": "#000000" },
        { "type": "Ellipse", "geometry": {"x":230, "y":160, "width":20, "height":20}, "fill_color": "#000000" }
      ]
    }
  ]
}

Now, convert the user's following request.
)";

        QJsonObject userMessage;
        userMessage["role"] = "user";
        userMessage["content"] = prompt;
        messages.append(systemMessage);
        messages.append(userMessage);
        requestData["messages"] = messages;

        // 6. 设置API请求的地址和头部信息
        QNetworkRequest request{QUrl(apiUrl)};
        request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
        // 使用从对话框中获取的apiKey
        request.setRawHeader("Authorization", ("Bearer " + apiKey).toUtf8());

        // 7. 发送POST请求，并设置回调函数来异步处理返回结果
        QNetworkReply *reply = manager->post(request, QJsonDocument(requestData).toJson());

        QMessageBox* msgBox = new QMessageBox(QMessageBox::Information, "AI正在创作中", "请稍候...", QMessageBox::NoButton, this);
        msgBox->setStandardButtons(QMessageBox::NoButton);
        msgBox->show();

        connect(reply, &QNetworkReply::finished, this, [=]() {
            msgBox->close();
            delete msgBox;

            if (reply->error() == QNetworkReply::NoError) {
                // 8. 解析返回的JSON数据
                QByteArray responseData = reply->readAll();
                QJsonObject jsonResponse = QJsonDocument::fromJson(responseData).object();
                QString aiContent = jsonResponse["choices"].toArray()[0].toObject()["message"].toObject()["content"].toString();

                // (数据清洗逻辑，以防万一AI还是返回了Markdown)
                int firstBrace = aiContent.indexOf('{');
                int lastBrace = aiContent.lastIndexOf('}');
                if (firstBrace != -1 && lastBrace != -1) {
                    aiContent = aiContent.mid(firstBrace, lastBrace - firstBrace + 1);
                }

                QJsonObject shapeData = QJsonDocument::fromJson(aiContent.toUtf8()).object();
                QJsonArray shapesArray = shapeData["shapes"].toArray();

                QList<AbstractShape*> newShapes;
                for (const QJsonValue &val : shapesArray) {
                    AbstractShape* shape = AbstractShape::fromJsonObject(val.toObject());
                    if (shape) {
                        newShapes.append(shape);
                    }
                }

                if (!newShapes.isEmpty()) {
                    myArtboardView->executeCommand(new AddMultipleShapesCommand(newShapes, myArtboardView));
                }
            } else {
                QString errorString = reply->errorString();
                QMessageBox::critical(this, "AI创作失败", "请求失败，请检查网络或API Key。\n错误信息: " + errorString);
            }

            reply->deleteLater();
            manager->deleteLater();
        });
    }
}
