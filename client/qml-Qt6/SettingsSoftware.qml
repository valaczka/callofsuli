import QtQuick
import QtQuick.Controls
import Qaterial as Qaterial
import "./QaterialHelper" as Qaterial
import CallOfSuli
import "JScript.js" as JS

Column {
	id: root

	spacing: 10 * Qaterial.Style.pixelSizeRatio

	Qaterial.SwitchButton {
		anchors.horizontalCenter: parent.horizontalCenter
		text: qsTr("Frissítések automatikus keresése induláskor")
		checked: Client.updater.autoUpdate
		onToggled: Client.updater.autoUpdate = checked
	}

	QButton {
		icon.source: Qaterial.Icons.update
		anchors.horizontalCenter: parent.horizontalCenter
		text: qsTr("Frissítések keresése")
		wrapMode: implicitWidth > parent.width ? Text.Wrap : Text.NoWrap
		onClicked: {
			Client.updater.checkAvailableUpdates(true)
			enabled = false
		}
	}

	Qaterial.IconLabel {
		id: _cacheLabel

		property string cacheSize: Client.Utils.getFormattedDiskCacheSize()

		text: qsTr("Gyorsítótár mérete: <b>%1</b>").arg(cacheSize)

		icon.source: Qaterial.Icons.folderDownload
		anchors.horizontalCenter: parent.horizontalCenter
	}

	QButton {
		icon.source: Qaterial.Icons.folderRemoveOutline
		anchors.horizontalCenter: parent.horizontalCenter
		text: qsTr("Gyorsítótár törlése")
		onClicked: {
			JS.questionDialog({
								  onAccepted: function()
								  {
									  Client.Utils.clearDiskCache()
									  Client.snack("Gyorsítótár törölve")
									  _cacheLabel.cacheSize = Client.Utils.getFormattedDiskCacheSize()
								  },
								  text: qsTr("Biztosan törlöd a gyorsítótárat?"),
								  iconSource: Qaterial.Icons.folderRemoveOutline,
								  title: qsTr("Gyorsítótár törlése")
							  })
		}
	}
}
