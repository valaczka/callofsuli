import QtQuick 2.15
import QtQuick.Controls 2.15
import Qaterial 1.0 as Qaterial
import "./QaterialHelper" as Qaterial
import CallOfSuli 1.0

Column {
	id: root

	Row {
		spacing: 12 * Qaterial.Style.pixelSizeRatio
		Qaterial.IconLabel {
			icon.source: Qaterial.Icons.formatFont
			anchors.verticalCenter: parent.verticalCenter
			text: qsTr("Betűméret:")
		}

		Qaterial.LabelHeadline6 {
			anchors.verticalCenter: parent.verticalCenter
			text: Qaterial.Style.userPixelSize
			color: Qaterial.Style.accentColor
		}

		Row {
			anchors.verticalCenter: parent.verticalCenter
			Qaterial.ToolButton {
				action: Client.mainWindow.fontMinus
				display: AbstractButton.IconOnly
			}

			Qaterial.ToolButton {
				action: Client.mainWindow.fontNormal
				display: AbstractButton.IconOnly
			}

			Qaterial.ToolButton {
				action: Client.mainWindow.fontPlus
				display: AbstractButton.IconOnly
			}
		}
	}
}
