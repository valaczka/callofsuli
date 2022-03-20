import QtQuick 2.15
import QtQuick.Controls 2.15
import SortFilterProxyModel 0.2
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS



QTabContainer {
	id: control

	title: qsTr("Adatbankok")
	icon: CosStyle.iconComputer

	readonly property int contextAction: MapEditorAction.ActionTypeStorageList
	property int actionContextType: -1
	property var actionContextId: null

	QObjectListDelegateView {
		id: storageList
		anchors.fill: parent

		visible: mapEditor.editor.storages.count

		selectorSet: mapEditor.editor.storages.selectedCount

		autoSelectorChange: false

		model: SortFilterProxyModel {
			sourceModel: mapEditor.editor.storages

			sorters: RoleSorter {
				roleName: "id"
			}
		}


		delegate: Item {
			id: item
			width: storageList.width
			height: CosStyle.twoLineHeight*1.7

			required property bool selected
			required property int index

			property GameMapEditorStorage storageSelf: storageList.modelObject(index)
			readonly property var storageInfo: mapEditor.storageInfo(storageSelf)

			readonly property color mainColor: CosStyle.colorOKLighter


			onStorageSelfChanged: if (!storageSelf) {
									  delete item
								  }

			QRectangleBg {
				anchors.fill: parent
				acceptedButtons: Qt.LeftButton

				Item {
					id: rect
					anchors.fill: parent
					anchors.leftMargin: 10
					anchors.rightMargin: badge.visible ? 20 : 10


					QLabel {
						id: labelName
						text: storageInfo ? storageInfo.name : ""
						color: CosStyle.colorPrimary
						font.weight: Font.DemiBold
						font.pixelSize: CosStyle.pixelSize*0.6
						font.capitalization: Font.AllUppercase
						anchors.left: parent.left
						anchors.top: parent.top
						anchors.leftMargin: 3
						anchors.topMargin: 1
					}

					Row {
						anchors.verticalCenter: parent.verticalCenter

						spacing: 0

						QFontImage {
							id: imgModule
							width: Math.max(rect.height*0.5, size*1.1)
							size: CosStyle.pixelSize*1.5
							anchors.verticalCenter: parent.verticalCenter
							icon: storageInfo ? storageInfo.icon : ""
							color: CosStyle.colorPrimary
						}


						Column {
							anchors.verticalCenter: parent.verticalCenter
							anchors.verticalCenterOffset: (labelName.height/2)*(subtitle.lineCount-1)/3

							QLabel {
								id: title
								anchors.left: parent.left
								width: rect.width-imgModule.width
									   -(badge.visible ? badge.width : 0)
									   -(btnDelete.visible ? btnDelete.width : 0)
								text: storageInfo ? storageInfo.title : ""
								color: CosStyle.colorPrimaryLighter
								font.pixelSize: CosStyle.pixelSize
								font.weight: Font.Normal
								maximumLineCount: 1
								lineHeight: 0.9
								elide: Text.ElideRight
							}
							QLabel {
								id: subtitle
								anchors.left: parent.left
								width: title.width
								text: storageInfo ? storageInfo.details : ""
								color: CosStyle.colorPrimary
								font.pixelSize: CosStyle.pixelSize*0.75
								font.weight: Font.Light
								maximumLineCount: 1
								lineHeight: 0.8
								wrapMode: Text.Wrap
								elide: Text.ElideRight
							}
						}

						QBadge {
							id: badge
							text: storageSelf ? storageSelf.objectiveCount : ""
							color: CosStyle.colorPrimaryDarker
							anchors.verticalCenter: parent.verticalCenter
							visible: storageSelf && storageSelf.objectiveCount > 0
						}

						QToolButton {
							id: btnDelete
							anchors.verticalCenter: parent.verticalCenter
							icon.source: CosStyle.iconDelete
							color: CosStyle.colorErrorLighter
							visible: storageSelf && storageSelf.objectiveCount === 0
							onClicked: {
								mapEditor.storageRemove(storageSelf)
							}
						}

					}
				}


				mouseArea.onClicked: {
					/*storageId = item.id > 0 ? item.id : -1
					control.storageData = item.storageData
					storageModule = item.module
					stack.replace(cmpEdit)*/
				}
			}

			Rectangle {
				anchors.top: parent.top
				anchors.left: parent.left
				width: parent.width
				height: 1
				color: CosStyle.colorPrimaryDark
			}
		}


		header: QTabHeader {
			tabContainer: control
			isPlaceholder: true
		}


		/*footer: Column {
			width: storageList.width

			QToolButtonFooter {
				anchors.horizontalCenter: parent.horizontalCenter
				action: actionChapterNew
			}

			QToolButtonFooter {
				anchors.horizontalCenter: parent.horizontalCenter
				icon.source: CosStyle.iconAdd
				text: qsTr("Importálás")

				onClicked: {
					var d = JS.dialogCreateQml("File", {
												   isSave: false,
												   folder: cosClient.getSetting("mapFolder", ""),
											   })
					d.item.filters = ["*.xlsx", "*.xls"]

					d.accepted.connect(function(data){
						mapEditor.chapterImport({filename: data})
						cosClient.setSetting("mapFolder", d.item.modelFolder)
					})

					d.open()
				}
			}
		}*/
	}

	/*QToolButtonBig {
		anchors.centerIn: parent
		visible: !mapEditor.editor.chapters.count
		action: actionChapterNew
		color: CosStyle.colorOKLighter
	}*/


	onPopulated: {
		storageList.forceActiveFocus()
	}


	function loadContextId(type, id) {

	}


	backCallbackFunction: function () {
		if (mapEditor.editor.storages.selectedCount) {
			mapEditor.editor.storages.unselectAll()
			return true
		}

		return false
	}
}

