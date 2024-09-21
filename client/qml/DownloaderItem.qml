import QtQuick
import QtQuick.Controls
import CallOfSuli
import Qaterial as Qaterial
import "./QaterialHelper" as Qaterial

QScrollable {
	id: root

	property Downloader downloader: null

	contentCentered: true
	spacing: 30 * Qaterial.Style.pixelSizeRatio

	Qaterial.LabelBody2 {
		readonly property real _progress: downloader && downloader.fullSize > 0 ?
											  downloader.downloadedSize/downloader.fullSize :
											  0.

		anchors.horizontalCenter: parent.horizontalCenter
		color: Qaterial.Style.accentColor
		text: qsTr("Tartalom letöltése folyamatban...\n%1%").arg(downloader ? Math.floor(_progress*100.) : 0)
		horizontalAlignment: Text.AlignHCenter
	}

	Qaterial.ProgressBar
	{
		width: Math.min(250 * Qaterial.Style.pixelSizeRatio, parent.width*0.75)
		anchors.horizontalCenter: parent.horizontalCenter
		from: 0
		to: downloader ? downloader.fullSize : 0
		value: downloader ? downloader.downloadedSize : 0
		color: Qaterial.Colors.green400

		Behavior on value {
			NumberAnimation { duration: 200; easing.type: Easing.OutQuad }
		}
	}
}
