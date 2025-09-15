// ----------------- aipromptdialog.h (完整替换) -----------------
#ifndef AIPROMPTDIALOG_H
#define AIPROMPTDIALOG_H

#include <QDialog>

namespace Ui {
class AiPromptDialog;
}

class AiPromptDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AiPromptDialog(QWidget *parent = nullptr);
    ~AiPromptDialog();


    QString getPromptText() const;
    QString getSelectedModelName() const;
    QString getApiKey() const;

private:
    Ui::AiPromptDialog *ui;

    static QString s_lastUsedApiKey;
};

#endif // AIPROMPTDIALOG_H
