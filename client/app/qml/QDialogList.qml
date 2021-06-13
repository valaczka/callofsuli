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
	property string modelImageRole: ""
	property string modelImagePattern: "%1"
	property string modelRightTextRole: ""

	maximumHeight: 0
	maximumWidth: 700

	acceptedData: -1

	SortFilterProxyModel {
		id: model

		sorters: [
			StringSorter {
				roleName: roles[0]
			}
		]
	}

	QVariantMapProxyView {
		id: list

		anchors.fill: parent

		model: model

		modelTitleRole: roles[0]

		autoSelectorChange: false

		leftComponent: Item {
			width: imageWidth+20
			height: list.delegateHeight
			anchors.verticalCenter: parent.verticalCenter
			visible: model && modelImageRole.length

			Image {
				width: imageWidth
				height: imageHeight
				fillMode: Image.PreserveAspectFit
				anchors.centerIn: parent
				source: model && modelImageRole.length ? modelImagePattern.arg(model[modelImageRole]) : ""
			}
		}

		rightComponent: QLabel {
			anchors.verticalCenter: parent.verticalCenter
			visible: model && modelRightTextRole.length
			text: model && modelRightTextRole.length ? model[modelRightTextRole] : ""
		}


		onClicked: if (simpleSelect) {
					   acceptedData = model.mapToSource(currentIndex)
					   dlgClose()
				   }
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


	function selectCurrentItem(field, value) {
		for (var i=0; i<list.model.count; i++) {
			var p = list.model.get(i)
			if (p[field] === value) {
				list.currentIndex = i
				list.positionViewAtIndex(i, ListView.Contain)
				break
			}
		}
	}


	function populated() {
		list.forceActiveFocus()
	}

}
