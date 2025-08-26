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
    , m_alwaysOnTopCheckBox(nullptr)
    , m_okButton(nullptr)
    , m_cancelButton(nullptr)
    , m_startupGroup(nullptr)
    , m_displayGroup(nullptr)
    , m_weatherGroup(nullptr)
    , m_cityCodeEdit(nullptr)
    , m_cityCodeLabel(nullptr)
    , m_cityCodeHintLabel(nullptr)
{
    setWindowTitle(QStringLiteral("设置"));
    setFixedSize(450, 280);
    setModal(true);
    
    setupUI();
    loadSettings();
}

SettingsDialog::SettingsDialog(bool currentAlwaysOnTop, QWidget *parent)
    : QDialog(parent)
    , m_autoStartCheckBox(nullptr)
    , m_alwaysOnTopCheckBox(nullptr)
    , m_okButton(nullptr)
    , m_cancelButton(nullptr)
    , m_startupGroup(nullptr)
    , m_displayGroup(nullptr)
    , m_weatherGroup(nullptr)
    , m_cityCodeEdit(nullptr)
    , m_cityCodeLabel(nullptr)
    , m_cityCodeHintLabel(nullptr)
{
    setWindowTitle(QStringLiteral("设置"));
    setFixedSize(450, 280);
    setModal(true);
    
    setupUI();
    loadSettings(currentAlwaysOnTop);
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
    
    // 显示设置组
    m_displayGroup = new QGroupBox(QStringLiteral("显示设置"), this);
    QVBoxLayout *displayLayout = new QVBoxLayout(m_displayGroup);
    
    m_alwaysOnTopCheckBox = new QCheckBox(QStringLiteral("窗口置顶"), this);
    displayLayout->addWidget(m_alwaysOnTopCheckBox);
    
    mainLayout->addWidget(m_displayGroup);
    
    // 天气设置组
    m_weatherGroup = new QGroupBox(QStringLiteral("天气设置"), this);
    QVBoxLayout *weatherLayout = new QVBoxLayout(m_weatherGroup);
    
    m_cityCodeLabel = new QLabel(QStringLiteral("城市编码:"), this);
    m_cityCodeEdit = new QLineEdit(this);
    m_cityCodeEdit->setPlaceholderText(QStringLiteral("请输入城市编码，如：101210408"));
    
    m_cityCodeHintLabel = new QLabel(QStringLiteral("提示：可在 http://t.weather.itboy.net/api/weather/city/城市编码 查询"), this);
    m_cityCodeHintLabel->setStyleSheet("color: gray; font-size: 10px;");
    m_cityCodeHintLabel->setWordWrap(true);
    
    weatherLayout->addWidget(m_cityCodeLabel);
    weatherLayout->addWidget(m_cityCodeEdit);
    weatherLayout->addWidget(m_cityCodeHintLabel);
    
    mainLayout->addWidget(m_weatherGroup);
    
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
    connect(m_alwaysOnTopCheckBox, &QCheckBox::toggled, this, &SettingsDialog::onAlwaysOnTopChanged);
}

void SettingsDialog::loadSettings()
{
    // 检查当前开机自启动状态
    bool autoStartEnabled = isAutoStartEnabled();
    m_autoStartCheckBox->setChecked(autoStartEnabled);
    
    // 加载置顶设置
    QSettings settings;
    bool alwaysOnTop = settings.value("display/alwaysOnTop", false).toBool();
    m_alwaysOnTopCheckBox->setChecked(alwaysOnTop);
    
    // 加载天气城市编码设置
    QString cityCode = settings.value("weather/cityCode", "101210408").toString(); // 默认杭州
    m_cityCodeEdit->setText(cityCode);
}

void SettingsDialog::loadSettings(bool currentAlwaysOnTop)
{
    // 检查当前开机自启动状态
    bool autoStartEnabled = isAutoStartEnabled();
    m_autoStartCheckBox->setChecked(autoStartEnabled);
    
    // 使用传入的当前置顶状态而不是从配置文件读取
    m_alwaysOnTopCheckBox->setChecked(currentAlwaysOnTop);
    
    // 加载天气城市编码设置
    QSettings settings;
    QString cityCode = settings.value("weather/cityCode", "101210408").toString(); // 默认杭州
    m_cityCodeEdit->setText(cityCode);
}

void SettingsDialog::saveSettings()
{
    // 保存开机自启动设置
    bool autoStart = m_autoStartCheckBox->isChecked();
    setAutoStart(autoStart);
    
    // 保存置顶设置
    QSettings settings;
    bool alwaysOnTop = m_alwaysOnTopCheckBox->isChecked();
    settings.setValue("display/alwaysOnTop", alwaysOnTop);
    
    // 保存天气城市编码设置
    QString cityCode = m_cityCodeEdit->text().trimmed();
    if (cityCode.isEmpty()) {
        cityCode = "101210408"; // 默认杭州
    }
    settings.setValue("weather/cityCode", cityCode);
    settings.sync();
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

void SettingsDialog::onAlwaysOnTopChanged(bool checked)
{
    // 发射信号通知ClockWidget置顶设置变化
    emit alwaysOnTopChanged(checked);
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