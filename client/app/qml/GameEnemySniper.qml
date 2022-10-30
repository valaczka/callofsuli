import Bacon2D 1.0
import QtQuick 2.15
import COS.Client 1.0
import QtGraphicalEffects 1.0
import QtMultimedia 5.12
import "Style"
import "JScript.js" as JS

GameEntity {
    id: root
    sleepingAllowed: false
    width: spriteSequence.width
    height: spriteSequence.height

    z: 9

    entityPrivate: ep

    property bool showPickable: false
    property bool showTarget: false
    property bool playerDiscovered: false

    glowColor: showPickable ? CosStyle.colorGlowItem : CosStyle.colorGlowEnemy
    glowEnabled: ep.aimedByPlayer || showPickable || showTarget

    overlayColor: JS.setColorAlpha(CosStyle.colorGlowEnemy, 0.8)

    hpColor: CosStyle.colorWarningLighter
    hpVisible: ep.aimedByPlayer
    hpValue: ep.hp


    GameEnemySniperPrivate {
        id: ep

        SoundEffect {
            id: shotEffect
            source: ep.shotSoundFile
            volume: cosClient.sfxVolume
        }

        onKilled: {
            spriteSequence.jumpTo("dead")
        }

        onMovingChanged: setSprite()

        onPlayerChanged: {
            setSprite()

            if (ep.player) {
                playerDiscovered = true
                var o = crosshairComponent.createObject(root.scene)
                o.playerItem = ep.player.parentEntity
            }
        }

        onAttack: {
            spriteSequence.jumpTo("shot")
            //cosClient.playSound(shotSoundFile, CosSound.EnemyShoot)
            shotEffect.play()
        }

        onRayCastPerformed: {
            setray(rect)
            /*if (cosGame.gameScene.debug)
                setray(rect)*/
        }

        Connections {
            target: ep.cosGame ? ep.cosGame.gameScene : null
            function onShowPickablesChanged() {
                if (ep.cosGame.gameScene.showPickables && ep.enemyData.pickableType !== GamePickablePrivate.PickableInvalid)
                    showPickable = true
                else
                    showPickable = false
            }

            function onShowTargetsChanged() {
                if (ep.cosGame.gameScene.showTargets && ep.enemyData.targetId != -1)
                    showTarget = true
                else
                    showTarget = false
            }

            function onIsSceneZoomChanged() {
                overlayEnabled = ep.cosGame.gameScene.isSceneZoom
            }
        }
    }

    Component {
        id: crosshairComponent

        GameEnemySniperCrosshair {
            enemyPrivate: ep
            z: 16
        }
    }



    function setray(rect) {
        var k = mapFromItem(scene, rect.x, rect.y)

        rayRect.x = k.x + (facingLeft ? 0 : width)
        rayRect.width = rect.width - width
        rayRect.y = k.y
        rayRect.height = Math.max(rect.height, 1)
        //rayRect.visible = true
        //timerOff.start()

    }

    Rectangle {
        id: rayRect
        color: CosStyle.colorOKLighter
        visible: ep.isAlive && !ep.player && playerDiscovered
        border.width: 1
        border.color: rayRect.color
        x: 0
        y: 0
        width: 1
        height: 1

        SequentialAnimation {
            running: true
            loops: Animation.Infinite
            ColorAnimation {
                target: rayRect
                property: "color"
                from: "transparent"
                to: CosStyle.colorOKLighter
                duration: 175
                easing.type: Easing.InQuad
            }

            ColorAnimation {
                target: rayRect
                property: "color"
                from: CosStyle.colorOKLighter
                to: "transparent"
                duration: 175
                easing.type: Easing.OutQuad
            }
        }

        /*Timer {
            id: timerOff
            interval: 200
            triggeredOnStart: false
            running: false
            repeat: false
            onTriggered: rayRect.visible = false
        }*/
    }



    Timer {
        interval: 100
        running: ep.isAlive && !ep.player
        repeat: true

        property int elapsed: 0

        onTriggered: {
            elapsed += interval

            if (elapsed >= ep.msecBeforeTurn) {
                elapsed = 0
                facingLeft = !facingLeft
            }
        }
    }


    function setSprite() {
        if (!ep.isAlive)
            return

        //if (ep.player) {
            spriteSequence.jumpTo("idle")
        /*} else {
            if (!ep.atBound && ep.moving) {
                spriteSequence.jumpTo("walk")
            }
            else {
                spriteSequence.jumpTo("idle")
            }
        }*/
    }

}

