QT += widgets network svg
CONFIG += c++11
TEMPLATE = app
TARGET = ClockWidget

# 应用程序图标
RC_ICONS = src/clock-icon.ico

SOURCES += \
    src/main.cpp \
    src/ClockWidget.cpp \
    src/SettingsDialog.cpp

HEADERS += \
    src/ClockWidget.h \
    src/SettingsDialog.h