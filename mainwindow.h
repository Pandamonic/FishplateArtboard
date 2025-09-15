#ifndef MAINWINDOW_H
#define MAINWINDOW_H

// ---------------------------------------------------------------------------
// 描述: 定义 MainWindow 类，这是应用程序的主窗口。
//       它包含了菜单栏、工具栏、状态栏以及主要的绘图区域 (ArtboardView)。
//       负责响应用户的界面操作（如点击工具按钮、菜单项），并将这些操作
//       转换为对 ArtboardView 的调用，同时也接收来自 ArtboardView 的信号以更新UI状态。
// ---------------------------------------------------------------------------

#include <QMainWindow> // QMainWindow 是所有主窗口类的基类
#include "artboardview.h" // MainWindow 会包含一个 ArtboardView 实例

// 向前声明 QActionGroup，因为我们只在成员变量中使用了它的指针类型，
// 这样可以避免在头文件中包含 <QActionGroup> 的完整定义，减少编译依赖。
class QActionGroup;

// Qt Designer 生成的 UI 类通常放在一个命名空间 Ui 中
QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; } // 向前声明 Designer 生成的 Ui::MainWindow 类
QT_END_NAMESPACE

/// @brief MainWindow 类是 Fishplate Artboard 应用程序的主窗口界面。
///
/// 它负责：
/// - 构建和管理主窗口的整体布局，包括菜单栏、多个工具栏、状态栏以及中央的绘图视图 (ArtboardView)。
/// - 初始化和管理所有的用户界面动作 (QAction)，如选择绘图工具、设置颜色、执行撤销/重做、导出图片等。
/// - 将这些 QAction 分配到相应的菜单项和工具栏按钮上。
/// - 使用 QActionGroup 来确保绘图工具之间的互斥选择（单选效果）。
/// - 实现槽函数，以响应用户与这些 QAction 和其他控件（如线宽滑块）的交互。
/// - 将用户的操作意图（如选择的工具、颜色、线宽）传递给 ArtboardView 实例。
/// - 连接 ArtboardView 发射的信号（如撤销/重做可用性改变），以相应地更新自身UI元素（如按钮的启用/禁用状态）。
class MainWindow : public QMainWindow
{
    Q_OBJECT // Q_OBJECT 宏是所有需要使用 Qt 信号槽机制和元对象特性的 QObject 子类所必需的

public:
    /// @brief MainWindow 类的构造函数。
    /// @param parent 父 QWidget 对象，对于主窗口为 nullptr。
    explicit MainWindow(QWidget *parent = nullptr); // 互斥 explicit 防止隐式转换

    /// @brief MainWindow 类的析构函数。
    /// 主要负责释放由 Qt Designer 生成的 ui 对象的内存。
    ~MainWindow();

    // 槽函数通常声明为 private slots 或 public slots。
    // private slots 意味着它们主要由内部信号（如此类自身的UI控件发出的信号）或其他友元类调用。
private slots:
    // --- 绘图工具选择相关的槽函数 ---
    /// @brief 响应“选择工具”动作 (actionSelectTool) 被触发。
    void on_actionSelectTool_triggered();
    /// @brief 响应“直线工具”动作 (actionDrawLine) 被触发。
    void on_actionDrawLine_triggered();
    /// @brief 响应“矩形工具”动作 (actionDrawRectangle) 被触发。
    void on_actionDrawRectangle_triggered();
    /// @brief 响应“自由画笔工具”动作 (actionDrawFreehand) 被触发。
    void on_actionDrawFreehand_triggered();
    /// @brief 响应“椭圆工具”动作 (actionDrawEllipse) 被触发。
    void on_actionDrawEllipse_triggered();
    /// @brief 响应“五角星工具”动作 (actionDrawStar) 被触发。
    void on_actionDrawStar_triggered();

    // --- 橡皮擦工具相关的槽函数 ---
    /// @brief 响应“普通橡皮擦”动作 (actionNormalEraser) 被触发。
    void on_actionNormalEraser_triggered();
    /// @brief 响应“点击式笔画橡皮擦”动作 (actionStrokeEraser) 被触发。
    void on_actionStrokeEraser_triggered();
    /// @brief 响应“拖动式笔画橡皮擦”动作 (actionDraggingStrokeEraser) 被触发。
    void on_actionDraggingStrokeEraser_triggered();

    // --- 绘图属性设置相关的槽函数 ---
    /// @brief 响应“更改边框颜色”动作 (actionChangeColor) 被触发。弹出颜色选择对话框。
    void on_actionChangeColor_triggered();
    /// @brief 响应“更改填充颜色”动作 (actionChangeFillColor) 被触发。弹出颜色选择对话框。
    void on_actionChangeFillColor_triggered();
    /// @brief 响应线宽滑块 (sliderPenWidth) 的值改变信号。
    /// @param value 滑块当前的新值，代表线宽。
    void on_sliderPenWidth_valueChanged(int value);

    // --- 编辑与文件操作相关的槽函数 ---
    /// @brief 响应“撤销”动作 (actionUndo) 被触发。
    void on_actionUndo_triggered();
    /// @brief 响应“重做”动作 (actionRedo) 被触发。
    void on_actionRedo_triggered();
    /// @brief 响应“清空画布”动作 (actionClearCanvas) 被触发。
    void on_actionClearCanvas_triggered();
    /// @brief 响应“导出图片”动作 (actionExportImage) 被触发。




    void on_actionOpen_triggered();    // 对应新的 actionOpen

    void on_actionSaveAs_triggered();  // 对应新的 actionSaveAs

    void on_actionGroup_triggered();

    void on_actionUngroup_triggered();

    void on_actionAiDraw_triggered();
    // --- 更新UI状态的槽函数 (响应来自 ArtboardView 的信号) ---
    /// @brief 更新“撤销”按钮的启用/禁用状态。
    /// @param available 如果为 true，则启用撤销按钮；否则禁用。
    void updateUndoActionState(bool available);
    /// @brief 更新“重做”按钮的启用/禁用状态。
    /// @param available 如果为 true，则启用重做按钮；否则禁用。
    void updateRedoActionState(bool available);

private:
    Ui::MainWindow *ui; ///< 指向由 Qt Designer 自动生成的 UI 类实例的指针。
        ///< 通过 ui 指针可以访问在 Designer 中创建的所有界面元素。

    ArtboardView *myArtboardView; ///< 指向我们自定义的绘图视图 (画布) 对象的指针。
        ///< MainWindow 将用户的绘图相关操作委派给此对象处理。

    QActionGroup *drawingToolGroup; ///< 用于管理所有绘图工具 QAction 的动作组。
        ///< 设置为互斥 (exclusive)，以确保一次只能选择一个绘图工具。

    void setupAdaptiveIcons();
};

#endif // MAINWINDOW_H
