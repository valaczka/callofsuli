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

	property GameMapEditorMissionLevel missionLevel: null
	property bool canDelete: false

	property ListModel availableTerrainModel: ListModel {}


	onMissionLevelChanged: {
		if (missionLevel && missionLevel.mission)
			canDelete = missionLevel.isLastLevel() && missionLevel.level > 1
		else
			canDelete = false
	}

	SortFilterProxyModel {
		id: terrainProxyModel

		sourceModel: availableTerrainModel

		sorters: [
			StringSorter {
				roleName: "readableName"
				priority: 2
			},
			RoleSorter {
				roleName: "level"
				priority: 1
			}
		]

		proxyRoles: ExpressionRole {
			name: "readableLevel"
			expression: "Level %1".arg(model.level)
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
				onClicked: mapEditor.missionLevelPlay(missionLevel)
			}


			QButton {
				anchors.verticalCenter: parent.verticalCenter
				visible: canDelete

				themeColors: CosStyle.buttonThemeRed

				text: qsTr("Törlés")
				icon.source: CosStyle.iconDelete
				onClicked: mapEditor.missionLevelRemove(missionLevel)
			}
		}

		QGridLayout {
			watchModification: false

			QGridText { text: qsTr("Harcmező") }

			QGridImage {
				id: imageTerrain
				Layout.columnSpan: 1

				property var _terrainData: missionLevel ? cosClient.terrainMap()[missionLevel.terrain] : null
				property bool _invalid: true

				title: _terrainData ? _terrainData.readableName : (missionLevel ? missionLevel.terrain : "")
				subtitle: _terrainData ? "Level %1".arg(_terrainData.level) : ""

				image: _terrainData ? _terrainData.thumbnail : ""

				imageImg.width: imageTerrain.height*1.5

				labelTitle.color: CosStyle.colorAccentLighter
				labelSubtitle.color: CosStyle.colorAccentLighter

				rightComponent: QFontImage {
					visible: !imageTerrain._terrainData
					color: CosStyle.colorWarning
					icon: CosStyle.iconDialogWarning
					size: CosStyle.pixelSize*1.2
					width: size*2
				}

				mouseArea.onClicked: {
					var dd = JS.dialogCreateQml("List", {
													icon: CosStyle.iconLockAdd,
													title: qsTr("Harcmező kiválasztása"),
													selectorSet: false,
													modelTitleRole: "readableName",
													modelImageRole: "thumbnail",
													modelSubtitleRole: "readableLevel",
													delegateHeight: CosStyle.twoLineHeight*1.5,
													model: terrainProxyModel
												})

					if (_terrainData)
						dd.item.selectCurrentItem("terrain", missionLevel.terrain)

					dd.accepted.connect(function(data) {
						if (data)
							mapEditor.missionLevelModify(missionLevel, {terrain: data.terrain})
					})
					dd.open()

				}
			}


			QGridLabel { text: qsTr("Időtartam") }

			QGridSpinBox {
				id: spinDuration
				from: 30
				to: 2400
				stepSize: 30
				//editable: true

				value: missionLevel ? missionLevel.duration : 0

				textFromValue: function(value) {
					return JS.secToMMSS(value)
				}

				valueFromText: function(text) {
					return JS.mmSStoSec(text)
				}

				onValueModified: {
					mapEditor.missionLevelModify(missionLevel, {duration: value})
				}
			}


			QGridLabel { text: qsTr("HP") }

			QGridSpinBox {
				id: spinHP
				from: 1
				to: 99

				value: missionLevel ? missionLevel.startHP : 0

				onValueModified: {
					mapEditor.missionLevelModify(missionLevel, {startHP: value})
				}
			}


			QGridLabel { text: qsTr("Kérdések aránya") }

			QGridSpinBox {
				id: spinQuestions
				from: 0
				to: 100
				stepSize: 5
				//editable: true

				value: missionLevel ? missionLevel.questions*100 : 0

				textFromValue: function(value) {
					return String("%1%").arg(value)
				}

				/*valueFromText: function(text) {
						return JS.mmSStoSec(text)
					}*/

				onValueModified: {
					mapEditor.missionLevelModify(missionLevel, {questions: Number(value/100)})
				}
			}



			QGridCheckBox {
				id: checkDeathmatch
				text: qsTr("Sudden death engedélyezve")

				checked: missionLevel ? missionLevel.canDeathmatch : false

				onToggled: {
					mapEditor.missionLevelModify(missionLevel, {canDeathmatch: checked})
				}
			}


		}



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
					if (missionLevel.chapters.selectedCount > 0) {
						mapEditor.missionLevelRemoveChapterList(missionLevel, missionLevel.chapters.getSelected())
					} else {
						mapEditor.missionLevelRemoveChapter(missionLevel, self)
					}
				}
			}

		}

		QToolButtonFooter {
			anchors.horizontalCenter: parent.horizontalCenter
			icon.source: CosStyle.iconEdit
			text: qsTr("Szakaszok kiválasztása")
			onClicked: {
				mapEditor.updateChapterModelMissionLevel(missionLevel)

				if (mapEditor.editor.chapters.count < 1) {
					cosClient.sendMessageWarning(qsTr("Szakaszok"), qsTr("Még nincsen egyetlen szakasz sem!"))
					return
				}


				var d = JS.dialogCreateQml("List", {
											   icon: CosStyle.iconLockAdd,
											   title: qsTr("%1 - Szakaszok").arg(labelTitle.text),
											   selectorSet: true,
											   modelTitleRole: "name",
											   delegateHeight: CosStyle.baseHeight,
											   model: mapEditor.editor.chapters
										   })

				d.accepted.connect(function(dlgdata) {
					var l = mapEditor.editor.chapters.getSelected()
					mapEditor.editor.chapters.unselectAll()

					if (!dlgdata)
						return

					mapEditor.missionLevelModifyChapters(missionLevel, l)
				})

				d.rejected.connect(function() {
					mapEditor.editor.chapters.unselectAll()
				})

				d.open()
			}
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
						mapEditor.chapterAdd({name: data}, missionLevel)
				})
				d.open()
			}
		}


		MapEditorInventory {
			id: inventory
			collapsed: true
			missionLevel: control.missionLevel
		}

	}

	Component.onCompleted: {
		var l = mapEditor.availableTerrains

		for (var i=0; i<l.length; i++) {
			availableTerrainModel.append(l[i])
		}
	}
}
