import QtQuick 2.12
import QtQuick.Controls 2.14
import QtGraphicalEffects 1.14
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS

QToolButton {
	id: control

	property UserDetails userDetails: null
	property bool userNameVisible: true

	display: userNameVisible ? Button.TextBesideIcon : Button.IconOnly

	icon.source: cosClient.userRankImage.length ?
					 "image://sql/"+cosClient.userRankImage :
					 "image://sql/rank/"+cosClient.userRank+".svg"
	icon.color: "transparent"
	icon.width: CosStyle.pixelSize*1.9
	icon.height: CosStyle.pixelSize*1.9

	ToolTip.text: ""

	topPadding: 0
	bottomPadding: 0

	font.capitalization: Font.MixedCase

	font.pixelSize: CosStyle.pixelSize*0.8

	text: cosClient.userFirstName+"\n"+cosClient.userLastName

	onClicked: {
		if (userDetails && !userDetails.running) {
			if (userDetails.opened)
				userDetails.close()
			else
				userDetails.open()
		}
	}
}
