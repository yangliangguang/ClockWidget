#include "ClockWidget.h"
#include "SettingsDialog.h"
#include <QPainter>
#include <QTimer>
#include <QTime>
#include <QDate>
#include <QtMath>
#include <QMouseEvent>
#include <QRegion>
#include <QResizeEvent>
#include <QContextMenuEvent>
#include <QMenu>
#include <QAction>
#include <QProcess>
#include <QFileInfo>
#include <QMessageBox>
#include <QDesktopServices>
#include <QUrl>
#include <QRandomGenerator>
#include <QCloseEvent>
#include <QApplication>
#include <QStyle>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QLabel>
#include <QUrl>
#include <QTextBrowser>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSvgRenderer>
#include <QFileInfo>

ClockWidget::ClockWidget(QWidget* parent)
    : QWidget(parent), m_timer(new QTimer(this))
    , m_trayIcon(nullptr)
    , m_trayMenu(nullptr)
    , m_showAction(nullptr)
    , m_hideAction(nullptr)
    , m_settingsAction(nullptr)
    , m_exitAction(nullptr)
    , m_networkManager(new QNetworkAccessManager(this))
    , m_weatherTimer(new QTimer(this))
    , m_weatherText(QStringLiteral("加载天气中..."))
    , m_cityName(QStringLiteral("杭州"))
    , m_weatherBrowser(nullptr)
    , m_weatherButton(nullptr)
    , m_weatherBrowserVisible(false)
{
    setWindowTitle(QStringLiteral("时钟"));
    setMinimumSize(400, 400);
    // 无边框 + 圆角显示
    setWindowFlags(windowFlags() | Qt::FramelessWindowHint | Qt::WindowSystemMenuHint | Qt::WindowMinMaxButtonsHint);
    setAttribute(Qt::WA_TranslucentBackground, true);
    setAutoFillBackground(false);

    connect(m_timer, &QTimer::timeout, this, QOverload<>::of(&ClockWidget::update));
    m_timer->start(1000);
    updateWindowMask();
    
    // 创建系统托盘
    createTrayIcon();
    createTrayMenu();
    
    // 初始化天气相关
    
    // 创建天气浏览器组件
    m_weatherBrowser = new QTextBrowser(this);
    m_weatherBrowser->setWindowTitle("天气信息");
    m_weatherBrowser->setWindowFlags(Qt::Window);
    m_weatherBrowser->hide();
    m_weatherBrowser->resize(400, 300);
    m_weatherBrowser->setHtml("<h2>天气信息</h2><p>正在加载天气数据...</p>");
    
    // 创建天气按钮
    m_weatherButton = new QPushButton("天气", this);
    m_weatherButton->setGeometry(10, 10, 60, 30);
    m_weatherButton->setStyleSheet("QPushButton { background-color: rgba(255, 255, 255, 180); border: 1px solid #ccc; border-radius: 5px; }");
    connect(m_weatherButton, &QPushButton::clicked, this, &ClockWidget::showWeatherBrowser);
    
    // 连接天气相关信号槽
    connect(m_weatherTimer, &QTimer::timeout, this, &ClockWidget::updateWeatherData);
    
    // 启动天气数据获取
    updateWeatherData();
    m_weatherTimer->start(600000); // 每10分钟更新一次天气
}

ClockWidget::~ClockWidget() = default;

QSize ClockWidget::sizeHint() const { return {420, 420}; }

QString ClockWidget::dayOfWeekCn(int dow) const
{
    static const QString names[8] = {
        QString(), QStringLiteral("星期一"), QStringLiteral("星期二"), QStringLiteral("星期三"),
        QStringLiteral("星期四"), QStringLiteral("星期五"), QStringLiteral("星期六"), QStringLiteral("星期日")
    };
    if (dow < 1 || dow >7) return QString();
    return names[dow];
}

