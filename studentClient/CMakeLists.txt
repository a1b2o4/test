cmake_minimum_required(VERSION 3.5)

project(diplom1 VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets Network)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Network)

set(PROJECT_SOURCES
        main.cpp
        mainwindow.cpp
        mainwindow.h
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(diplom1
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
        studentinfo.h studentinfo.cpp
        studentinputdialog.h studentinputdialog.cpp
        elementselector.h elementselector.cpp
        switchingelement.h switchingelement.cpp
        qswitchingelement.h qswitchingelement.cpp
        port.h port.cpp
        qport.h qport.cpp
        connection.h connection.cpp
        qconnection.h qconnection.cpp
        ioport.h ioport.cpp
        qioport.h qioport.cpp
        diagramscene.h diagramscene.cpp
        elementbutton.h elementbutton.cpp
        propertiestable.h propertiestable.cpp
        diagramitem.h diagramitem.cpp
        variantinfodialog.h variantinfodialog.cpp
        taskinfo.h taskinfo.cpp
        variantinfo.h variantinfo.cpp
        topology.h topology.cpp
        banyan.h banyan.cpp
        scheme.h scheme.cpp
        omega.h omega.cpp
        delta.h delta.cpp
        closa.h closa.cpp
        startschemeinputdialog.h startschemeinputdialog.cpp
        prioritysettingsdialog.h prioritysettingsdialog.cpp
        testdialog.h testdialog.cpp
        newgraphicsview.h newgraphicsview.cpp
        schememode.h schememode.cpp
    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET diplom1 APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        add_library(diplom1 SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(diplom1
            ${PROJECT_SOURCES}
        )
    endif()
endif()

#target_link_libraries(diplom1 PRIVATE Qt${QT_VERSION_MAJOR}::Widgets::Network)
target_link_libraries(diplom1 PRIVATE Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::Network)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.diplom1)
endif()
set_target_properties(diplom1 PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS diplom1
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(diplom1)
endif()
