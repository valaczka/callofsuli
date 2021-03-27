import QtQuick 2.15
import QtQuick.Controls 2.15
import QtGraphicalEffects 1.0
import SortFilterProxyModel 0.2
import "Style"
import "JScript.js" as JS
import "."

QDialogPanel {
	id: control

	property alias list: list
	property alias selectorSet: list.selectorSet
	readonly property bool simpleSelect: !list.selectorSet
	readonly property alias model: model
	property alias sourceModel: model.sourceModel
	property var roles: ["title"]

	property real imageWidth: delegateHeight
	property real imageHeight: delegateHeight*0.8

	property alias modelTitleRole: list.modelTitleRole
	property alias modelSubtitleRole: list.modelSubtitleRole
	property alias delegateHeight: list.delegateHeight
	property string modelIconRole: ""

	maximumHeight: 0
	maximumWidth: 700

	acceptedData: -1

	QVariantMapProxyView {
		id: list

		anchors.fill: parent

		model: SortFilterProxyModel {
			id: model

			filters: [
				RegExpFilter {
					enabled: toolbar.searchBar.text.length
					roleName: roles[0]
					pattern: toolbar.searchBar.text
					caseSensitivity: Qt.CaseInsensitive
					syntax: RegExpFilter.FixedString
				}
			]

			sorters: [
				StringSorter { roleName: roles[0] }
			]
		}

		modelTitleRole: roles[0]

		autoSelectorChange: false

		leftComponent: Item {
			width: imageWidth+20
			height: list.delegateHeight
			anchors.verticalCenter: parent.verticalCenter
			visible: model && modelIconRole.length

			Image {
				width: imageWidth
				height: imageHeight
				fillMode: Image.PreserveAspectFit
				anchors.centerIn: parent
				source: model && modelIconRole.length ? model[modelIconRole] : ""
			}
		}


		onClicked: if (simpleSelect) {
					   acceptedData = model.mapToSource(currentIndex)
					   dlgClose()
				   }
	}

	QPagePanelSearch {
		id: toolbar

		listView: list

		enabled: model.sourceModel.count
		labelCountText: model.sourceModel.selectedCount
		onSelectAll: JS.selectAllProxyModelToggle(model)
	}


	buttons: Row {
		id: buttonRow
		spacing: 10

		anchors.horizontalCenter: parent.horizontalCenter

		QButton {
			id: buttonNo
			anchors.verticalCenter: parent.verticalCenter
			text: qsTr("Mégsem")
			icon.source: CosStyle.iconCancel
			themeColors: CosStyle.buttonThemeRed

			onClicked: dlgClose()
		}

		QButton {
			id: buttonYes

			anchors.verticalCenter: parent.verticalCenter

			visible: !simpleSelect

			text: qsTr("OK")
			icon.source: CosStyle.iconOK
			themeColors: CosStyle.buttonThemeGreen

			onClicked: {
				dlgClose()
			}
		}
	}



	function populated() {
		list.forceActiveFocus()
	}

}