void ClockWidget::paintEvent(QPaintEvent*)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);
    p.setRenderHint(QPainter::SmoothPixmapTransform, true);
    p.setRenderHint(QPainter::HighQualityAntialiasing, true);

    const int w = width();
    const int h = height();
    const int side = qMin(w,h);
    const QPointF center(w/2.0, h/2.0);

    // 高级渐变背景：动态时间色彩
    QTime currentTime = QTime::currentTime();
    int hour = currentTime.hour();
    QColor bgColor1, bgColor2;
    
    // 根据时间段调整背景色调 - 增强版
    if (hour >= 6 && hour < 12) {
        // 早晨：温暖的金色调 - 晨光渐变
        bgColor1 = QColor(255, 251, 235);  // 温暖的象牙白
        bgColor2 = QColor(255, 243, 205);  // 柔和的香槟金
    } else if (hour >= 12 && hour < 18) {
        // 下午：清新的蓝色调 - 天空渐变
        bgColor1 = QColor(245, 251, 255);  // 纯净的天空蓝
        bgColor2 = QColor(225, 242, 255);  // 深邃的天蓝
    } else if (hour >= 18 && hour < 22) {
        // 傍晚：温暖的橙色调 - 夕阳渐变
        bgColor1 = QColor(255, 248, 240);  // 温暖的米色
        bgColor2 = QColor(255, 235, 205);  // 柔和的桃色
    } else {
        // 夜晚：深邃的紫色调 - 星空渐变
        bgColor1 = QColor(20, 24, 40);     // 深邃的午夜蓝
        bgColor2 = QColor(35, 42, 70);     // 神秘的深紫
    }
    
    // 主背景渐变
    QRadialGradient bg(center, side/2.0);
    bg.setColorAt(0, bgColor1);
    bg.setColorAt(0.7, bgColor2);
    bg.setColorAt(1, bgColor2.darker(105)); // 边缘稍微加深
    p.setBrush(bg);
    p.setPen(Qt::NoPen);
    p.drawRect(rect());
    
    // 添加中间层光晕效果
    QRadialGradient midGradient(center, side*0.6);
    QColor midColor = bgColor1;
    midColor.setAlpha(30); // 很淡的光晕
    midGradient.setColorAt(0, midColor);
    midGradient.setColorAt(0.5, QColor(midColor.red(), midColor.green(), midColor.blue(), 15));
    midGradient.setColorAt(1, QColor(0, 0, 0, 0)); // 完全透明
    p.setBrush(midGradient);
    p.drawRect(rect());

    // 绘制背景猫咪
    QString catPath = QApplication::applicationDirPath() + "/src/cat.svg";
    QSvgRenderer catRenderer(catPath);
    if (catRenderer.isValid()) {
        // 计算猫咪的位置和大小 - 移到时钟左上角更明显的位置
        qreal catSize = side * 0.25; // 猫咪大小为时钟的25%
        QRectF catRect(center.x() - side*0.45, center.y() - side*0.45, catSize, catSize);
        
        // 设置猫咪的透明度
        p.setOpacity(0.8);
        catRenderer.render(&p, catRect);
        p.setOpacity(1.0); // 恢复透明度
    } else {
        // 如果SVG加载失败，绘制一个简单的圆形作为替代
        p.setBrush(QColor(255, 179, 102, 200)); // 橙色半透明
        p.setPen(Qt::NoPen);
        qreal catSize = side * 0.15;
        QRectF fallbackRect(center.x() - side*0.45, center.y() - side*0.45, catSize, catSize);
        p.drawEllipse(fallbackRect);
    }

    // 主容器：精致的玻璃质感设计
    QRectF mainRect(center.x() - side*0.42, center.y() - side*0.42, side*0.84, side*0.84);
    QPainterPath mainPath;
    qreal mainRadius = side * 0.08;
    mainPath.addRoundedRect(mainRect, mainRadius, mainRadius);
    
    // 多层阴影效果
    for (int i = 0; i < 3; ++i) {
        p.save();
        QRectF shadowRect = mainRect.adjusted(i*2, i*2, i*2, i*2);
        QPainterPath shadowPath;
        shadowPath.addRoundedRect(shadowRect, mainRadius, mainRadius);
        p.setBrush(QColor(0, 0, 0, 15 - i*3));
        p.setPen(Qt::NoPen);
        p.drawPath(shadowPath);
        p.restore();
    }
    
    // 主容器玻璃质感背景 - 增强版多层渐变
    QRadialGradient mainGradient(center, mainRect.width()/2);
    if (hour >= 22 || hour < 6) {
        // 夜间模式：深色玻璃 - 星空质感
        mainGradient.setColorAt(0, QColor(70, 75, 95, 220));   // 中心亮点
        mainGradient.setColorAt(0.3, QColor(55, 62, 85, 210)); // 中间过渡
        mainGradient.setColorAt(0.7, QColor(45, 52, 75, 200)); // 外围渐变
        mainGradient.setColorAt(1, QColor(35, 42, 65, 230));   // 边缘深化
    } else {
        // 日间模式：亮色玻璃 - 水晶质感
        mainGradient.setColorAt(0, QColor(255, 255, 255, 250)); // 中心高光
        mainGradient.setColorAt(0.2, QColor(252, 252, 255, 240)); // 内层光晕
        mainGradient.setColorAt(0.6, QColor(248, 250, 255, 220)); // 中层过渡
        mainGradient.setColorAt(1, QColor(240, 245, 255, 210));   // 边缘柔化
    }
    p.setBrush(mainGradient);
    p.setPen(QPen(QColor(255, 255, 255, 100), 1.5));
    p.drawPath(mainPath);

    // 表盘：极简圆形设计
    QRectF faceRect = QRectF(center.x() - side*0.35, center.y() - side*0.35, side*0.70, side*0.70);
    
    // 表盘背景渐变 - 精致多层次
    QRadialGradient faceGradient(center, faceRect.width()/2);
    if (hour >= 22 || hour < 6) {
        // 夜间模式表盘 - 深邃星空
        faceGradient.setColorAt(0, QColor(95, 100, 120));      // 中心亮区
        faceGradient.setColorAt(0.3, QColor(85, 90, 110));     // 内层过渡
        faceGradient.setColorAt(0.6, QColor(70, 75, 95));      // 中层渐变
        faceGradient.setColorAt(0.85, QColor(55, 62, 80));     // 外层深化
        faceGradient.setColorAt(1, QColor(45, 52, 70));        // 边缘暗化
    } else {
        // 日间模式表盘 - 纯净光辉
        faceGradient.setColorAt(0, QColor(255, 255, 255));     // 中心纯白
        faceGradient.setColorAt(0.2, QColor(253, 253, 255));   // 内层微光
        faceGradient.setColorAt(0.5, QColor(250, 252, 255));   // 中层柔光
        faceGradient.setColorAt(0.8, QColor(245, 248, 255));   // 外层渐变
        faceGradient.setColorAt(1, QColor(238, 242, 250));     // 边缘柔化
    }
    p.setBrush(faceGradient);
    p.setPen(Qt::NoPen);
    p.drawEllipse(faceRect);
    
    // 表盘边框
    p.setBrush(Qt::NoBrush);
    QColor borderColor = (hour >= 22 || hour < 6) ? QColor(120, 120, 140, 150) : QColor(200, 200, 220, 150);
    p.setPen(QPen(borderColor, 2));
    p.drawEllipse(faceRect);

    // 精致的刻度线
    qreal faceRadius = faceRect.width() / 2.0;
    QColor tickColor = (hour >= 22 || hour < 6) ? QColor(180, 180, 200) : QColor(120, 120, 140);
    
    for (int i = 0; i < 60; ++i) {
        qreal angle = i * 6.0 * M_PI / 180.0;
        QPointF outer = center + QPointF(qCos(angle - M_PI/2) * (faceRadius - 8), 
                                        qSin(angle - M_PI/2) * (faceRadius - 8));
        QPointF inner;
        
        if (i % 15 == 0) {
            // 主刻度（12, 3, 6, 9点）
            inner = center + QPointF(qCos(angle - M_PI/2) * (faceRadius - 25), 
                                    qSin(angle - M_PI/2) * (faceRadius - 25));
            p.setPen(QPen(tickColor, 3, Qt::SolidLine, Qt::RoundCap));
        } else if (i % 5 == 0) {
            // 次要刻度（小时）
            inner = center + QPointF(qCos(angle - M_PI/2) * (faceRadius - 18), 
                                    qSin(angle - M_PI/2) * (faceRadius - 18));
            p.setPen(QPen(tickColor, 2, Qt::SolidLine, Qt::RoundCap));
        } else {
            // 分钟刻度
            inner = center + QPointF(qCos(angle - M_PI/2) * (faceRadius - 12), 
                                    qSin(angle - M_PI/2) * (faceRadius - 12));
            p.setPen(QPen(QColor(tickColor.red(), tickColor.green(), tickColor.blue(), 100), 1));
        }
        
        p.drawLine(outer, inner);
    }

    // 时钟数字：优雅的主要时间点
    QColor numberColor = (hour >= 22 || hour < 6) ? QColor(220, 220, 240) : QColor(60, 60, 80);
    p.setPen(numberColor);
    QFont numberFont = font();
    numberFont.setPointSizeF(qMax(12.0, side/35.0));
    numberFont.setWeight(QFont::Bold);
    p.setFont(numberFont);
    
    int rNum = int(faceRect.width()/2.0) - 35;
    for (int i=1;i<=12;++i) {
        if (i % 3 != 0) continue;
        qreal angle = (i-3) * 30.0 * M_PI/180.0;
        QPointF pos = center + QPointF(qCos(angle)*rNum, qSin(angle)*rNum);
        QRectF tr(pos.x()-20, pos.y()-15, 40, 30);
        QString text = QString::number(i == 12 ? 12 : i);
        
        // 添加文字阴影效果
        p.setPen(QColor(0, 0, 0, 50));
        p.drawText(tr.adjusted(1, 1, 1, 1), Qt::AlignCenter, text);
        p.setPen(numberColor);
        p.drawText(tr, Qt::AlignCenter, text);
    }

    // 获取当前时间
    QTime t = QTime::currentTime();
    QDate d = QDate::currentDate();

    // 精致的指针绘制函数
    auto drawHand = [&](qreal angleDeg, qreal length, const QColor& color, int width, bool isSecond = false){
        p.save();
        p.translate(center);
        p.rotate(angleDeg);
        
        if (isSecond) {
            // 秒针：细长优雅
            p.setPen(QPen(color, width, Qt::SolidLine, Qt::RoundCap));
            p.drawLine(QPointF(0, length*0.2), QPointF(0, -length));
            // 秒针尾部
            p.setBrush(color);
            p.setPen(Qt::NoPen);
            p.drawEllipse(QPointF(0, length*0.25), 3, 3);
        } else {
            // 时针分针：带阴影的立体效果
            QPainterPath handPath;
            handPath.moveTo(0, 0);
            handPath.lineTo(-width/2, -length*0.1);
            handPath.lineTo(-width/3, -length);
            handPath.lineTo(width/3, -length);
            handPath.lineTo(width/2, -length*0.1);
            handPath.closeSubpath();
            
            // 阴影
            p.translate(1, 1);
            p.setBrush(QColor(0, 0, 0, 80));
            p.setPen(Qt::NoPen);
            p.drawPath(handPath);
            
            // 主体
            p.translate(-1, -1);
            p.setBrush(color);
            p.setPen(QPen(color.darker(120), 1));
            p.drawPath(handPath);
        }
        p.restore();
    };

    qreal rHand = faceRect.width()/2.0 - 40;
    qreal secAngle = t.second()*6.0;
    qreal minAngle = t.minute()*6.0 + t.second()*0.1;
    qreal hourAngle = ((t.hour()%12)*30.0) + t.minute()*0.5;
 
    // 基于表盘的字体与布局
    const qreal base = faceRect.height();
 
    // 数字时间显示：中心位置
    QFont timeFont = font();
    timeFont.setPointSizeF(qMax(10.0, base/12.0));
    timeFont.setWeight(QFont::Medium);
    p.setFont(timeFont);
    
    QColor textColor = (hour >= 22 || hour < 6) ? QColor(200, 200, 220) : QColor(60, 60, 80);
    QString timeStr = t.toString("HH:mm:ss");
    
    // 时间文字阴影
    p.setPen(QColor(0, 0, 0, 60));
    QRectF timeRect(center.x() - base*0.3, center.y() - base*0.08, base*0.6, base*0.16);
    p.drawText(timeRect.adjusted(1, 1, 1, 1), Qt::AlignCenter, timeStr);
    
    // 时间文字主体
    p.setPen(textColor);
    p.drawText(timeRect, Qt::AlignCenter, timeStr);

    // 星期显示：上方
    QFont weekF = font();
    weekF.setPointSizeF(qMax(9.0, base/12.0));
    weekF.setWeight(QFont::Normal);
    p.setFont(weekF);
    p.setPen(QColor(textColor.red(), textColor.green(), textColor.blue(), 160));
    QRectF weekRect(center.x() - base*0.3, center.y() - base*0.25, base*0.6, base*0.1);
    p.drawText(weekRect, Qt::AlignCenter, dayOfWeekCn(d.dayOfWeek()));

    // 日期显示：下方
    QFont dateF = font();
    dateF.setPointSizeF(qMax(8.0, base/14.0));
    dateF.setWeight(QFont::Normal);
    p.setFont(dateF);
    p.setPen(QColor(textColor.red(), textColor.green(), textColor.blue(), 140));
    QRectF dateRect(center.x() - base*0.35, center.y() + base*0.15, base*0.7, base*0.1);
    p.drawText(dateRect, Qt::AlignCenter, d.toString("yyyy年MM月dd日"));

    // 天气显示：在日期下方，字体较小
    QFont weatherF = font();
    weatherF.setPointSizeF(qMax(3.0, base/36.0));  // 字体再小一半
    weatherF.setWeight(QFont::Light);
    p.setFont(weatherF);
    p.setPen(QColor(textColor.red(), textColor.green(), textColor.blue(), 120));
    QRectF weatherRect(center.x() - base*0.4, center.y() + base*0.26, base*0.8, base*0.08);
    p.drawText(weatherRect, Qt::AlignCenter, m_weatherText.isEmpty() ? "加载天气中..." : m_weatherText);

    // 绘制指针：从短到长 - 增强版
    QColor handColor, secColor;
    if (hour >= 22 || hour < 6) {
        // 夜间模式指针 - 50%透明度
        handColor = QColor(200, 205, 220, 128); // 更亮的夜间指针，50%透明度
        secColor = QColor(255, 100, 120);       // 温暖的粉红色秒针
    } else {
        // 日间模式指针 - 50%透明度
        handColor = QColor(60, 70, 90, 128);    // 深邃的日间指针，50%透明度
        secColor = QColor(255, 69, 58);         // 经典苹果红秒针
    }
    
    drawHand(hourAngle, rHand*0.5, handColor, 5);
    drawHand(minAngle, rHand*0.75, handColor, 4);
    drawHand(secAngle, rHand*0.85, secColor, 2, true);

    // 中心装饰：精致多层圆点 - 增强版
    // 最外圈阴影
    p.setBrush(QColor(0, 0, 0, 40));
    p.setPen(Qt::NoPen);
    p.drawEllipse(center + QPointF(1, 1), 14, 14);
    
    // 外圈光晕
    QRadialGradient outerGrad(center, 12);
    outerGrad.setColorAt(0, QColor(handColor.red(), handColor.green(), handColor.blue(), 120));
    outerGrad.setColorAt(1, QColor(handColor.red(), handColor.green(), handColor.blue(), 60));
    p.setBrush(outerGrad);
    p.drawEllipse(center, 12, 12);
    
    // 中圈主体
    QRadialGradient midGrad(center, 8);
    midGrad.setColorAt(0, handColor.lighter(110));
    midGrad.setColorAt(1, handColor);
    p.setBrush(midGrad);
    p.setPen(QPen(QColor(255, 255, 255, 80), 0.5));
    p.drawEllipse(center, 8, 8);
    
    // 内圈高光
    QRadialGradient innerGrad(center, 4);
    innerGrad.setColorAt(0, secColor.lighter(120));
    innerGrad.setColorAt(0.7, secColor);
    innerGrad.setColorAt(1, secColor.darker(110));
    p.setBrush(innerGrad);
    p.setPen(QPen(QColor(255, 255, 255, 150), 1));
    p.drawEllipse(center, 4, 4);
    
    // 最内圈亮点
    p.setBrush(QColor(255, 255, 255, 100));
    p.setPen(Qt::NoPen);
    p.drawEllipse(center + QPointF(-1, -1), 2, 2);
 
     // 去掉天气信息（不再绘制天气占位）
}

