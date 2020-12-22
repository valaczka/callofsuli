import QtQuick 2.12
import QtQuick.Controls 2.12
import SortFilterProxyModel 0.2
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS

QPagePanel {
	id: panel

	layoutFillWidth: true

	title: qsTr("Küldetés")
	icon: CosStyle.iconUsers

	contextMenuFunc: function (m) {
		m.addAction(actionMapAdd)
		m.addAction(actionMapRemove)
		m.addSeparator()
		m.addAction(actionPlay)
	}


	Connections {
		target: mapEditor

		function onLevelLoaded(data) {
			mapEditor.modelBlockChapterMapList.clear()

			if (!Object.keys(data).length) {
				return
			}

			if (data.rowid !== mapEditor.selectedLevelRowID)
				return


			spinBlock.to = data.terrainBlocks

			JS.setSqlFields([
								textMissionName,
								textLevel,
								textDuration,
								spinHP,
								spinBlock
							], data)


			mapEditor.modelBlockChapterMapList.setVariantList(data.blockDataList, "id")

			actionMapAdd.enabled = data.canAdd
		}

		function onPlayReady(gameMatch) {
			console.debug("PLAY", gameMatch)
			var o = JS.createPage("Game", {
									  gameMatch: gameMatch,
									  deleteGameMatch: true
								  })
		}
	}

	Connections {
		target: mapEditor.db

		function onUndone() {
			mapEditor.run("levelLoad", {rowid: mapEditor.selectedLevelRowID})
		}
	}


	QAccordion {
		anchors.fill: parent

		QCollapsible {
			title: qsTr("Általános")

			QGridLayout {
				width: parent.width

				watchModification: false

				QGridLabel { field: textMissionName }

				QGridTextField {
					id: textMissionName
					fieldName: qsTr("Küldetés")
					sqlField: "name"

					readOnly: true
				}


				QGridLabel { field: textLevel }

				QGridTextField {
					id: textLevel
					fieldName: qsTr("Szint")
					sqlField: "level"

					readOnly: true
				}

				QGridLabel { field: textDuration }

				QGridTextField {
					id: textDuration
					fieldName: qsTr("Hossz")
					sqlField: "duration"

					placeholderText: qsTr("MM:SS")

					validator: RegExpValidator { regExp: /\d\d:\d\d/ }

					onTextModified: {
						mapEditor.run("levelModify", {rowid: mapEditor.selectedLevelRowID, data:{ duration: JS.mmSStoSec(text) }})
					}

					function setData(t) {
						text = JS.secToMMSS(t)
					}
				}

				QGridLabel { text: qsTr("HP") }

				QGridSpinBox {
					id: spinHP
					sqlField: "startHP"
					from: 1
					to: 99

					onValueModified: {
						mapEditor.run("levelModify", {rowid: mapEditor.selectedLevelRowID, data: { startHP: value }})
					}
				}

				QGridLabel { text: qsTr("Kezdő csatatér") }

				QGridSpinBox {
					id: spinBlock
					sqlField: "startBlock"
					from: 1
					to: 1

					onValueModified: {
						mapEditor.run("levelModify", {rowid: mapEditor.selectedLevelRowID, data: { startBlock: value }})
					}
				}

				QGridLabel { text: qsTr("Háttérkép") }

				QGridTextField {
					text: "--- TODO ---"
					readOnly: true
				}

				QGridButton {
					action: actionPlay
					display: Button.TextBesideIcon
				}
			}
		}


		QCollapsible {
			title: qsTr("Feladatelosztás")

			QVariantMapProxyView {
				id: blocksView

				model: SortFilterProxyModel {
					id: blocksProxyModel

					sourceModel: mapEditor.modelBlockChapterMapList

					sorters: [
						RoleSorter { roleName: "id" }
					]

					proxyRoles: [
						ExpressionRole {
							name: "fullname"
							expression: model.blocks ? qsTr("Csataterek: ")+model.blocks : qsTr("Teljes harcmező")
						}
					]
				}

				autoSelectorChange: true

				modelTitleRole: "fullname"

				delegateHeight: CosStyle.twoLineHeight

				leftComponent: QLabel {
					width: height*2
					height: blocksView.delegateHeight
					text: model && model.enemies ? model.enemies : ""
					color: CosStyle.colorAccentLighter
					font.pixelSize: height*0.75
					font.weight: Font.Thin
					horizontalAlignment: Text.AlignHCenter
					verticalAlignment: Text.AlignVCenter
				}

				width: parent.width

				onClicked: {
					var id = model.get(index).id
					mapEditor.blockChapterMapSelected(id)
					mapEditor.run("blockChapterMapLoad", {id: id})
				}
			}

		}


	}



	Action {
		id: actionMapAdd
		text: qsTr("Elosztás hozzáadása")
		icon.source: CosStyle.iconAdd
		enabled: false

		onTriggered: mapEditor.run("blockChapterMapAdd", { rowid: mapEditor.selectedLevelRowID })
	}


	Action {
		id: actionMapRemove
		text: qsTr("Elosztás törlése")
		icon.source: CosStyle.iconDelete
		enabled: mapEditor.modelBlockChapterMapList.count > 1 && (blocksView.currentIndex !== -1 || mapEditor.modelBlockChapterMapList.selectedCount)

		onTriggered: {
			var o = blocksView.model.get(blocksView.currentIndex)

			var more = mapEditor.modelBlockChapterMapList.selectedCount

			if (more > 0)
				mapEditor.run("blockChapterMapRemove", {
								  rowid: mapEditor.selectedLevelRowID,
								  list: mapEditor.modelBlockChapterMapList.getSelectedData("id")
							  })
			else
				mapEditor.run("blockChapterMapRemove", {
								  rowid: mapEditor.selectedLevelRowID,
								  id: o.id
							  })
		}
	}

	Action {
		id: actionPlay
		icon.source: CosStyle.iconSend
		text: qsTr("Lejátszás")
		onTriggered: {
			mapEditor.play({rowid: mapEditor.selectedLevelRowID})
		}
	}


	Component.onCompleted: {
		mapEditor.levelComponentsCompleted++
	}

	onPanelActivated: {
	}
}



