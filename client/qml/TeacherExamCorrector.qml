import QtQuick 2.15
import QtQuick.Controls 2.15
import Qaterial 1.0 as Qaterial
import "./QaterialHelper" as Qaterial
import CallOfSuli 1.0
import "JScript.js" as JS

Rectangle {
	id: root

	property ExamUser user: null
	property int questionIndex: -1
	property real horizontalPadding: 10 * Qaterial.Style.pixelSizeRatio

	implicitWidth: _labelContent.implicitWidth+_colCorr.width
	implicitHeight: _row.height

	color: "transparent"

	Row {
		id: _row
		anchors.centerIn: parent

		TextEdit {
			id: _labelContent
			wrapMode: Text.Wrap
			width: root.width-_colCorr.width-parent.spacing-2*horizontalPadding
			anchors.verticalCenter: parent.verticalCenter
			font: Qaterial.Style.textTheme.body1
			color: Qaterial.Style.primaryTextColor()
			readOnly: true
		}

		Column {
			id: _colCorr
			anchors.verticalCenter: parent.verticalCenter

			Qaterial.ToggleButton {
				id: _chSuccess
				enabled: user
				anchors.horizontalCenter: parent.horizontalCenter
				text: qsTr("Helyes")
				icon.source: Qaterial.Icons.check
				font.capitalization: Font.AllUppercase
				display: AbstractButton.TextBesideIcon
				leftPadding: 5 * Qaterial.Style.pixelSizeRatio
				rightPadding: 5 * Qaterial.Style.pixelSizeRatio
				onToggled: {
					if (checked && _spinPoint.value < _spinPoint.maxPoint)
						_spinPoint.value = _spinPoint.maxPoint
					user.modify(questionIndex, checked, _spinPoint.value)
				}
			}

			Row {
				anchors.left: parent.left

				QSpinBox {
					id: _spinPoint

					property int maxPoint: 0

					anchors.verticalCenter: parent.verticalCenter
					from: 0
					to: 100
					editable: true
					width: 100 * Qaterial.Style.pixelSizeRatio
					onValueModified: {
						if (value >= maxPoint)
							_chSuccess.checked = true
						user.modify(questionIndex, _chSuccess.checked, value)
					}
				}

				Qaterial.LabelSubtitle1 {
					anchors.verticalCenter: parent.verticalCenter
					text: "/%1".arg(_spinPoint.maxPoint)
					color: _spinPoint.value > _spinPoint.maxPoint ? Qaterial.Style.accentColor : Qaterial.Style.primaryTextColor()
				}

			}
		}
	}

	function setContent() {
		if (!user)
			return

		user.getContent(questionIndex, _labelContent.textDocument, _chSuccess, _spinPoint)

		root.color = user.isModified(questionIndex) ? Qt.rgba(Qaterial.Style.accentColor.r, Qaterial.Style.accentColor.g, Qaterial.Style.accentColor.b,
															  0.2) : "transparent"
	}

	Component.onCompleted: {
		if (!user)
			return

		user.pendingCorrectionChanged.connect(setContent)
		user.correctionChanged.connect(setContent)

		setContent()
	}
}
