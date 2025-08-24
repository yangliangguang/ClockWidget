#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>
#include <QCheckBox>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QGroupBox>
#include <QSettings>
#include <QTimeEdit>
#include <QTime>
#include <QLineEdit>

class SettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SettingsDialog(QWidget *parent = nullptr);
    ~SettingsDialog();

private slots:
    void onOkClicked();
    void onCancelClicked();
    void onAutoStartChanged(bool checked);

private:
    void setupUI();
    void loadSettings();
    void saveSettings();
    bool isAutoStartEnabled();
    void setAutoStart(bool enabled);
    QString getExecutablePath();
    
    QCheckBox *m_autoStartCheckBox;
    QPushButton *m_okButton;
    QPushButton *m_cancelButton;
    QGroupBox *m_startupGroup;
    
    // 天气设置
    QGroupBox *m_weatherGroup;
    QLineEdit *m_cityCodeEdit;
    QLabel *m_cityCodeLabel;
    QLabel *m_cityCodeHintLabel;
};

#endif // SETTINGSDIALOG_H