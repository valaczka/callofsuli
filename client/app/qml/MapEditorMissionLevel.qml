import QtQuick 2.15
import QtQuick.Controls 2.15
import SortFilterProxyModel 0.2
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS


QSwipeContainer {
	id: container

	required property int level
	property int maxLevel: 0

	title: qsTr("Level %1").arg(level)
	icon: level<=maxLevel ? CosStyle.iconTrophy : CosStyle.iconCancel

	QAccordion {
		id: acc
		visible: level <= maxLevel

		QCollapsible {
			title: qsTr("Alapadatok")
			rightComponent: Row {
				spacing: 0
				QToolButton {
					icon.source: CosStyle.iconPlay
					color: CosStyle.colorOKLighter
					ToolTip.text: qsTr("Lejátszás")
					onClicked: mapEditor.play({"level": level})
				}
				QToolButton {
					icon.source: CosStyle.iconDelete
					color: CosStyle.colorError
					ToolTip.text: qsTr("Szint törlése")
					visible: level == maxLevel && level>1
					onClicked: mapEditor.missionLevelRemove({"level": level})
				}
			}

			QGridLayout {
				width: parent.width
				watchModification: false

				QGridImage {
					id: imageTerrain
					fieldName: qsTr("Harcmező")

					property string _terrain: ""
					property bool _invalid: true

					imageImg.width: imageTerrain.height*1.5

					labelTitle.color: CosStyle.colorAccentLighter
					labelSubtitle.color: CosStyle.colorAccent

					rightComponent: QFontImage {
						visible: imageTerrain._invalid
						color: CosStyle.colorWarning
						icon: CosStyle.iconDialogWarning
					}

					mouseArea.onClicked: {
						var d = JS.dialogCreateQml("List", {
													   roles: ["readableName", "details", "thumbnail"],
													   icon: CosStyle.iconLockAdd,
													   title: qsTr("Harcmező kiválasztása"),
													   selectorSet: false,
													   modelImageRole: "thumbnail",
													   modelSubtitleRole: "details",
													   delegateHeight: CosStyle.twoLineHeight*1.2,
													   sourceModel: mapEditor.modelTerrainList
												   })

						if (_terrain.length)
							d.item.selectCurrentItem("name", _terrain)

						d.accepted.connect(function(data) {
							if (data === -1)
								return

							var p = d.item.sourceModel.get(data)
							mapEditor.missionLevelModify({level: container.level, terrain: p.name})

						})
						d.open()
					}
				}


				QGridLabel { text: qsTr("Időtartam") }

				QGridSpinBox {
					id: spinDuration
					from: 30
					to: 2400
					stepSize: 30
					//editable: true

					textFromValue: function(value) {
						return JS.secToMMSS(value)
					}

					valueFromText: function(text) {
						return JS.mmSStoSec(text)
					}

					onValueModified: {
						mapEditor.missionLevelModify({level: container.level, duration: value})
					}
				}


				QGridLabel { text: qsTr("HP") }

				QGridSpinBox {
					id: spinHP
					from: 1
					to: 99

					onValueModified: {
						mapEditor.missionLevelModify({level: container.level, startHP: value})
					}
				}


				QGridLabel { text: qsTr("Kérdések aránya csataterenként") }

				QGridSpinBox {
					id: spinQuestions
					from: 0
					to: 100
					stepSize: 5
					//editable: true

					textFromValue: function(value) {
						return String("%1%").arg(value)
					}

					/*valueFromText: function(text) {
						return JS.mmSStoSec(text)
					}*/

					onValueModified: {
						mapEditor.missionLevelModify({level: container.level, questions: Number(value/100)})
					}
				}




				QGridCheckBox {
					id: checkDeathmatch
					text: qsTr("Deathmatch engedélyezve")

					onToggled: {
						mapEditor.missionLevelModify({level: container.level, deathmatch: checked})
					}
				}


			}

		}

		MapEditorInventory {
			id: inventory
			collapsed: true
			level: container.level
		}



		Repeater {
			model: SortFilterProxyModel {
				sourceModel: mapEditor.modelLevelChapterList

				filters: ValueFilter {
					roleName: "level"
					value: container.level
				}

				sorters: StringSorter {
					roleName: "name"
				}
			}

			MapEditorChapter {
				collapsed: true
				level: container.level
			}
		}

		QToolButtonFooter {
			anchors.horizontalCenter: parent.horizontalCenter
			icon.source: CosStyle.iconAdd
			text: qsTr("Létező szakasz hozzáadása")
			onClicked: mapEditor.missionLevelGetChapterList(level)
		}

		QToolButtonFooter {
			anchors.horizontalCenter: parent.horizontalCenter
			icon.source: CosStyle.iconAdd
			text: qsTr("Új szakasz létrehozása")
			onClicked: {
				var d = JS.dialogCreateQml("TextField", {
											   title: qsTr("Új szakasz"),
											   text: qsTr("Az új szakasz neve")
											   })

				d.accepted.connect(function(data) {
					if (data.length)
						mapEditor.missionLevelChapterAdd({level: level, name: data})
				})
				d.open()
			}
		}
	}

	QToolButtonBig {
		visible: level == maxLevel+1
		anchors.centerIn: parent
		color: CosStyle.colorAccent
		text: qsTr("Szint hozzáadása")
		icon.source: CosStyle.iconAdd
		onClicked: mapEditor.missionLevelAdd({"level": level})
	}


	Connections {
		target: mapEditor

		function onCurrentMissionDataChanged(data) {
			var l = data.levels ? data.levels.length : 0

			maxLevel = l

			for (var i=0; i<l; i++) {
				var p = data.levels[i]
				if (p.level && p.level === level) {
					var t = cosClient.terrainMap()[p.terrain]

					imageTerrain._terrain = p.terrain

					if (t) {
						imageTerrain.image = t.thumbnail
						imageTerrain.title = t.readableName
						imageTerrain.subtitle = t.details
						imageTerrain._invalid = false
					} else {
						imageTerrain.image = ""
						imageTerrain.title = p.terrain
						imageTerrain.subtitle = ""
						imageTerrain._invalid = true
					}

					spinDuration.value = p.duration
					spinHP.value = p.startHP
					spinQuestions.value = p.questions*100
					checkDeathmatch.checked = p.deathmatch

					break
				}
			}
		}

		function onCurrentMissionChanged(m) {
			inventory.collapsed = true
			acc.flickable.returnToBounds()
		}
	}
}
