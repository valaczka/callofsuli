import QtQuick 2.15
import QtQuick.Controls 2.15
import QtGraphicalEffects 1.14
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS

QToolButton {
	id: control

	property UserDetails userDetails: null
	property bool userNameVisible: true

	width: Math.min(implicitWidth, 200)

	display: userNameVisible ? Button.TextBesideIcon : Button.IconOnly

	icon.source: cosClient.rankImageSource(cosClient.userRank, cosClient.userRankLevel, cosClient.userRankImage)

	icon.color: "transparent"
	icon.width: CosStyle.pixelSize*1.9
	icon.height: CosStyle.pixelSize*1.9

	ToolTip.text: ""

	topPadding: 0
	bottomPadding: 0

	font.capitalization: Font.MixedCase

	font.pixelSize: CosStyle.pixelSize*0.8

	text: cosClient.userNickName.length ? cosClient.userNickName :
										  cosClient.userFirstName+(cosClient.userLastName.length ? "\n"+cosClient.userLastName : "")

	onClicked: {
		if (userDetails && !userDetails.running) {
			if (userDetails.opened)
				userDetails.close()
			else
				userDetails.open()
		}
	}
}
