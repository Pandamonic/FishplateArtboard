// ----------------- aipromptdialog.cpp (修正后) -----------------
#include "aipromptdialog.h"
#include "ui_aipromptdialog.h"

// 初始化静态成员变量
QString AiPromptDialog::s_lastUsedApiKey = "";

AiPromptDialog::AiPromptDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AiPromptDialog)
{
    ui->setupUi(this);
    // 对话框打开时，自动填入上次使用过的Key
    ui->apiKeyLineEdit->setText(s_lastUsedApiKey);
    // 在这里设置下拉框的默认选项
    ui->modelComboBox->setCurrentIndex(0);
}

AiPromptDialog::~AiPromptDialog()
{
    // 对话框关闭前，“记住”当前输入的Key，以便下次使用
    s_lastUsedApiKey = ui->apiKeyLineEdit->text();
    delete ui;
}

QString AiPromptDialog::getPromptText() const
{
    // [ 关键修正 ]
    // 使用我们在UI设计器中定义的正确名字 "promptTextEdit"
    return ui->promptTextEdit->toPlainText();
}

QString AiPromptDialog::getSelectedModelName() const
{
    // 使用我们在UI设计器中定义的正确名字 "modelComboBox"
    return ui->modelComboBox->currentText();
}

QString AiPromptDialog::getApiKey() const
{
    // 使用我们在UI设计器中定义的正确名字 "apiKeyLineEdit"
    return ui->apiKeyLineEdit->text();
}
