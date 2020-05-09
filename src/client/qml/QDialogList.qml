import QtQuick 2.14
import QtQuick.Controls 2.14
import QtGraphicalEffects 1.0
import SortFilterProxyModel 0.2
import "Style"
import "JScript.js" as JS
import "."

Item {
	id: item

	implicitHeight: 300
	implicitWidth: 400

	property alias title: mainRow.title
	property alias newField: tfInput
	property alias list: list
	property ListModel model: ListModel {}
	property bool simpleSelect: !list.selectorSet

	signal dlgClose()
	signal dlgAccept(var data)

	Item {
		id: dlgItem
		anchors.centerIn: parent

		width: Math.min(parent.width*0.9, 650)
		height: parent.height*0.9



		BorderImage {
			id: bgRectMask
			source: "qrc:/img/border.svg"
			visible: false

			anchors.fill: bgRectData

			border.left: 15; border.top: 10
			border.right: 15; border.bottom: 10

			horizontalTileMode: BorderImage.Repeat
			verticalTileMode: BorderImage.Repeat
		}

		Rectangle {
			id: bgRectData

			anchors.fill: rectBg
			visible: false

			color: JS.setColorAlpha(CosStyle.colorPrimaryDark, 0.2)
		}

		OpacityMask {
			id: rectBg
			source: bgRectData
			maskSource: bgRectMask

			anchors.top: parent.top
			anchors.left: parent.left
			anchors.right: parent.right
			anchors.bottom: buttonRow.top
			anchors.bottomMargin: 10

			QDialogHeader {
				id: mainRow
				icon: CosStyle.iconDialogQuestion
			}


			QPageHeader {
				id: header

				anchors.top: mainRow.bottom
				anchors.left: parent.left
				anchors.right: parent.right

				isSelectorMode: list.selectorSet || !tfInput.visible

				labelCountText: list.selectedItemCount

				QTextField {
					id: tfInput
					width: parent.width
				}

				onSelectAll: list.selectAll()
			}

			SortFilterProxyModel {
				id: userProxyModel
				sourceModel: item.model
				filters:  RegExpFilter {
					enabled: header.searchText.text.length
					roleName: list.modelTitleRole
					pattern: header.searchText.text
					caseSensitivity: Qt.CaseInsensitive
					syntax: RegExpFilter.FixedString
				}
			}

			QListItemDelegate {
				id: list
				anchors.top: header.bottom
				anchors.left: parent.left
				anchors.right: parent.right
				anchors.bottom: parent.bottom
				anchors.margins: 30
				anchors.topMargin: 0
				anchors.bottomMargin: mainRow.padding

				model: userProxyModel
				isProxyModel: true

				onClicked: if (simpleSelect) {
							   if (isProxyModel)
								   dlgAccept(model.mapToSource(index))
							   else
								   dlgAccept(index)
						   }
			}

		}

		Row {
			id: buttonRow
			spacing: 10

			anchors.horizontalCenter: parent.horizontalCenter
			anchors.bottom: parent.bottom

			QButton {
				id: buttonNo
				anchors.verticalCenter: parent.verticalCenter
				text: qsTr("Mégsem")
				icon.source: CosStyle.iconCancel
				themeColors: CosStyle.buttonThemeDelete

				onClicked: dlgClose()
			}

			QButton {
				id: buttonYes

				anchors.verticalCenter: parent.verticalCenter

				visible: !simpleSelect

				text: qsTr("OK")
				icon.source: CosStyle.iconOK
				themeColors: CosStyle.buttonThemeApply

				onClicked: dlgAccept(list.currentIndex)
			}
		}

	}


	function populated() {
		list.calculateSelectedItems()
		tfInput.forceActiveFocus()
	}

}
