import QtQuick 2.15
import QtQuick.Controls 2.15
import COS.Client 1.0
import "."
import "Style"

Column {
	id: control
	spacing: 10

	topPadding: 20
	bottomPadding: 20


	property alias userName: labelName.text

	property string picture: ""
	property int rankId: -1
	property string rankImage: ""
	property int rankLevel: -1

	property string rankName: ""

	Row {
		id: row
		anchors.horizontalCenter: parent.horizontalCenter

		spacing: 15

		QProfileImage {
			id: image
			source: picture //cosClient.userPicture
			rankId: picture == "" ? rankId : -1 //cosClient.userRank
			rankImage: picture == "" ? rankImage : "" //cosClient.userRankImage
			width: source != "" ? 75 : 0
			height: 75

			anchors.verticalCenter: parent.verticalCenter
		}

		QLabel {
			id: labelName
			font.pixelSize: CosStyle.pixelSize*1.7
			font.weight: Font.Normal
			color: CosStyle.colorAccentLight

			anchors.verticalCenter: parent.verticalCenter

			elide: Text.ElideRight
			width: Math.min(implicitWidth, control.width-row.spacing-image.width)
		}
	}

	Row {
		anchors.horizontalCenter: parent.horizontalCenter

		spacing: 10

		Image {
			source: cosClient.rankImageSource(rankId, rankLevel, rankImage)

			width: CosStyle.pixelSize*1.6
			height: CosStyle.pixelSize*1.6

			fillMode: Image.PreserveAspectFit

			anchors.verticalCenter: parent.verticalCenter
		}

		QLabel {
			anchors.verticalCenter: parent.verticalCenter
			font.pixelSize: CosStyle.pixelSize*1.3
			font.weight: Font.Normal
			color: CosStyle.colorAccentLighter

			horizontalAlignment: Text.AlignHCenter

			elide: Text.ElideRight
			width: Math.min(implicitWidth, control.width)

			text: rankName+(rankLevel > 0 ? "<br><small>level "+rankLevel+"</small>" : "")
			textFormat: Text.RichText
		}
	}
}