// --- 鼠标拖动 ---
void ClockWidget::mousePressEvent(QMouseEvent* e)
{
    if (e->button() == Qt::LeftButton) {
        m_dragging = true;
        m_dragOffset = e->globalPos() - frameGeometry().topLeft();
        e->accept();
    } else {
        QWidget::mousePressEvent(e);
    }
}

void ClockWidget::mouseMoveEvent(QMouseEvent* e)
{
    if (m_dragging && (e->buttons() & Qt::LeftButton)) {
        move(e->globalPos() - m_dragOffset);
        e->accept();
    } else {
        QWidget::mouseMoveEvent(e);
    }
}

void ClockWidget::mouseReleaseEvent(QMouseEvent* e)
{
    if (m_dragging && e->button() == Qt::LeftButton) {
        m_dragging = false;
        e->accept();
    } else {
        QWidget::mouseReleaseEvent(e);
    }
}

void ClockWidget::mouseDoubleClickEvent(QMouseEvent* e)
{
    if (e->button() == Qt::LeftButton) {
        showMinimized();
        e->accept();
    } else {
        QWidget::mouseDoubleClickEvent(e);
    }
}

void ClockWidget::resizeEvent(QResizeEvent* e)
{
    QWidget::resizeEvent(e);
    updateWindowMask();
}

