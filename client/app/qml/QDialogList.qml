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
	property alias model: list.model

	property alias modelTitleRole: list.modelTitleRole
	property alias modelSubtitleRole: list.modelSubtitleRole
	property alias delegateHeight: list.delegateHeight
	property string modelIconRole: ""
	property alias modelTitleColorRole: list.modelTitleColorRole
	property alias modelSubtitleColorRole: list.modelSubtitleColorRole
	property string modelIconColorRole: ""
	property string modelRightTextRole: ""

	readonly property bool simpleSelect: !list.selectorSet

	icon: "qrc:/internal/icon/message-bulleted.svg"

	maximumHeight: 0
	maximumWidth: 700

	acceptedData: null

	QObjectListView {
		id: list

		anchors.fill: parent

		model: ListModel { }

		autoSelectorChange: false

		leftComponent: QFontImage {
			width: control.delegateHeight*1.2
			height: control.delegateHeight
			size: control.delegateHeight*0.7

			visible: model && modelIconRole.length

			icon: model && modelIconRole.length ? model[modelIconRole] : ""

			color: model && modelIconColorRole.length ? model[modelIconColorRole] : CosStyle.colorPrimaryLighter
		}

		rightComponent: QLabel {
			anchors.verticalCenter: parent.verticalCenter
			visible: model && modelRightTextRole.length
			text: model && modelRightTextRole.length ? model[modelRightTextRole] : ""
		}


		onClicked: if (simpleSelect) {
					   acceptedData = modelObject(index)
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
			icon.source: "qrc:/internal/icon/close-circle.svg"
			themeColors: CosStyle.buttonThemeRed

			onClicked: dlgClose()
		}

		QButton {
			id: buttonYes

			anchors.verticalCenter: parent.verticalCenter

			visible: !simpleSelect

			text: qsTr("OK")
			icon.source: "qrc:/internal/icon/check-bold.svg"
			themeColors: CosStyle.buttonThemeGreen

			onClicked: {
				acceptedData = true
				dlgClose()
			}
		}
	}


	function selectCurrentItem(field, value) {
		for (var i=0; i<list.model.count; i++) {
			var p = list.modelObject(i)
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
