import Bacon2D 1.0
import QtQuick 2.15
import QtGraphicalEffects 1.0
import COS.Client 1.0
import "Style"

PhysicsEntity {
    id: root
    sleepingAllowed: true

    z: 4

    width: img.width
    height: img.height+elevation

    opacity: 0.7

    bodyType: Body.Static

    transformOrigin: Item.Center

    property bool glowEnabled: false
    readonly property int elevation: 5

    fixtures: [
        Box {
            width: root.width
            height: root.height
            x: 0
            y: 0
            sensor: false
            //collidesWith: Box.Category3
            categories: Box.Category4

            readonly property PhysicsEntity targetObject: root
            readonly property var targetData: {"teleport": true}

            onBeginContact: glowEnabled = true
            onEndContact: glowEnabled = false
        }
    ]


    Glow {
        id: glow
        opacity: glowEnabled ? 1.0 : 0.0
        visible: opacity != 0

        color: CosStyle.colorGlowItem
        source: img
        anchors.fill: img

        radius: 5
        samples: 5

        Behavior on opacity {
            NumberAnimation { duration: 200 }
        }
    }

    AnimatedImage {
        id: img
        anchors.top: parent.top
        anchors.horizontalCenter: parent.horizontalCenter
        source: "qrc:/internal/game/teleport.gif"
        width: 100
        height: 86
        //speed: 0.75
        fillMode: Image.PreserveAspectFit
    }

}