void ClockWidget::updateWindowMask()
{
    // 仅使用最小边作为圆直径
    int side = qMin(width(), height());
    QRect r((width()-side)/2, (height()-side)/2, side, side);
    QRegion region(r, QRegion::Ellipse);
    setMask(region);
    // 增加抗锯齿外观
    setAttribute(Qt::WA_TranslucentBackground, true);
}

void ClockWidget::contextMenuEvent(QContextMenuEvent* event)
{
    QMenu menu(this);
    
    // 设置现代化菜单样式
    menu.setStyleSheet(
        "QMenu {"
        "    background: qlineargradient(x1:0, y1:0, x2:0, y2:1, "
        "                                stop:0 rgba(255, 255, 255, 245), "
        "                                stop:0.5 rgba(250, 252, 255, 240), "
        "                                stop:1 rgba(245, 248, 252, 235));"
        "    border: 2px solid rgba(180, 200, 230, 150);"
        "    border-radius: 15px;"
        "    padding: 12px 6px;"
        "    margin: 4px;"
        "    font-family: 'Microsoft YaHei UI', 'Segoe UI', 'SF Pro Display', Arial, sans-serif;"
        "    font-size: 14px;"
        "    font-weight: 500;"
        "    letter-spacing: 0.3px;"
        "}"
        "QMenu::item {"
        "    background: transparent;"
        "    color: rgba(45, 55, 72, 255);"
        "    padding: 12px 25px 12px 50px;"
        "    margin: 3px 8px;"
        "    border-radius: 10px;"
        "    min-width: 160px;"
        "    transition: all 0.2s ease-in-out;"
        "}"
        "QMenu::item:hover {"
        "    background: qlineargradient(x1:0, y1:0, x2:0, y2:1, "
        "                                stop:0 rgba(120, 170, 255, 200), "
        "                                stop:0.5 rgba(100, 150, 255, 180), "
        "                                stop:1 rgba(80, 130, 255, 160));"
        "    color: white;"
        "    border: 1px solid rgba(60, 120, 255, 120);"
        "    transform: translateX(2px);"
        "}"
        "QMenu::item:selected {"
        "    background: qlineargradient(x1:0, y1:0, x2:0, y2:1, "
        "                                stop:0 rgba(100, 150, 255, 220), "
        "                                stop:0.5 rgba(80, 130, 255, 200), "
        "                                stop:1 rgba(60, 110, 255, 180));"
        "    color: white;"
        "    border: 1px solid rgba(50, 100, 255, 150);"
        "    font-weight: 600;"
        "}"
        "QMenu::item:pressed {"
        "    background: qlineargradient(x1:0, y1:0, x2:0, y2:1, "
        "                                stop:0 rgba(70, 120, 255, 240), "
        "                                stop:1 rgba(50, 100, 255, 220));"
        "    transform: scale(0.98);"
        "}"
        "QMenu::separator {"
        "    height: 2px;"
        "    background: qlineargradient(x1:0, y1:0, x2:1, y2:0, "
        "                                stop:0 transparent, "
        "                                stop:0.2 rgba(160, 180, 210, 80), "
        "                                stop:0.5 rgba(180, 200, 230, 120), "
        "                                stop:0.8 rgba(160, 180, 210, 80), "
        "                                stop:1 transparent);"
        "    margin: 8px 20px;"
        "    border-radius: 1px;"
        "}"
    );
    
    // 创建带图标的菜单项（使用Qt标准图标）
    QAction* wechatAction = menu.addAction(style()->standardIcon(QStyle::SP_ComputerIcon), QStringLiteral("打开微信"));
    QAction* weatherAction = menu.addAction(style()->standardIcon(QStyle::SP_DialogApplyButton), QStringLiteral("打开天气"));
    QAction* traeAction = menu.addAction(style()->standardIcon(QStyle::SP_FileDialogStart), QStringLiteral("打开Trae"));
    QAction* traeCnAction = menu.addAction(style()->standardIcon(QStyle::SP_FileDialogStart), QStringLiteral("打开Trae CN"));
    QAction* videoAction = menu.addAction(style()->standardIcon(QStyle::SP_MediaPlay), QStringLiteral("打开视频"));
    QAction* yuanbaoAction = menu.addAction(style()->standardIcon(QStyle::SP_DialogYesButton), QStringLiteral("打开元宝"));
    menu.addSeparator();
    QAction* settingsAction = menu.addAction(style()->standardIcon(QStyle::SP_FileDialogDetailedView), QStringLiteral("设置"));
    menu.addSeparator();
    QAction* exitAction = menu.addAction(style()->standardIcon(QStyle::SP_DialogCloseButton), QStringLiteral("退出"));
    
    connect(wechatAction, &QAction::triggered, [this]() {
        QStringList wechatPaths = {
            "C:\\Program Files (x86)\\Tencent\\Weixin\\Weixin.exe",
            "C:\\Program Files\\Tencent\\WeChat\\WeChat.exe"
        };
        
        bool success = false;
        QString foundPath;
        
        // 首先检查文件是否存在
        for (const QString& path : wechatPaths) {
            QFileInfo fileInfo(path);
            if (fileInfo.exists() && fileInfo.isExecutable()) {
                foundPath = path;
                break;
            }
        }
        
        if (!foundPath.isEmpty()) {
            // 使用QDesktopServices启动，更可靠
            success = QDesktopServices::openUrl(QUrl::fromLocalFile(foundPath));
            
            // 如果QDesktopServices失败，尝试QProcess
            if (!success) {
                QProcess process;
                success = process.startDetached(foundPath);
            }
        }
        
        // 如果直接路径都失败，尝试系统PATH中查找
        if (!success) {
            QStringList commands = {"Weixin.exe", "WeChat.exe"};
            for (const QString& cmd : commands) {
                if (QProcess::startDetached(cmd)) {
                    success = true;
                    break;
                }
            }
        }
        
        // 最后尝试协议启动
        if (!success) {
            success = QDesktopServices::openUrl(QUrl("weixin://"));
        }
        
        // 如果所有方法都失败，显示错误信息
        if (!success) {
            QMessageBox::warning(this, QStringLiteral("错误"), 
                               QStringLiteral("无法启动微信程序。\n请检查微信是否已正确安装。"));
        }
    });
    
    connect(weatherAction, &QAction::triggered, [this]() {
        QString weatherUrl = "https://e.weather.com.cn/mweather/101210408.shtml";
        bool success = QDesktopServices::openUrl(QUrl(weatherUrl));
        
        if (!success) {
            QMessageBox::warning(this, QStringLiteral("错误"), 
                               QStringLiteral("无法打开天气网站。\n请检查网络连接或默认浏览器设置。"));
        }
    });
    
    connect(traeAction, &QAction::triggered, [this]() {
        QString traePath = "D:\\Programs\\Trae\\Trae.exe";
        QFileInfo fileInfo(traePath);
        
        if (fileInfo.exists() && fileInfo.isExecutable()) {
            bool success = QDesktopServices::openUrl(QUrl::fromLocalFile(traePath));
            
            if (!success) {
                QProcess process;
                success = process.startDetached(traePath);
            }
            
            if (!success) {
                QMessageBox::warning(this, QStringLiteral("错误"), 
                                   QStringLiteral("无法启动Trae程序。\n请检查程序是否正常安装。"));
            }
        } else {
            QMessageBox::warning(this, QStringLiteral("错误"), 
                               QStringLiteral("找不到Trae程序。\n路径：D:\\Programs\\Trae\\Trae.exe"));
        }
    });
    
    connect(traeCnAction, &QAction::triggered, [this]() {
        QString traeCnPath = "E:\\Programs\\Trae CN\\Trae CN.exe";
        QFileInfo fileInfo(traeCnPath);
        
        if (fileInfo.exists() && fileInfo.isExecutable()) {
            bool success = QDesktopServices::openUrl(QUrl::fromLocalFile(traeCnPath));
            
            if (!success) {
                QProcess process;
                success = process.startDetached(traeCnPath);
            }
            
            if (!success) {
                QMessageBox::warning(this, QStringLiteral("错误"), 
                                   QStringLiteral("无法启动Trae CN程序。\n请检查程序是否正常安装。"));
            }
        } else {
            QMessageBox::warning(this, QStringLiteral("错误"), 
                               QStringLiteral("找不到Trae CN程序。\n路径：E:\\Programs\\Trae CN\\Trae CN.exe"));
        }
    });
    
    connect(videoAction, &QAction::triggered, [this]() {
        QString videoPath = "D:\\Program Files (x86)\\Tencent\\QQLive\\QQLive.exe";
        QFileInfo fileInfo(videoPath);
        
        if (fileInfo.exists() && fileInfo.isExecutable()) {
            bool success = QDesktopServices::openUrl(QUrl::fromLocalFile(videoPath));
            
            if (!success) {
                QProcess process;
                success = process.startDetached(videoPath);
            }
            
            if (!success) {
                QMessageBox::warning(this, QStringLiteral("错误"), 
                                   QStringLiteral("无法启动腾讯视频程序。\n请检查程序是否正常安装。"));
            }
        } else {
            QMessageBox::warning(this, QStringLiteral("错误"), 
                               QStringLiteral("找不到腾讯视频程序。\n路径：D:\\Program Files (x86)\\Tencent\\QQLive\\QQLive.exe"));
        }
    });
    
    connect(yuanbaoAction, &QAction::triggered, [this]() {
        QString yuanbaoPath = "D:\\Program Files\\Tencent\\Yuanbao\\yuanbao.exe";
        QFileInfo fileInfo(yuanbaoPath);
        
        if (fileInfo.exists() && fileInfo.isExecutable()) {
            bool success = QDesktopServices::openUrl(QUrl::fromLocalFile(yuanbaoPath));
            
            if (!success) {
                QProcess process;
                success = process.startDetached(yuanbaoPath);
            }
            
            if (!success) {
                QMessageBox::warning(this, QStringLiteral("错误"), 
                                   QStringLiteral("无法启动腾讯元宝程序。\n请检查程序是否正常安装。"));
            }
        } else {
            QMessageBox::warning(this, QStringLiteral("错误"), 
                               QStringLiteral("找不到腾讯元宝程序。\n路径：D:\\Program Files\\Tencent\\Yuanbao\\yuanbao.exe"));
        }
    });
    
    connect(settingsAction, &QAction::triggered, [this]() {
        SettingsDialog dialog(this);
        dialog.exec();
    });
    
    connect(exitAction, &QAction::triggered, this, &QWidget::close);
    
    menu.exec(event->globalPos());
}









