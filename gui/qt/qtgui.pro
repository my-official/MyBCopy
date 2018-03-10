QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = MyBCopy
TEMPLATE = app


SOURCES += RegexpFilterEditor.cpp \    
    WildcardFilterEditor.cpp \    
    DepthEditor.cpp \
    StorageEditorDialog.cpp \
    BackupEditor.cpp \
    MainWindow.cpp \
    SettingsEditor.cpp \
    StorageFrame.cpp \
    ToolChainFrame.cpp \
    SchemeFrame.cpp \
    ToolEditorDialog.cpp \
    ProgressPage.cpp

HEADERS  += RegexpFilterEditor.h \    
    WildcardFilterEditor.h \    
    DepthEditor.h \
    StorageEditorDialog.h \
    BackupEditor.h \
    MainWindow.h \
    SettingsEditor.h \
    StorageFrame.h \
    ToolChainFrame.h \
    SchemeFrame.h \
    ToolEditorDialog.h \
    ProgressPage.h

FORMS    += RegexpFilterEditor.ui \    
    WildcardFilterEditor.ui \    
    DepthEditor.ui \
    StorageEditorDialog.ui \
    BackupEditor.ui \
    MainWindow.ui \
    SettingsEditor.ui \
    ToolChainFrame.ui \
    SchemeFrame.ui \
    StorageFrame.ui \
    ToolEditorDialog.ui \
    ProgressViewer.ui

RESOURCES += traversabledelegate.qrc \
    backupmodel.qrc \
    backupeditor.qrc \
    progressviewer.qrc \
    main.qrc
