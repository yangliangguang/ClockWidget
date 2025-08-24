#include "SettingsDialog.h"
#include <QApplication>
#include <QMessageBox>
#include <QStandardPaths>
#include <QDir>
#include <QFileInfo>
#ifdef Q_OS_WIN
#include <QSettings>
#endif

SettingsDialog::SettingsDialog(QWidget *parent)
    : QDialog(parent)
    , m_autoStartCheckBox(nullptr)
    , m_okButton(nullptr)
    , m_cancelButton(nullptr)
    , m_startupGroup(nullptr)
{
    setWindowTitle(QStringLiteral("设置"));
    setFixedSize(400, 150);
    setModal(true);
    
    setupUI();
    loadSettings();
}

SettingsDialog::~SettingsDialog()
{
}

void SettingsDialog::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    
    // 启动设置组
    m_startupGroup = new QGroupBox(QStringLiteral("启动设置"), this);
    QVBoxLayout *startupLayout = new QVBoxLayout(m_startupGroup);
    
    m_autoStartCheckBox = new QCheckBox(QStringLiteral("开机自动启动"), this);
    startupLayout->addWidget(m_autoStartCheckBox);
    
    mainLayout->addWidget(m_startupGroup);
    
    // 按钮布局
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    m_okButton = new QPushButton(QStringLiteral("确定"), this);
    m_cancelButton = new QPushButton(QStringLiteral("取消"), this);
    
    buttonLayout->addStretch();
    buttonLayout->addWidget(m_okButton);
    buttonLayout->addWidget(m_cancelButton);
    
    mainLayout->addLayout(buttonLayout);
    
    // 连接信号
    connect(m_okButton, &QPushButton::clicked, this, &SettingsDialog::onOkClicked);
    connect(m_cancelButton, &QPushButton::clicked, this, &SettingsDialog::onCancelClicked);
    connect(m_autoStartCheckBox, &QCheckBox::toggled, this, &SettingsDialog::onAutoStartChanged);
}

void SettingsDialog::loadSettings()
{
    // 检查当前开机自启动状态
    bool autoStartEnabled = isAutoStartEnabled();
    m_autoStartCheckBox->setChecked(autoStartEnabled);
}

void SettingsDialog::saveSettings()
{
    // 保存开机自启动设置
    bool autoStart = m_autoStartCheckBox->isChecked();
    setAutoStart(autoStart);
}

void SettingsDialog::onOkClicked()
{
    saveSettings();
    accept();
}

void SettingsDialog::onCancelClicked()
{
    reject();
}

void SettingsDialog::onAutoStartChanged(bool checked)
{
    Q_UNUSED(checked)
    // 可以在这里添加实时预览逻辑
}

bool SettingsDialog::isAutoStartEnabled()
{
#ifdef Q_OS_WIN
    QSettings settings("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Run", QSettings::NativeFormat);
    return settings.contains("ClockWidget");
#else
    return false;
#endif
}

void SettingsDialog::setAutoStart(bool enabled)
{
#ifdef Q_OS_WIN
    QSettings settings("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Run", QSettings::NativeFormat);
    
    if (enabled) {
        QString executablePath = getExecutablePath();
        if (!executablePath.isEmpty()) {
            settings.setValue("ClockWidget", executablePath);
        } else {
            QMessageBox::warning(this, QStringLiteral("错误"), 
                               QStringLiteral("无法获取程序路径，设置开机自启动失败。"));
        }
    } else {
        settings.remove("ClockWidget");
    }
    
    settings.sync();
    
    // 验证设置是否成功
    bool actualState = isAutoStartEnabled();
    if (actualState != enabled) {
        QString message = enabled ? 
            QStringLiteral("设置开机自启动失败，可能需要管理员权限。") :
            QStringLiteral("取消开机自启动失败。");
        QMessageBox::warning(this, QStringLiteral("警告"), message);
        
        // 更新复选框状态以反映实际状态
        m_autoStartCheckBox->setChecked(actualState);
    }
#else
    Q_UNUSED(enabled)
    QMessageBox::information(this, QStringLiteral("提示"), 
                           QStringLiteral("开机自启动功能仅在Windows系统下可用。"));
#endif
}

QString SettingsDialog::getExecutablePath()
{
    QString executablePath = QApplication::applicationFilePath();
    QFileInfo fileInfo(executablePath);
    
    if (fileInfo.exists()) {
        // 使用引号包围路径以处理包含空格的路径
        return QString("\"%1\"").arg(QDir::toNativeSeparators(executablePath));
    }
    
    return QString();
}