void ClockWidget::showWeatherBrowser()
{
    if (!m_weatherBrowserVisible) {
        m_weatherBrowserVisible = true;
        
        // 更新天气浏览器内容
        QString weatherHtml = QString(
            "<html><body style='font-family: Arial, sans-serif; padding: 20px;'>" 
            "<h2 style='color: #2E86AB; text-align: center;'>🌤️ 天气信息</h2>"
            "<div style='background: #f0f8ff; padding: 15px; border-radius: 10px; margin: 10px 0;'>"
            "<h3 style='color: #1565C0; margin-top: 0;'>当前天气</h3>"
            "<p style='font-size: 16px; margin: 5px 0;'><strong>%1</strong></p>"
            "</div>"
            "<div style='background: #e8f5e8; padding: 15px; border-radius: 10px; margin: 10px 0;'>"
            "<h3 style='color: #2E7D32; margin-top: 0;'>天气详情</h3>"
            "<p style='margin: 5px 0;'>📍 城市: %2</p>"
            "<p style='margin: 5px 0;'>🌡️ 温度: %3°C</p>"
            "<p style='margin: 5px 0;'>💧 湿度: 请查看完整信息</p>"
            "</div>"
            "<div style='background: #fff3e0; padding: 15px; border-radius: 10px; margin: 10px 0;'>"
            "<h3 style='color: #F57C00; margin-top: 0;'>天气来源</h3>"
            "<p style='margin: 5px 0;'>数据来源: 天气API</p>"
            "<p style='margin: 5px 0;'>更新时间: %4</p>"
            "</div>"
            "</body></html>"
        ).arg(m_weatherText)
         .arg(m_cityName)
         .arg("实时")
         .arg(QTime::currentTime().toString("hh:mm:ss"));
        
        m_weatherBrowser->setHtml(weatherHtml);
        m_weatherBrowser->show();
        m_weatherBrowser->raise();
        
        // 将浏览器窗口定位到时钟窗口旁边
        QPoint pos = this->pos();
        pos.setX(pos.x() + this->width() + 10);
        m_weatherBrowser->move(pos);
        
        // 更新按钮文本
        m_weatherButton->setText("关闭");
        disconnect(m_weatherButton, &QPushButton::clicked, this, &ClockWidget::showWeatherBrowser);
        connect(m_weatherButton, &QPushButton::clicked, this, &ClockWidget::hideWeatherBrowser);
    }
}

