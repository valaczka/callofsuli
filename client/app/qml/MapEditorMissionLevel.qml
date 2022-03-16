import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.14
import SortFilterProxyModel 0.2
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS


QTabContainer {
	id: control

	title: labelTitle.text
	icon: srcImg.source

	property int contextAction: MapEditorAction.ActionTypeMissionLevel
	property int actionContextType: -1
	property var actionContextId: null

	property MapEditor _mapEditor: null
	property GameMapEditorMissionLevel missionLevel: null
	property bool canDelete: false

	onMissionLevelChanged: {
		if (missionLevel && missionLevel.mission) {
			var mx = 2

			for (var i=0; i<missionLevel.mission.levels.count; i++) {
				var ml = missionLevel.mission.levels.object(i)
				if (ml.level > mx)
					mx = ml.level
			}

			canDelete = (missionLevel.level === mx)

		} else {
			canDelete = false
		}
	}


	QAccordion {
		visible: missionLevel

		QTabHeader {
			tabContainer: control
			visible: control.compact
			isPlaceholder: true
		}

		Row {
			anchors.horizontalCenter: parent.horizontalCenter
			bottomPadding: 50

			Image {
				id: srcImg
				source: missionLevel && missionLevel.mission ? cosClient.medalIconPath(missionLevel.mission.medalImage) : ""
				anchors.verticalCenter: parent.verticalCenter
				width: CosStyle.pixelSize*2
				height: CosStyle.pixelSize*2
				fillMode: Image.PreserveAspectFit
			}

			Column {
				anchors.verticalCenter: parent.verticalCenter
				QLabel {
					id: labelTitle
					text: missionLevel && missionLevel.mission ? missionLevel.mission.name : ""
					width: Math.min(implicitWidth, control.width-srcImg.width-20)
					font.weight: Font.Normal
					font.pixelSize: CosStyle.pixelSize*1.4
					font.capitalization: Font.AllUppercase
					elide: Text.ElideMiddle
					color: CosStyle.colorWarningLight
					leftPadding: CosStyle.pixelSize/2
				}
				QLabel {
					text: missionLevel ? "Level %1".arg(missionLevel.level) : ""
					width: Math.min(implicitWidth, labelTitle.width)
					font.weight: Font.DemiBold
					font.pixelSize: CosStyle.pixelSize*0.8
					font.capitalization: Font.AllUppercase
					color: CosStyle.colorWarningLight
					leftPadding: CosStyle.pixelSize/2
				}
			}
		}

		Row {
			anchors.horizontalCenter: parent.horizontalCenter
			spacing: 10

			QButton {
				anchors.verticalCenter: parent.verticalCenter

				themeColors: CosStyle.buttonThemeGreen

				icon.source: CosStyle.iconPlay
				text: qsTr("Lejátszás")
				onClicked: mapEditor.play({"level": level})
			}


			QButton {
				anchors.verticalCenter: parent.verticalCenter
				visible: canDelete

				themeColors: CosStyle.buttonThemeRed

				text: qsTr("Törlés")
				icon.source: CosStyle.iconDelete
				onClicked: mapEditor.missionLevelRemove({"level": level})
			}
		}

		QGridLayout {
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
												   model: mapEditor.modelTerrainList
											   })

					if (_terrain.length)
						d.item.selectCurrentItem("name", _terrain)

					d.accepted.connect(function(data) {
						if (data === -1)
							return

						var p = d.item.list.modelObject(data)
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
				text: qsTr("Sudden death engedélyezve")

				Layout.fillWidth: false
				Layout.alignment: Qt.AlignHCenter

				onToggled: {
					mapEditor.missionLevelModify({level: container.level, deathmatch: checked})
				}
			}


		}


		/*MapEditorInventory {
		id: inventory
		collapsed: true
		level: container.level
	}*/




		QObjectListDelegateView {
			id: chapterList
			width: parent.width

			selectorSet: missionLevel && missionLevel.chapters.selectedCount

			model: SortFilterProxyModel {
				sourceModel: missionLevel ? missionLevel.chapters : null

				sorters: RoleSorter {
					roleName: "id"
				}
			}

			delegate: MapEditorChapter {
				id: chapterItem
				required property int index
				collapsed: true
				level: missionLevel.level
				selectorSet: chapterList.selectorSet
				onLongClicked: chapterList.onDelegateLongClicked(index)
				onSelectToggled: chapterList.onDelegateClicked(index, withShift)
				self: chapterList.modelObject(index)

				onSelfChanged: if (!self) {
								   delete chapterItem
							   }

				onChapterRemove: {
					/*if (mapEditor.editor.chapters.selectedCount > 0) {
						mapEditor.chapterRemoveList(mapEditor.editor.chapters.getSelected())
					} else {
						mapEditor.chapterRemove(self)
					}*/
				}
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

}
