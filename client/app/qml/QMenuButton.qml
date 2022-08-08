import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Controls.Material 2.3
import "Style"
import "JScript.js" as JS
import "."

QToolButton {
    id: button

    icon.source: CosStyle.iconMenu
    property QMenu menu: null

    visible: menu

    onClicked: {
        var p = button.mapToItem(ApplicationWindow.contentItem, button.x, button.y)
        var minP = Math.min(p.x, ApplicationWindow.contentItem.width-menu.width-ApplicationWindow.window.safeMarginRight)
        var mp = menu.parent.mapFromItem(ApplicationWindow.contentItem, minP, p.y+button.height)
        menu.x = mp.x
        menu.y = mp.y
        menu.open()
    }
}
