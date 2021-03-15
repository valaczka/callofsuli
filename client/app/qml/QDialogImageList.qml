import QtQuick 2.15
import QtQuick.Controls 2.15
import QtGraphicalEffects 1.0
import SortFilterProxyModel 0.2
import "Style"
import "JScript.js" as JS
import "."

QDialogPanel {
	id: item

	property alias list: list
	property alias selectorSet: list.selectorSet
	readonly property bool simpleSelect: !list.selectorSet
	readonly property alias model: model
	property alias sourceModel: model.sourceModel
	property var roles: ["title", "image"]

	property alias modelTitleRole: list.modelTitleRole
	property alias modelSubTitleRole: list.modelSubtitleRole
	property alias delegateHeight: list.delegateHeight

	property string modelImagePattern: "%1"
	property string modelImageRole: "image"
	property int modelImageHeight: list.delegateHeight*0.85
	property int modelImageWidth: list.delegateHeight

	maximumHeight: 0
	maximumWidth: 700

	acceptedData: -1

	QVariantMapProxyView {
		id: list

		anchors.fill: parent

		model: SortFilterProxyModel {
			id: model

			sorters: [
				StringSorter { roleName: roles[0] }
			]
		}

		leftComponent: Image {
			source: modelImagePattern.arg(model[modelImageRole])
			width: modelImageWidth
			height: modelImageHeight
			fillMode: Image.PreserveAspectFit
		}

		modelTitleRole: roles[0]

		autoSelectorChange: false

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



	function populated() {
		list.forceActiveFocus()
	}

}