void ClockWidget::hideWeatherBrowser()
{
    if (m_weatherBrowserVisible) {
        m_weatherBrowserVisible = false;
        m_weatherBrowser->hide();
        
        // 更新按钮文本
        m_weatherButton->setText("天气");
        disconnect(m_weatherButton, &QPushButton::clicked, this, &ClockWidget::hideWeatherBrowser);
        connect(m_weatherButton, &QPushButton::clicked, this, &ClockWidget::showWeatherBrowser);
    }
 }

void ClockWidget::createTrayIcon()
{
    if (!QSystemTrayIcon::isSystemTrayAvailable()) {
        QMessageBox::critical(this, tr("系统托盘"), tr("系统托盘不可用"));
        return;
    }
    
    m_trayIcon = new QSystemTrayIcon(this);
    // 尝试使用项目中的图标文件，如果不存在则使用系统默认图标
    QIcon trayIcon;
    if (QFileInfo("src/clock-icon.ico").exists()) {
        trayIcon = QIcon("src/clock-icon.ico");
    } else {
        // 使用系统默认的应用程序图标
        trayIcon = style()->standardIcon(QStyle::SP_ComputerIcon);
    }
    m_trayIcon->setIcon(trayIcon);
    m_trayIcon->setToolTip(tr("时钟小工具"));
    
    connect(m_trayIcon, &QSystemTrayIcon::activated, this, &ClockWidget::onTrayIconActivated);
    
    m_trayIcon->show();
}

