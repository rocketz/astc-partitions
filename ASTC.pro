# Create xcode project like this:
# qmake -spec macx-xcode -o ASTC astc.pro
# VS:
# qmake -t vcapp astc.pro

# macdeployqt

TEMPLATE = app
INCLUDEPATH += \
	.
	
OBJECTS_DIR = build
UI_DIR = build
MOC_DIR = build
#CONFIG += x86_64
QT = core gui widgets

# Input
HEADERS += app.h	\
	common.h

SOURCES += \
	main.cpp	\
	astc.cpp	\
	app.cpp

mac {
	DEFINES += _TARGET_MAC_
}

win32 {
	DEFINES += _TARGET_WIN_ _CRT_SECURE_NO_WARNINGS _CRT_SECURE_NO_DEPRECATE
}

