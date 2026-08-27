import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import CallOfSuli
import SortFilterProxyModel
import Qaterial as Qaterial
import "./QaterialHelper" as Qaterial
import "JScript.js" as JS

QItemGradient {
	id: root

	property RpgGame game: null
	property Item parentPage: null

	readonly property bool _isEmpty: game && game.isEmpty

	title: !_isEmpty && game ? game.name + qsTr(" – level %1").arg(game.level): ""

	appBar.rightComponent: Row {
		id: _rowToken

		spacing: 5

		Qaterial.Icon {
			color: Qaterial.Colors.blue500
			icon: Qaterial.Icons.shieldCrown
			anchors.verticalCenter: parent.verticalCenter
		}

		Qaterial.LabelHeadline6 {
			text: num

			anchors.verticalCenter: parent.verticalCenter
			rightPadding: Qaterial.Style.horizontalPadding

			color: Qaterial.Colors.blue500

			property int num: game ? game.rpgUserData.token : 0

			Behavior on num {
				NumberAnimation {
					duration: 250
					easing.type: Easing.InOutQuad
				}
			}
		}

		Component.onCompleted: {
			if (!_tour.list[1].target)
				_tour.list[1].target = _rowToken
		}
	}

	Qaterial.BusyIndicator {
		id: _busyIndicator
		anchors.centerIn: parent
		visible: false
	}

	Item {
		id: _content

		property real horizontalPadding: Qaterial.Style.horizontalPadding
		property real verticalPadding: Qaterial.Style.horizontalPadding

		anchors.leftMargin: Math.max(horizontalPadding, Client.safeMarginLeft)
		anchors.rightMargin: Math.max(horizontalPadding, Client.safeMarginRight)
		anchors.topMargin: Math.max(verticalPadding, Client.safeMarginTop, root.paddingTop)
		anchors.bottomMargin: Math.max(verticalPadding, Client.safeMarginBottom)

		anchors.fill: parent

		Qaterial.Card {
			outlined: true

			width: Math.min(parent.width, Math.max(_viewCharacters.contentWidth, Qaterial.Style.maxContainerSize))
			height: Math.min(parent.height, _isEmpty ? 800 : 500)

			anchors.centerIn: parent

			contentItem: GridLayout {
				id: _grid1

				columns: 2//width > height ? 3 : 2
				columnSpacing: 10
				rowSpacing: 10
				width: parent.width - 2 * Qaterial.Style.card.horizontalPadding
				height: parent.height - 2 * Qaterial.Style.card.verticalPadding


				RpgSelectTitle {
					Layout.fillHeight: false
					Layout.fillWidth: true
					Layout.columnSpan: 2

					icon.source: Qaterial.Icons.accountMultipleOutline
					text: qsTr("Characters")
				}

				RpgSelectView {
					id: _viewCharacters

					Layout.fillHeight: true
					Layout.maximumHeight: 200
					Layout.fillWidth: true
					Layout.columnSpan: 2


					delegate: RpgSelectCard {
						id: _selectPlayer

						width: ListView.view.height
						height: ListView.view.height

						readonly property bool isTarget: character === game.rpgUserData.target

						text: name
						image: game ? game.getCharacterImage(character) : ""
						selected: _viewCharacters.selected == character

						locked: level < 1
						iconLockColor: isTarget ? Qaterial.Colors.cyan800 : Qaterial.Style.disabledTextColor()

						borderVisible: true

						Qaterial.IconLabel {
							id: _labelLevel

							text: level
							icon.source: Qaterial.Icons.flash

							color: Qaterial.Colors.amber500

							anchors.left: parent.left
							anchors.bottom: parent.bottom
							anchors.leftMargin: 10 * Qaterial.Style.pixelSizeRatio
							anchors.bottomMargin: 10 * Qaterial.Style.pixelSizeRatio

							icon.width: 12 * Qaterial.Style.pixelSizeRatio
							icon.height: 12 * Qaterial.Style.pixelSizeRatio
							spacing: 0
							visible: level > 0
						}

						ProgressBar {
							id: _progress
							visible: ((_isEmpty || _selectPlayer.selected) && nextPoint > 0 && level > 0) || isTarget
							anchors.verticalCenter: _labelLevel.top
							anchors.verticalCenterOffset: 3
							anchors.left: _labelLevel.visible ? _labelLevel.right : parent.left
							anchors.right: parent.right
							anchors.leftMargin: 10 * Qaterial.Style.pixelSizeRatio
							anchors.rightMargin: 10 * Qaterial.Style.pixelSizeRatio

							from: 0
							to: level == 0 ? unlock : nextPoint
							value: level == 0 ? game.rpgUserData.token : point

							Material.accent: color

							property color color: isTarget ? Qaterial.Style.iconColor() : Qaterial.Style.accentColor

							Behavior on value {
								NumberAnimation { duration: 175; easing.type: Easing.InOutQuad }
							}
						}

						Qaterial.LabelCaption {
							visible: ((_isEmpty || _selectPlayer.selected) && nextPoint > 0 && level > 0) || isTarget
							text: isTarget ? game.rpgUserData.token : point
							anchors.left: _progress.left
							anchors.top: _progress.bottom
							color: isTarget ? Qaterial.Style.iconColor() : Qaterial.Style.accentColor
						}

						Qaterial.LabelCaption {
							visible: ((_isEmpty || _selectPlayer.selected) && nextPoint > 0 && level > 0) || isTarget
							text: isTarget ? unlock : nextPoint
							anchors.right: _progress.right
							anchors.top: _progress.bottom
							color: isTarget ? Qaterial.Style.iconColor() : Qaterial.Style.accentColor
						}

						Qaterial.AppBarButton {
							id: _btnPreview

							anchors.right: parent.right
							anchors.top: parent.top
							anchors.rightMargin: 5 * Qaterial.Style.pixelSizeRatio
							anchors.topMargin: Math.max(5 * Qaterial.Style.pixelSizeRatio, _selectPlayer.labelHeight)
							icon.source: Qaterial.Icons.eye
							icon.color: Qaterial.Colors.green500
							visible: level <= 0 && _isEmpty

							ToolTip.text: qsTr("Preview")

							onClicked: {
								game.loadTutorial(character)
							}

							Component.onCompleted: {
								if (!_tour.list[3].target)
									_tour.list[3].target = _btnPreview
							}

						}


						onClicked: {
							if (locked && !_isEmpty)
								return;

							_viewCharacters.currentIndex = index
							_viewCharacters.selected = character

							if (!_isEmpty)
								game.characterSelect({character: character})
						}
					}

					model: _modelCharacters
				}

				Item {
					id: _preview

					visible: _isEmpty

					readonly property var character: _viewCharacters.currentIndex != -1 ?
														 _modelCharacters.get(_viewCharacters.currentIndex) :
														 null

					property int levelCurrent: 1
					property int levelMin: Math.max(1, character ? character.level : 1)
					readonly property int levelMax: game.getCharactersMetric().maxLevel

					onLevelMinChanged: if (levelCurrent < levelMin) levelCurrent = levelMin

					Layout.fillHeight: true
					Layout.fillWidth: true

					implicitHeight: 50
					implicitWidth: 50

					Qaterial.LabelHeadline5 {
						id: _labelCharacter
						width: parent.width
						wrapMode: Text.Wrap
						anchors.top: parent.top
						bottomPadding: 5
						horizontalAlignment: Text.AlignHCenter
						text: _preview.character.name
					}

					Row {
						id: _rowLevel
						anchors.horizontalCenter: parent.horizontalCenter
						anchors.top: _labelCharacter.bottom
						spacing: 8

						Qaterial.ToolButton {
							checkable: false
							useSecondaryColor: false

							anchors.verticalCenter: parent.verticalCenter

							icon.source: Qaterial.Icons.minus

							enabled: _preview.levelCurrent > _preview.levelMin

							onClicked: --_preview.levelCurrent
						}

						Qaterial.IconLabel {
							anchors.verticalCenter: parent.verticalCenter
							color: Qaterial.Colors.amber500
							spacing: 0

							icon.source: Qaterial.Icons.flash
							text: _preview.levelCurrent
						}


						Qaterial.ToolButton {
							checkable: false
							useSecondaryColor: false

							anchors.verticalCenter: parent.verticalCenter

							icon.source: Qaterial.Icons.plus

							enabled: _preview.levelCurrent < _preview.levelMax

							onClicked: ++_preview.levelCurrent
						}
					}

					QScrollable {
						anchors.left: parent.left
						anchors.bottom: parent.bottom
						anchors.right: parent.right
						anchors.top: _rowLevel.bottom
						anchors.topMargin: 5

						contentCentered: true


						Row {
							visible: _preview.character && _preview.character.level == 0
							anchors.horizontalCenter: parent.horizontalCenter

							bottomPadding: 20
							spacing: 10

							Qaterial.LabelBody1 {
								anchors.verticalCenter: parent.verticalCenter
								text: qsTr("Unlock:")
								color: Qaterial.Colors.blue500
							}

							Qaterial.IconLabel {
								color: Qaterial.Colors.blue500
								icon.source: Qaterial.Icons.shieldCrown
								icon.width: 20 * Qaterial.Style.pixelSizeRatio
								icon.height: 20 * Qaterial.Style.pixelSizeRatio
								anchors.verticalCenter: parent.verticalCenter
								spacing: 3
								text: _preview.character ? _preview.character.unlock : 0
							}
						}

						GridLayout {
							id: _metricGrid
							width: Math.min(parent.width, 300)
							anchors.horizontalCenter: parent.horizontalCenter

							columns: 3
							columnSpacing: 10
							rowSpacing: 0

							readonly property bool _wide: width > 250
							readonly property var _metric: game.getCharactersMetric()
							readonly property var _characterMetric: game.getCharacterMetricAtLevel(_preview.character.character, _preview.levelCurrent)


							// Hp

							Qaterial.IconLabel {
								Layout.fillWidth: false
								Layout.fillHeight: false
								Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
								display: _metricGrid._wide ? IconLabel.Display.TextBesideIcon : IconLabel.Display.IconOnly
								spacing: 5
								icon.source: Qaterial.Icons.heartPulse
								horizontalAlignment: Qt.AlignLeft
								text: qsTr("HP")
							}

							ProgressBar {
								Layout.fillWidth: true
								Layout.fillHeight: false
								Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter

								from: 0//_metricGrid._metric.min.hp
								to: _metricGrid._metric.max.hp
								value: _metricGrid._characterMetric.hp

								Material.accent: color

								property color color: Qaterial.Style.iconColor()

								Behavior on value {
									NumberAnimation { duration: 175; easing.type: Easing.InOutQuad }
								}
							}

							Qaterial.LabelHint1 {
								Layout.fillWidth: false
								Layout.fillHeight: false
								Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
								horizontalAlignment: Text.AlignRight

								text: value

								property int value: _metricGrid._characterMetric.hp

								Behavior on value {
									NumberAnimation { duration: 175; easing.type: Easing.InOutQuad }
								}
							}


							// Mp

							Qaterial.IconLabel {
								Layout.fillWidth: false
								Layout.fillHeight: false
								Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
								display: _metricGrid._wide ? IconLabel.Display.TextBesideIcon : IconLabel.Display.IconOnly
								spacing: 5
								icon.source: Qaterial.Icons.shimmer
								horizontalAlignment: Qt.AlignLeft
								text: qsTr("MP")
							}

							ProgressBar {
								Layout.fillWidth: true
								Layout.fillHeight: false
								Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter

								from: 0//_metricGrid._metric.min.mp
								to: _metricGrid._metric.max.mp
								value: _metricGrid._characterMetric.mp

								Material.accent: color

								property color color: Qaterial.Style.iconColor()

								Behavior on value {
									NumberAnimation { duration: 175; easing.type: Easing.InOutQuad }
								}
							}

							Qaterial.LabelHint1 {
								Layout.fillWidth: false
								Layout.fillHeight: false
								Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
								horizontalAlignment: Text.AlignRight

								text: value

								property int value: _metricGrid._characterMetric.mp

								Behavior on value {
									NumberAnimation { duration: 175; easing.type: Easing.InOutQuad }
								}
							}





							// Bullet

							Qaterial.IconLabel {
								Layout.fillWidth: false
								Layout.fillHeight: false
								Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
								display: _metricGrid._wide ? IconLabel.Display.TextBesideIcon : IconLabel.Display.IconOnly
								spacing: 5
								icon.source: Qaterial.Icons.bullet
								horizontalAlignment: Qt.AlignLeft
								text: qsTr("Bullet")
							}

							ProgressBar {
								Layout.fillWidth: true
								Layout.fillHeight: false
								Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter

								from: 0//_metricGrid._metric.min.bullet
								to: _metricGrid._metric.max.bullet
								value: _metricGrid._characterMetric.bullet

								Material.accent: color

								property color color: Qaterial.Style.iconColor()

								Behavior on value {
									NumberAnimation { duration: 175; easing.type: Easing.InOutQuad }
								}
							}

							Qaterial.LabelHint1 {
								Layout.fillWidth: false
								Layout.fillHeight: false
								Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
								horizontalAlignment: Text.AlignRight

								text: value

								property int value: _metricGrid._characterMetric.bullet

								Behavior on value {
									NumberAnimation { duration: 175; easing.type: Easing.InOutQuad }
								}
							}




							// Tower plus

							Qaterial.IconLabel {
								Layout.fillWidth: false
								Layout.fillHeight: false
								Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
								display: _metricGrid._wide ? IconLabel.Display.TextBesideIcon : IconLabel.Display.IconOnly
								spacing: 5
								icon.source: Qaterial.Icons.plus
								horizontalAlignment: Qt.AlignLeft
								text: qsTr("PWR+")
							}

							ProgressBar {
								Layout.fillWidth: true
								Layout.fillHeight: false
								Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter

								from: 0//_metricGrid._metric.min.mp
								to: _metricGrid._metric.max.towerPlus
								value: _metricGrid._characterMetric.towerPlus

								Material.accent: color

								property color color: Qaterial.Style.iconColor()

								Behavior on value {
									NumberAnimation { duration: 175; easing.type: Easing.InOutQuad }
								}
							}

							Qaterial.LabelHint1 {
								Layout.fillWidth: false
								Layout.fillHeight: false
								Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
								horizontalAlignment: Text.AlignRight

								text: value

								property int value: _metricGrid._characterMetric.towerPlus

								Behavior on value {
									NumberAnimation { duration: 175; easing.type: Easing.InOutQuad }
								}
							}



							// Tower minus

							Qaterial.IconLabel {
								Layout.fillWidth: false
								Layout.fillHeight: false
								Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
								display: _metricGrid._wide ? IconLabel.Display.TextBesideIcon : IconLabel.Display.IconOnly
								spacing: 5
								icon.source: Qaterial.Icons.minus
								horizontalAlignment: Qt.AlignLeft
								text: qsTr("PWR-")
							}

							ProgressBar {
								Layout.fillWidth: true
								Layout.fillHeight: false
								Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter

								from: 0//_metricGrid._metric.min.mp
								to: _metricGrid._metric.max.towerMinus
								value: _metricGrid._characterMetric.towerMinus

								Material.accent: color

								property color color: Qaterial.Style.iconColor()

								Behavior on value {
									NumberAnimation { duration: 175; easing.type: Easing.InOutQuad }
								}
							}

							Qaterial.LabelHint1 {
								Layout.fillWidth: false
								Layout.fillHeight: false
								Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
								horizontalAlignment: Text.AlignRight

								text: value

								property int value: _metricGrid._characterMetric.towerMinus

								Behavior on value {
									NumberAnimation { duration: 175; easing.type: Easing.InOutQuad }
								}
							}



							// Penalty

							Qaterial.IconLabel {
								Layout.fillWidth: false
								Layout.fillHeight: false
								Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
								display: _metricGrid._wide ? IconLabel.Display.TextBesideIcon : IconLabel.Display.IconOnly
								spacing: 5
								icon.source: Qaterial.Icons.timerAlert
								horizontalAlignment: Qt.AlignLeft
								text: qsTr("Penalty")
							}

							ProgressBar {
								Layout.fillWidth: true
								Layout.fillHeight: false
								Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter

								from: 0//_metricGrid._metric.min.mp
								to: _metricGrid._metric.max.penalty
								value: _metricGrid._characterMetric.penalty

								Material.accent: color

								property color color: Qaterial.Style.iconColor()

								Behavior on value {
									NumberAnimation { duration: 175; easing.type: Easing.InOutQuad }
								}
							}

							Qaterial.LabelHint1 {
								Layout.fillWidth: false
								Layout.fillHeight: false
								Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
								horizontalAlignment: Text.AlignRight

								text: value

								property int value: _metricGrid._characterMetric.penalty

								Behavior on value {
									NumberAnimation { duration: 175; easing.type: Easing.InOutQuad }
								}
							}



							// Skip

							Qaterial.IconLabel {
								Layout.fillWidth: false
								Layout.fillHeight: false
								Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
								display: _metricGrid._wide ? IconLabel.Display.TextBesideIcon : IconLabel.Display.IconOnly
								spacing: 5
								icon.source: Qaterial.Icons.skipForward
								horizontalAlignment: Qt.AlignLeft
								text: qsTr("Skip")
							}

							ProgressBar {
								Layout.fillWidth: true
								Layout.fillHeight: false
								Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter

								from: 0//_metricGrid._metric.min.mp
								to: _metricGrid._metric.max.skipLock
								value: _metricGrid._characterMetric.skipLock

								Material.accent: color

								property color color: Qaterial.Style.iconColor()

								Behavior on value {
									NumberAnimation { duration: 175; easing.type: Easing.InOutQuad }
								}
							}

							Qaterial.LabelHint1 {
								Layout.fillWidth: false
								Layout.fillHeight: false
								Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
								horizontalAlignment: Text.AlignRight

								text: value

								property int value: _metricGrid._characterMetric.skipLock

								Behavior on value {
									NumberAnimation { duration: 175; easing.type: Easing.InOutQuad }
								}
							}
						}

						Column {
							topPadding: 10
							anchors.horizontalCenter: parent.horizontalCenter

							Repeater {
								model: _metricGrid._characterMetric.defenders
								delegate: Row {
									spacing: 10
									Qaterial.IconLabel {
										spacing: 5

										color: Qaterial.Colors.green500
										anchors.verticalCenter: parent.verticalCenter
										icon.source: modelData.icon
										text: modelData.description
									}

									Qaterial.ToolButton {
										checkable: false
										anchors.verticalCenter: parent.verticalCenter
										icon.source: Qaterial.Icons.helpCircleOutline
										onClicked: {
											Qaterial.DialogManager.showDialog(
														{
															text: modelData.helper,
															title: modelData.description,
															iconSource: modelData.icon,
															iconColor: Qaterial.Style.iconColor(),
															iconFill: false,
															iconSize: Qaterial.Style.roundIcon.size,
															standardButtons: DialogButtonBox.Ok
														})
										}
									}
								}
							}

							Item {
								width: parent.width
								height: 10
							}


							Repeater {
								model: _metricGrid._characterMetric.utilities
								delegate: Row {
									spacing: 10
									Qaterial.IconLabel {
										spacing: 5

										color: Qaterial.Colors.amber500
										anchors.verticalCenter: parent.verticalCenter
										icon.source: modelData.icon
										text: modelData.description
									}

									Qaterial.ToolButton {
										checkable: false
										anchors.verticalCenter: parent.verticalCenter
										icon.source: Qaterial.Icons.helpCircleOutline
										onClicked: {
											Qaterial.DialogManager.showDialog(
														{
															text: modelData.helper,
															title: modelData.description,
															iconSource: modelData.icon,
															iconColor: Qaterial.Style.iconColor(),
															iconFill: false,
															iconSize: Qaterial.Style.roundIcon.size,
															standardButtons: DialogButtonBox.Ok
														})
										}
									}
								}
							}
						}

						/*Qaterial.LabelBody2 {
							width: Math.min(implicitWidth, parent.width)
							wrapMode: Text.Wrap

							text: JSON.stringify(game.getCharactersMetric()) + "\n"
								  + (_preview.character ? _preview.character.character : "---") + "\n"
								  + (_preview.character ? _preview.character.level : "---") + "\n"
								  + JSON.stringify(_preview.character) + "\n"
								  + JSON.stringify(game.getCharacterMetricAtLevel(_preview.character.character, _preview.levelCurrent))
						}*/
					}
				}

				Column {
					id: _world

					visible: !_isEmpty

					Layout.fillHeight: true
					Layout.fillWidth: false
					Layout.preferredWidth: _selectTerrain.implicitWidth
					Layout.preferredHeight: _selectTerrain.implicitHeight
					Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
					Layout.leftMargin: Qaterial.Style.card.horizontalPadding

					RpgSelectTitle {
						id: _worldTitle
						anchors.horizontalCenter: parent.horizontalCenter
						icon.source: Qaterial.Icons.earth
						text: qsTr("World")
					}

					RpgSelectCard {
						id: _selectTerrain

						anchors.horizontalCenter: parent.horizontalCenter

						readonly property RpgWorldLandData _selected: Client.world ? Client.world.selectedLand : null

						readonly property real _size: Math.min(parent.width, parent.height-_worldTitle.height)

						width: _size
						height: _size

						fullBg: true

						text: _selected ? _selected.name :
										  enabled ? qsTr("World...") : ""
						image: _selected ? _selected.backgroundSource : ""

						//selected: enabled
						enabled: game /*&& (game.gameMode == RpgGame.SinglePlayer ||
										  _multiplayer.gameMode == ActionRpgGame.MultiPlayerHost*/
						onClicked: {
							Client.stackPushPage("RpgWorldSelect.qml", {
													 world: Client.world
												 })
						}

					}
				}


				Column {
					Layout.fillHeight: false
					Layout.fillWidth: !_isEmpty
					Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter

					spacing: 5

					QButton {
						id: _btnTarget

						anchors.horizontalCenter: parent.horizontalCenter

						visible: _isEmpty

						icon.source: Qaterial.Icons.bullseyeArrow
						text: qsTr("Target")

						enabled: _preview.character && _preview.character.level <= 0

						onClicked: {
							Client.send(HttpConnection.ApiUser, "rpg/target/%1".arg(_preview.character.character))
							.done(root, function(r){
								game.reloadRpgData()
							})
							.fail(root, JS.failMessage(qsTr("Karakter kiválasztása sikertelen")))
						}
					}


					QButton {
						id: _btnBuy

						anchors.horizontalCenter: parent.horizontalCenter

						text: qsTr("Vásárlás")
						icon.source: Qaterial.Icons.crownCircle
						enabled: _preview.character && _preview.character.level == 0 && game.rpgUserData.token >= _preview.character.unlock

						visible: _isEmpty

						onClicked: {
							if (_isEmpty) {
								Client.send(HttpConnection.ApiUser, "rpg/buy/%1".arg(_preview.character.character))
								.done(root, function(r){
									game.reloadRpgData()
								})
								.fail(root, JS.failMessage(qsTr("Karakter kiválasztása sikertelen")))

							} else
								return;

						}
					}


					QButton {
						id: _btnTutorial

						anchors.horizontalCenter: parent.horizontalCenter

						icon.source: _isEmpty ? Qaterial.Icons.eye : Qaterial.Icons.play
						text: _isEmpty ? qsTr("Tutorial") : qsTr("Play")

						bgColor: Qaterial.Colors.green500

						topPadding: 15
						bottomPadding: 15
						//leftPadding: 10
						//rightPadding: 10

						enabled: _isEmpty || _viewCharacters.selected != ""

						onClicked: _isEmpty ? game.loadTutorial("")
											: game.characterSelect({
																	   character: _viewCharacters.selected,
																	   terrain: "test",
																	   ready: true
																   })

						Component.onCompleted: {
							if (!_tour.list[0].target && _isEmpty)
								_tour.list[0].target = _btnTutorial
						}
					}
				}

				/*Row {
					spacing: 5
					Layout.fillHeight: false
					Layout.fillWidth: false
					Layout.columnSpan: 2

					QButton {
						text: "DROP"

						visible: game.rpgUserData.drops.length > 0

						onClicked: Client.stackPushPage("PageRpgDrop.qml", {
															game: root.game
														})
					}
				}*/

			}


		}
	}


	Timer {
		id: _timerOldCurrency
		interval: 3000
		repeat: false
		triggeredOnStart: false

		onTriggered: {
			if (root.parentPage && root.parentPage.StackView.status == StackView.Active)
				Qaterial.DialogManager.showDialog(
							{
								text: qsTr("A megújult akciójátékba áthozzuk az eddigi pénzedet: %1\nVálaszd ki, melyik karakterek között akarod egyenlő mértékben szétosztani.\nEzt a lépést későbbre is halaszthatod.").arg(game.rpgUserData.oldCurrency),
								title: qsTr("Konvertálás az új játékra"),
								iconSource: Qaterial.Icons.cash100,
								iconColor: Qaterial.Style.accentColor,
								textColor: Qaterial.Style.accentColor,
								iconFill: false,
								iconSize: Qaterial.Style.roundIcon.size,
								standardButtons: DialogButtonBox.Ok,
								onAccepted: function() { loadCurrencyDialog() }
							})
		}
	}

	SortFilterProxyModel {
		id: _modelCharacters
		sourceModel: game ? game.modelCharacters : null

		onSourceModelChanged: autoSelect()

		sorters: [
			FilterSorter {
				ValueFilter {
					roleName: "unlock"
					value: 0
				}
				priority: 2
			},

			FilterSorter {
				ValueFilter {
					roleName: "level"
					value: 0
				}
				priority: 1
				sortOrder: Qt.DescendingOrder
			},

			StringSorter {
				roleName: "name"
				priority: 0
				sortOrder: Qt.AscendingOrder
			}
		]
	}


	Connections {
		target: game

		function onRpgUserDataChanged() {
			autoSelect()
		}
	}




	SpotlightCoachTour {
		id: _tour

		page: "rpgCharacters"

		basePage: root

		list: [
			{ target: null, title: qsTr("Tutorial"), text: qsTr("Itt tudod megnézni a tutorialt")},
			{ target: null, title: qsTr("Tokenek"), text: qsTr("Itt láthatod, mennyi tokened van. Ezzel tudsz karaktereket feloldani")},
			{ target: _btnTarget, title: qsTr("Target"), text: qsTr("Ezzel tudod kiválasztani, hogy melyik karakter feloldására gyűjtesz")},
			{ id: 1, target: null, title: qsTr("Preview"), text: qsTr("Ezzel tudod kipróbálni a karaktert") },
		]
	}




	ListModel {
		id: _currencyModel
	}

	function loadCurrencyDialog() {
		_currencyModel.clear()

		for (let i=0; i<_modelCharacters.count; ++i) {
			let ch = _modelCharacters.get(i)
			if (ch.level > 0)
				_currencyModel.append({
										  character: ch.character,
										  text: ch.name
									  })
		}

		Qaterial.DialogManager.openCheckListView(
					{
						onAccepted: function(indexList)
						{
							if (indexList.length === 0)
								return

							var l = []

							for (let i=0; i<indexList.length; ++i) {
								l.push(_currencyModel.get(indexList[i]).character)
							}

							Client.send(HttpConnection.ApiUser, "rpg/upgrade", {
											list: l
										})
							.done(root, function(r){
								game.reloadRpgData()
								Client.snack(qsTr("Sikeres konvertálás"))
							})
							.fail(root, JS.failMessage(qsTr("Konvertálás sikertelen")))
						},
						title: qsTr("Pénz szétosztása"),
						standardButtons: DialogButtonBox.Cancel | DialogButtonBox.Ok,
						model: _currencyModel
					})
	}

	function autoSelect() {
		if (_viewCharacters.selected == "" && game && game.rpgUserData.lastCharacter != "") {
			_viewCharacters.selected = game.rpgUserData.lastCharacter
			game.characterSelect({character: game.rpgUserData.lastCharacter})
		}

		for (let i=0; i<_modelCharacters.count; ++i) {
			if (_modelCharacters.get(i).character === _viewCharacters.selected)
				_viewCharacters.currentIndex = i
		}

		if (!_selectTerrain._selected && game && game.rpgUserData.lastTerrain != "") {
			Client.world.select(game.rpgUserData.lastTerrain)
		}
	}

	onGameChanged: {
		if (!game)
			return

		if (!Client.server || !Client.server.user)
			return

		game.characterSelect({ nickname: Client.server.user.fullNickName })

		if (Client.world)
			Client.world.resetLands(game.rpgUserData.terrains)

		autoSelect()
	}

	StackView.onActivated: {
		if (!_isEmpty)
			Client.sound.playSound("qrc:/sound/voiceover/choose_your_character.mp3", Sound.VoiceoverChannel)

		if (_isEmpty && game.rpgUserData.oldCurrency > 0)
			_timerOldCurrency.start()

		_tour.start()
	}

	StackView.onDeactivating: {
	}
}