void ClockWidget::createTrayMenu()
{
    m_trayMenu = new QMenu(this);
    
    // 设置托盘菜单的现代化样式
    m_trayMenu->setStyleSheet(
        "QMenu {"
        "    background: qlineargradient(x1:0, y1:0, x2:0, y2:1, "
        "                                stop:0 rgba(255, 255, 255, 250), "
        "                                stop:0.5 rgba(252, 254, 255, 245), "
        "                                stop:1 rgba(248, 251, 255, 240));"
        "    border: 2px solid rgba(170, 190, 220, 160);"
        "    border-radius: 12px;"
        "    padding: 10px 5px;"
        "    margin: 3px;"
        "    font-family: 'Microsoft YaHei UI', 'Segoe UI', 'SF Pro Display', Arial, sans-serif;"
        "    font-size: 13px;"
        "    font-weight: 500;"
        "    letter-spacing: 0.2px;"
        "}"
        "QMenu::item {"
        "    background: transparent;"
        "    color: rgba(40, 50, 65, 255);"
        "    padding: 10px 22px 10px 45px;"
        "    margin: 2px 6px;"
        "    border-radius: 8px;"
        "    border: 1px solid transparent;"
        "    min-width: 120px;"
        "    text-align: left;"
        "}"
        "QMenu::item:hover {"
        "    background: qlineargradient(x1:0, y1:0, x2:0, y2:1, "
        "                                stop:0 rgba(100, 150, 220, 120), "
        "                                stop:0.5 rgba(120, 170, 240, 100), "
        "                                stop:1 rgba(140, 190, 255, 80));"
        "    color: rgba(255, 255, 255, 255);"
        "    border: 1px solid rgba(80, 130, 200, 150);"
        "    font-weight: 600;"
        "    transform: translateY(-1px);"
        "}"
        "QMenu::item:pressed {"
        "    background: qlineargradient(x1:0, y1:0, x2:0, y2:1, "
        "                                stop:0 rgba(80, 130, 200, 150), "
        "                                stop:1 rgba(100, 150, 220, 130));"
        "    transform: translateY(0px);"
        "}"
        "QMenu::separator {"
        "    height: 2px;"
        "    background: qlineargradient(x1:0, y1:0, x2:1, y2:0, "
        "                                stop:0 transparent, "
        "                                stop:0.2 rgba(150, 170, 200, 100), "
        "                                stop:0.5 rgba(170, 190, 220, 140), "
        "                                stop:0.8 rgba(150, 170, 200, 100), "
        "                                stop:1 transparent);"
        "    margin: 6px 18px;"
        "    border-radius: 1px;"
        "}"
    );
    
    // 创建带图标的托盘菜单项
    m_showAction = new QAction(style()->standardIcon(QStyle::SP_MediaPlay), tr("显示"), this);
    connect(m_showAction, &QAction::triggered, this, &ClockWidget::showWindow);
    
    m_hideAction = new QAction(style()->standardIcon(QStyle::SP_MediaPause), tr("隐藏"), this);
    connect(m_hideAction, &QAction::triggered, this, &ClockWidget::hideWindow);
    
    m_settingsAction = new QAction(style()->standardIcon(QStyle::SP_FileDialogDetailedView), tr("设置"), this);
    connect(m_settingsAction, &QAction::triggered, [this]() {
        SettingsDialog dialog(this);
        dialog.exec();
    });
    
    m_exitAction = new QAction(style()->standardIcon(QStyle::SP_DialogCloseButton), tr("退出"), this);
    connect(m_exitAction, &QAction::triggered, qApp, &QApplication::quit);
    
    m_trayMenu->addAction(m_showAction);
    m_trayMenu->addAction(m_hideAction);
    m_trayMenu->addSeparator();
    m_trayMenu->addAction(m_settingsAction);
    m_trayMenu->addSeparator();
    m_trayMenu->addAction(m_exitAction);
    
    if (m_trayIcon) {
        m_trayIcon->setContextMenu(m_trayMenu);
    }
}

