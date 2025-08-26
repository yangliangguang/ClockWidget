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
    explicit SettingsDialog(bool currentAlwaysOnTop, QWidget *parent = nullptr);
    ~SettingsDialog();

signals:
    void alwaysOnTopChanged(bool enabled);

private slots:
    void onOkClicked();
    void onCancelClicked();
    void onAutoStartChanged(bool checked);
    void onAlwaysOnTopChanged(bool checked);

private:
    void setupUI();
    void loadSettings();
    void loadSettings(bool currentAlwaysOnTop);
    void saveSettings();
    bool isAutoStartEnabled();
    void setAutoStart(bool enabled);
    QString getExecutablePath();
    
    QCheckBox *m_autoStartCheckBox;
    QCheckBox *m_alwaysOnTopCheckBox;
    QPushButton *m_okButton;
    QPushButton *m_cancelButton;
    QGroupBox *m_startupGroup;
    QGroupBox *m_displayGroup;
    
    // 天气设置
    QGroupBox *m_weatherGroup;
    QLineEdit *m_cityCodeEdit;
    QLabel *m_cityCodeLabel;
    QLabel *m_cityCodeHintLabel;
};

#endif // SETTINGSDIALOG_H