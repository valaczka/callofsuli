import Bacon2D 1.0
import QtQuick 2.15
import COS.Client 1.0
import "Style"

AnimatedImage {
    id: root
    visible: enemyPrivate && enemyPrivate.player //&& enemyPrivate.player.enemy != enemyPrivate
    source: "qrc:/internal/game/sniper_crosshair.gif"

    property GameEnemyPrivate enemyPrivate: null
    property Item playerItem: null

    anchors.centerIn: playerItem ? playerItem : undefined

    /*onPlayerItemChanged: playerRectSet()

    Connections {
        target: enemyPrivate.player && playerItem ? playerItem : null
        function onXChanged(x) {
            playerRectSet()
        }

    }*/

    Connections {
        target: enemyPrivate
        function onPlayerChanged(player) {
            if (player)
                playerRectSet()
            else {
                root.destroy()
            }
        }
    }


    function playerRectSet() {
        /*if (playerItem) {
            var rx

            if (parent.facingLeft) {
                rx = playerItem.x+playerItem.width
                root.width = parent.x-rx
                root.x = rx-parent.x
            } else {
                rx = parent.x+parent.width
                root.width = playerItem.x-rx
                root.x = parent.width
            }
        }*/
    }
}