void ClockWidget::showWindow()
{
    show();
    raise();
    activateWindow();
}

void ClockWidget::hideWindow()
{
    hide();
}

void ClockWidget::toggleWindow()
{
    if (isVisible()) {
        hideWindow();
    } else {
        showWindow();
    }
}

void ClockWidget::onTrayIconActivated(QSystemTrayIcon::ActivationReason reason)
{
    switch (reason) {
    case QSystemTrayIcon::Trigger:
    case QSystemTrayIcon::DoubleClick:
        toggleWindow();
        break;
    default:
        break;
    }
}

void ClockWidget::closeEvent(QCloseEvent* event)
{
    if (m_trayIcon && m_trayIcon->isVisible()) {
        // 如果系统托盘可用，隐藏窗口而不是退出
        hide();
        event->ignore();
        
        // 首次隐藏时显示提示
        static bool firstHide = true;
        if (firstHide) {
            m_trayIcon->showMessage(tr("时钟小工具"), 
                                  tr("程序已最小化到系统托盘"), 
                                  QSystemTrayIcon::Information, 2000);
            firstHide = false;
        }
    } else {
        // 如果系统托盘不可用，正常退出
        event->accept();
    }
}

void ClockWidget::updateWeatherData()
{
    QUrl url("http://t.weather.itboy.net/api/weather/city/101210408");
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::UserAgentHeader, "ClockWidget/1.0");
    
    QNetworkReply* reply = m_networkManager->get(request);
    connect(reply, &QNetworkReply::finished, this, &ClockWidget::onWeatherDataReceived);
}

void ClockWidget::onWeatherDataReceived()
{
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply) {
        m_weatherText = QStringLiteral("回复对象为空");
        update();
        return;
    }
    
    if (reply->error() == QNetworkReply::NoError) {
        QByteArray data = reply->readAll();
        QJsonDocument doc = QJsonDocument::fromJson(data);
        
        if (!doc.isNull() && doc.isObject()) {
            QJsonObject root = doc.object();
            
            // 检查API响应状态
            int status = root.value("status").toInt();
            if (status != 200) {
                m_weatherText = QStringLiteral("API响应错误");
                update();
                reply->deleteLater();
                return;
            }
            
            QJsonObject cityInfo = root.value("cityInfo").toObject();
            QJsonObject data = root.value("data").toObject();
            
            if (!cityInfo.isEmpty() && !data.isEmpty()) {
                 QString city = cityInfo.value("city").toString();
                 QString wendu = data.value("wendu").toString();
                 QString ganmao = data.value("ganmao").toString();
                 QString shidu = data.value("shidu").toString();
                 
                 // 获取今日天气类型和温度范围、风力、风速
                 QJsonArray forecast = data.value("forecast").toArray();
                 QString weatherType = "";
                 QString tempRange = "";
                 QString windLevel = "";
                 QString windSpeed = "";
                 if (!forecast.isEmpty()) {
                     QJsonObject today = forecast[0].toObject();
                     weatherType = today.value("type").toString();
                     
                     // 获取温度范围
                     QString high = today.value("high").toString(); // 格式: "高温 25℃"
                     QString low = today.value("low").toString();   // 格式: "低温 15℃"
                     
                     // 提取数字部分
                     QString highTemp = high.split(" ").last().replace("℃", "");
                     QString lowTemp = low.split(" ").last().replace("℃", "");
                     tempRange = QString("%1~%2°C").arg(lowTemp).arg(highTemp);
                     
                     // 获取风力信息
                     QString fengli = today.value("fengli").toString(); // 格式: "<![CDATA[3-4级]]>"
                     QString fengxiang = today.value("fengxiang").toString(); // 风向
                     
                     // 清理风力数据
                     fengli = fengli.replace("<![CDATA[", "").replace("]]", "").replace(">", "");
                     windLevel = QString("%1 %2").arg(fengxiang).arg(fengli);
                     
                     // 获取风速信息（如果API提供）
                     if (today.contains("windspeed")) {
                         windSpeed = today.value("windspeed").toString() + "km/h";
                     } else if (today.contains("wind_speed")) {
                         windSpeed = today.value("wind_speed").toString() + "km/h";
                     } else if (today.contains("fengsu")) {
                         windSpeed = today.value("fengsu").toString() + "km/h";
                     }
                 }
                 
                 // 同时检查实时数据中的风速信息
                 if (windSpeed.isEmpty() && data.contains("windspeed")) {
                     windSpeed = data.value("windspeed").toString() + "km/h";
                 } else if (windSpeed.isEmpty() && data.contains("wind_speed")) {
                     windSpeed = data.value("wind_speed").toString() + "km/h";
                 } else if (windSpeed.isEmpty() && data.contains("fengsu")) {
                     windSpeed = data.value("fengsu").toString() + "km/h";
                 }
                 
                 m_cityName = city;
                 
                 // 更新天气显示格式，包含温度范围、风力和风速
                 QString windInfo = windLevel;
                 if (!windSpeed.isEmpty()) {
                     windInfo += " " + windSpeed;
                 }
                 
                 m_weatherText = QString("%1 %2 %3\n%4 湿度%5")
                     .arg(city)
                     .arg(tempRange.isEmpty() ? wendu + "°C" : tempRange)
                     .arg(weatherType)
                     .arg(windInfo)
                     .arg(shidu);
            } else {
                m_weatherText = QStringLiteral("天气数据格式错误");
            }
        } else {
            m_weatherText = QStringLiteral("JSON解析失败");
        }
    } else {
        m_weatherText = QString("网络错误: %1").arg(reply->errorString());
    }
    
    // 触发界面重绘
    update();
    reply->deleteLater();
}