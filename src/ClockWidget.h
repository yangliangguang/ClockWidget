#pragma once
#include <QWidget>
#include <QPoint>
#include <QTime>
#include <QSettings>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLabel>
#include <QTextBrowser>
#include <QPushButton>

class QTimer;
class QMouseEvent;
class QResizeEvent;
class QAction;

class ClockWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ClockWidget(QWidget* parent = nullptr);
    ~ClockWidget();
    
    // 重新加载设置并更新天气数据
    void reloadWeatherSettings();
    
    // 置顶功能
    void loadAlwaysOnTopSetting();
    
private:
    void setAlwaysOnTop(bool alwaysOnTop);
    bool isAlwaysOnTop() const;

protected:
    void paintEvent(QPaintEvent* event) override;
    QSize sizeHint() const override;
    // 支持无边框窗口拖动
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void mouseDoubleClickEvent(QMouseEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    void contextMenuEvent(QContextMenuEvent* event) override;
    void closeEvent(QCloseEvent* event) override;

private slots:
    void showWindow();
    void hideWindow();
    void toggleWindow();
    void onTrayIconActivated(QSystemTrayIcon::ActivationReason reason);
    void onWeatherDataReceived();
    void updateWeatherData();
    void showWeatherBrowser();
    void hideWeatherBrowser();

private:
    void loadWeatherSettings();
    QString getCityCodeFromSettings();
    QTimer* m_timer;
    QString dayOfWeekCn(int dow) const;
    bool m_dragging = false;
    QPoint m_dragOffset; // event->globalPos() - frameGeometry().topLeft()
    void updateWindowMask();
    
    // 闹钟相关方法

    
    // 系统托盘相关
    void createTrayIcon();
    void createTrayMenu();
    
    
    
    // 系统托盘
    QSystemTrayIcon* m_trayIcon;
    QMenu* m_trayMenu;
    QAction* m_showAction;
    QAction* m_hideAction;
    QAction* m_settingsAction;
    QAction* m_exitAction;
    
    // 天气相关
    QNetworkAccessManager* m_networkManager;
    QTimer* m_weatherTimer;
    QString m_weatherText;
    QString m_currentWeather;
    QString m_currentTemperature;
    QString m_cityName;
    
    // 天气浏览器相关
    QTextBrowser* m_weatherBrowser;
    QPushButton* m_weatherButton;
    bool m_weatherBrowserVisible;
};