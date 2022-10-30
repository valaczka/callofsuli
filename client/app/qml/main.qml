import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.14
import QtQuick.Window 2.15
import QtQuick.Controls.Material 2.3
import QtMultimedia 5.12
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS


ApplicationWindow {
    id: mainWindow
    visible: false
    width: 640
    height: 480

    title: (cosClient.connectionState !== Client.Standby && cosClient.serverName.length ? cosClient.serverName+" - " : "") + "Call of Suli"

    minimumWidth: 640
    minimumHeight: 480

    flags: Qt.Window | Qt.WindowTitleHint | Qt.WindowSystemMenuHint
           | (Qt.platform.os === "ios" ? Qt.MaximizeUsingFullscreenGeometryHint : 0)

    property alias actionFontPlus: fontPlus
    property alias actionFontMinus: fontMinus
    property alias actionFontReset: fontNormal

    property int safeMarginLeft: 0
    property int safeMarginRight: 0
    property int safeMarginTop: 0
    property int safeMarginBottom: 0


    FontLoader { source: "qrc:/internal/font/ariblk.ttf" }
    FontLoader { source: "qrc:/internal/font/Books.ttf" }
    FontLoader { source: "qrc:/internal/font/Material.ttf" }
    FontLoader { source: "qrc:/internal/font/School.ttf" }
    FontLoader { source: "qrc:/internal/font/Academic.ttf" }
    FontLoader { source: "qrc:/internal/font/AcademicI.ttf" }

    FontLoader { source: "qrc:/internal/font/rajdhani-bold.ttf" }
    FontLoader { source: "qrc:/internal/font/rajdhani-light.ttf" }
    FontLoader { source: "qrc:/internal/font/rajdhani-regular.ttf" }
    FontLoader { source: "qrc:/internal/font/rajdhani-medium.ttf" }
    FontLoader { source: "qrc:/internal/font/rajdhani-semibold.ttf" }

    FontLoader { source: "qrc:/internal/font/SpecialElite.ttf" }
    FontLoader { source: "qrc:/internal/font/HVD_Peace.ttf" }
    FontLoader { source: "qrc:/internal/font/RenegadeMaster.ttf" }



    background: Rectangle {
        color: "black"
    }

    Action {
        id: actionBack
        shortcut: "Esc"
        onTriggered: if (mainStack.depth > 1)
                         mainStack.get(1).stackBack()
    }


    StackView {
        id: mainStack
        objectName: "mainStack"
        anchors.fill: parent

        focus: true

        Keys.onReleased: {
            if (event.key === Qt.Key_Back) {
                back()
                event.accepted=true;
            }
        }


        initialItem: QCosImage {
            maxWidth: Math.min(mainWindow.width*0.7, 800)
            glowRadius: 6
        }


        Transition {
            id: transitionEnter

            PropertyAnimation {
                property: "opacity"
                from: 0.0
                to: 1.0
            }


        }



        Transition {
            id: transitionExit

            PropertyAnimation {
                property: "opacity"
                from: 1.0
                to: 0.0
            }
        }


        pushEnter: transitionEnter
        pushExit: transitionExit
        popEnter: transitionEnter
        popExit: transitionExit

        function back() {
            if (depth > 1) {
                if (!get(1).stackBack())
                    mainWindow.close()
            } else {
                mainWindow.close()
            }
        }

        function loginRequest() {
            if ((cosClient.connectionState === Client.Connected || cosClient.connectionState === Client.Reconnected)
                    && (cosClient.userRoles & Client.RoleGuest)) {
                JS.createPage("Login", {})
            }
        }

    }


    MouseArea {
        anchors.fill: parent
        acceptedButtons: Qt.NoButton
        onWheel: {
            if (wheel.modifiers & Qt.ControlModifier) {
                var i = wheel.angleDelta.y/120
                if (i>0)
                    fontPlus.trigger()
                else if (i<0)
                    fontMinus.trigger()

                wheel.accepted = true
            } else {
                wheel.accepted = false
            }
        }
    }


    Action {
        id: fontPlus
        shortcut: "Ctrl++"
        text: qsTr("Növelés")
        icon.source: "qrc:/internal/icon/magnify-plus.svg"
        enabled: CosStyle.pixelSize < 36
        onTriggered: CosStyle.pixelSize++
    }

    Action {
        id: fontMinus
        shortcut: "Ctrl+-"
        text: qsTr("Csökkentés")
        icon.source: "qrc:/internal/icon/magnify-minus.svg"
        enabled: CosStyle.pixelSize > 10
        onTriggered: CosStyle.pixelSize--
    }

    Action {
        id: fontNormal
        shortcut: "Ctrl+0"
        text: qsTr("Visszaállítás")
        icon.source: "qrc:/internal/icon/magnify-remove-outline.svg"
        onTriggered: CosStyle.pixelSize = 18
    }


    Action {
        id: actionFullscreen
        shortcut: "Ctrl+F11"
        onTriggered: {
            if (mainWindow.visibility === Window.FullScreen)
                mainWindow.showMaximized()
            else
                mainWindow.showFullScreen()
        }
    }

    onClosing: {
        if (mainStack.currentItem && mainStack.currentItem.closeCallbackFunction) {
            if (!mainStack.currentItem.closeCallbackFunction()) {
                close.accepted = true
                cosClient.windowSaveGeometry(mainWindow)
                cosClient.setSetting("window/fontSize", CosStyle.pixelSize)
                Qt.quit()
            } else {
                close.accepted = false
            }

        } else {
            close.accepted = true
            cosClient.windowSaveGeometry(mainWindow)
            cosClient.setSetting("window/fontSize", CosStyle.pixelSize)
            Qt.quit()
        }
    }



    Component.onCompleted: {
        cosClient.messageSent.connect(JS.dialogMessage)

        cosClient.windowSetIcon(mainWindow)
        cosClient.windowRestoreGeometry(mainWindow)

        var fs = cosClient.getSetting("window/fontSize", 0)
        if (fs > 0)
            CosStyle.pixelSize = fs

        timerOrientation.start()

        var g = cosClient.guiLoad()

        if (String(g).startsWith("map:")) {
            JS.createPage("MapEditor", {
                              fileToOpen: cosClient.urlFromLocalFile(String(g).substr(4))
                          })
        } else if (String(g).startsWith("play:")) {
            var f = String(g).substr(5)
            JS.createPage("Map", {
                              demoMode: true,
                              title: f,
                              readOnly: false,
                              fileToOpen: f
                          })
        } else
            JS.createPage("Start", {})
    }


    readonly property bool _isPortrait: (Screen.primaryOrientation === Qt.PortraitOrientation ||
                                                Screen.primaryOrientation === Qt.InvertedPortraitOrientation)

    on_IsPortraitChanged: timerOrientation.start()


    Timer {
        id: timerOrientation
        interval: 10
        running: false
        repeat: false

        onTriggered: {
            var m = cosClient.getWindowSafeMargins(mainWindow)
            safeMarginBottom = m.bottom
            safeMarginLeft = m.left
            safeMarginTop = m.top
            safeMarginRight = m.right
        }
    }
}
