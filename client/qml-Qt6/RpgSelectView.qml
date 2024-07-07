import QtQuick
import QtQuick.Controls
import CallOfSuli
import Qaterial as Qaterial
import "./QaterialHelper" as Qaterial
import "JScript.js" as J

ListView {
	id: view

	orientation: ListView.Horizontal

	implicitHeight: 110*Qaterial.Style.pixelSizeRatio

	spacing: 5 * Qaterial.Style.pixelSizeRatio

	clip: true

	snapMode: ListView.SnapToItem

	header: Item {
		width: Math.max(Client.safeMarginLeft, Qaterial.Style.card.horizontalPadding)
		height: view.height
	}

	footer: Item {
		width: Math.max(Client.safeMarginRight, Qaterial.Style.card.horizontalPadding)
		height: view.height
	}

}
