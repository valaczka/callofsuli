import QtQuick 2.15
import QtQuick.Controls 2.15
import Bacon2D 1.0
import COS.Client 1.0
import QtMultimedia 5.12
import "."
import "Style"
import "JScript.js" as JS


Scene {
    id: scene
    //debug: true
    physics: true
    focus: true

    width: scenePrivate.implicitWidth
    height: scenePrivate.implicitHeight

    property alias scenePrivate: scenePrivate
    property bool showPickables: false
    property bool showTargets: false
    property bool isSceneZoom: false

    property alias playerLocatorComponent: playerLocatorComponent

    GameScenePrivate {
        id: scenePrivate

        onSceneLoaded: {
            game.currentScene = gameScene
            scene.forceActiveFocus()
        }
    }


    Connections {
        target: game
        function onGameStarted() {
            createLadders()
        }
    }


    PhysicsEntity {
        x: 0
        y: scene.height+5
        width: scene.width
        height: 10

        fixtures: Box {
            width: scene.width
            height: 10
            density: 1
            restitution: 0
            friction: 1
            categories: Box.Category1
            collidesWith: (Box.Category2|Box.Category5)
            readonly property bool baseGround: true
        }
    }


    MouseArea {
        id: area
        anchors.fill: parent
        hoverEnabled: true
    }

    Keys.onBackPressed: mainStack.back()

    Keys.onPressed: {
        if (game.player) {
            switch(event.key) {
            case Qt.Key_Left:
                if (event.modifiers & Qt.ShiftModifier)
                    game.player.walkLeft()
                else
                    game.player.runLeft()
                break;
            case Qt.Key_Right:
                if (event.modifiers & Qt.ShiftModifier)
                    game.player.walkRight()
                else
                    game.player.runRight()
                break;
            case Qt.Key_Up:
                game.player.moveUp()
                break;
            case Qt.Key_Down:
                game.player.moveDown()
                break;
            case Qt.Key_Space:
                if (!game.player.isOperating)
                    game.player.entityPrivate.attackByGun()
                break;
            case Qt.Key_Enter:
            case Qt.Key_Return:
                if (game.pickable) {
                    game.pickPickable()
                }
                break;
            case Qt.Key_F10:
                showPickables = true
                break
            case Qt.Key_F11:
                showTargets = true
                break
            }
        }

        event.accepted = true
    }

    Keys.onReleased: {
        if (game.player) {
            switch(event.key) {
            case Qt.Key_Left:
                if(!event.isAutoRepeat)
                    game.player.stopMoving();
                break;
            case Qt.Key_Right:
                if(!event.isAutoRepeat)
                    game.player.stopMoving();
                break;
            case Qt.Key_Up:
                if(!event.isAutoRepeat)
                    game.player.stopMoving();
                break;
            case Qt.Key_Down:
                if(!event.isAutoRepeat)
                    game.player.stopMoving();
                break;
            case Qt.Key_F10:
                showPickables = false
                break
            case Qt.Key_F11:
                showTargets = false
                break

            case Qt.Key_F3:				// Pinch zoom
                scenePrivate.game.gameSceneScaleToggleRequest()
                break

            case Qt.Key_W:				// Water
                if (game.player && game.player.entityPrivate.fire && game.gameMatch.water)
                    game.player.entityPrivate.operate(game.player.entityPrivate.fire)
                break

            case Qt.Key_P:				// Pliers
                if (game.player && game.player.entityPrivate.fence && game.gameMatch.pliers)
                    game.player.entityPrivate.operate(game.player.entityPrivate.fence)
                break

            case Qt.Key_I:				// Invisible
                if (game.player && game.gameMatch.glasses)
                    game.player.startInvisibility(10000)
                break
            }


            if (DEBUG_MODE) {
                switch(event.key) {

                case Qt.Key_N:
                    if (event.modifiers & (Qt.ShiftModifier|Qt.ControlModifier) && game.isStarted) {
                        game.gameCompleted()
                    }
                    break;
                case Qt.Key_X:
                    if (event.modifiers & Qt.ShiftModifier && area.containsMouse && game.player && game.isStarted) {
                        game.player.x = area.mouseX
                        game.player.y = area.mouseY
                        game.player.entityPrivate.ladderClimbFinish()
                    }
                    break;

                case Qt.Key_T:
                    if (event.modifiers & Qt.ShiftModifier && game.isStarted) {
                        game.addSecs(-30)
                    }
                    break;

                case Qt.Key_B:
                    if (event.modifiers & Qt.ShiftModifier && game.isStarted) {
                        game.player.entityPrivate.diedByBurn()
                    }
                    break;

                case Qt.Key_G:
                    if (event.modifiers & Qt.ShiftModifier && game.isStarted) {
                        game.increaseGlasses(1)
                    }
                    break;

                }
            }
        }

        event.accepted = true
    }





    Component {
        id: playerComponent
        GamePlayer { }
    }


    Component {
        id: ladderComponent
        GameLadder { }
    }

    Component {
        id: enemySoldierComponent
        GameEnemySoldier { }
    }

    Component {
        id: enemySniperComponent
        GameEnemySniper { }
    }


    Component {
        id: fireComponent
        GameFire {}
    }

    Component {
        id: fenceComponent
        GameFence {}
    }



    Component {
        id: pickableComponentHealth
        GamePickableHealth { }
    }


    Component {
        id: pickableComponentGeneral
        GamePickableGeneral { }
    }

    Component {
        id: playerLocatorComponent
        GamePlayerLocator { }
    }


    function createPlayer() : Item {
        if (!game.player) {
            var r = playerLocatorComponent.createObject(scene)
            var p = playerComponent.createObject(scene)
            r.anchors.centerIn = p
            p.loadSprites()
            return p
        }
            return null
        }


            function createComponent(enemyType: int) : Item {
                var obj = null

                switch (enemyType) {
                    case GameEnemyData.EnemySoldier:
                        obj = enemySoldierComponent.createObject(scene)
                    break
                    case GameEnemyData.EnemySniper:
                        obj = enemySniperComponent.createObject(scene)
                    break
                    case GameEnemyData.EnemyOther:
                        obj = enemySoldierComponent.createObject(scene)
                    break
                }

                return obj
            }


                function createLadders() {
                    if (!game || !game.ladderCount)
                    return

                    for (var i=0; i<game.ladderCount; i++) {
                        var l = game.ladderAt(i)

                        var obj = ladderComponent.createObject(scene,{
                            ladder: l
                        })
                    }
                }



                function createPickable(pickableType: int, pickableData) : Item {
                    if (!game)
                    return

                    var obj = null
                    var img = ""

                    switch (pickableType) {
                        case GamePickablePrivate.PickableHealth:
                            obj = pickableComponentHealth.createObject(scene, {
                                                                           cosGame: game,
                                                                           pickableData: pickableData
                                                                       })
                        break

                        case GamePickablePrivate.PickableTime:
                            if (pickableData.secs >= 60)
                                img = "qrc:/internal/game/time-60.png"
                            else if (pickableData.secs >= 30)
                                img = "qrc:/internal/game/time-30.png"

                        obj = pickableComponentGeneral.createObject(scene, {
                            cosGame: game,
                            type: GamePickablePrivate.PickableTime,
                            image: img,
                            pickableData: pickableData
                        })
                        break

                        case GamePickablePrivate.PickableShield:
                            if (pickableData.num >= 5)
                                img = "qrc:/internal/game/shield-gold.png"
                            else if (pickableData.num >= 3)
                                img = "qrc:/internal/game/shield-red.png"
                            else if (pickableData.num >= 2)
                                img = "qrc:/internal/game/shield-blue.png"
                            else
                                img = "qrc:/internal/game/shield-green.png"

                        obj = pickableComponentGeneral.createObject(scene, {
                            cosGame: game,
                            type: GamePickablePrivate.PickableShield,
                            image: img,
                            pickableData: pickableData
                        })
                        break

                        case GamePickablePrivate.PickablePliers:
                            obj = pickableComponentGeneral.createObject(scene, {
                                                                            cosGame: game,
                                                                            type: GamePickablePrivate.PickablePliers,
                                                                            image: "qrc:/internal/game/pliers.png",
                                                                            pickableData: pickableData
                                                                        })
                        break

                        case GamePickablePrivate.PickableWater:
                            obj = pickableComponentGeneral.createObject(scene, {
                                                                            cosGame: game,
                                                                            type: GamePickablePrivate.PickableWater,
                                                                            image: "qrc:/internal/game/water.svg",
                                                                            imageWidth: 30,
                                                                            imageHeight: 30,
                                                                            imageSourceWidth: 50,
                                                                            imageSourceHeight: 50,
                                                                            pickableData: pickableData
                                                                        })
                        break

                        case GamePickablePrivate.PickableGlasses:
                            obj = pickableComponentGeneral.createObject(scene, {
                                                                            cosGame: game,
                                                                            type: GamePickablePrivate.PickableGlasses,
                                                                            image: "qrc:/internal/game/glasses.png",
                                                                            pickableData: pickableData
                                                                        })
                        break
                    }

                    return obj
                }


                    function createFire() : Item {
                        var obj = fireComponent.createObject(scene)
                        return obj
                    }

                        function createFence() : Item {
                            var obj = fenceComponent.createObject(scene, {
                                cosGame: game
                            })
                            return obj
                        }
                        }
