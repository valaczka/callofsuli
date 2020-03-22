import QtQuick 2.15
import QtQuick.Controls 2.15
import QtGraphicalEffects 1.14
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS


QRectangleBg {
	id: control

	property UserDetails userDetails: null
	property bool userNameVisible: true
	readonly property real _spacing: userNameVisible ? 5 : 0
	property real maximumWidth: CosStyle.baseHeight*4

	implicitWidth: Math.min(userNameVisible ? CosStyle.baseHeight+Math.max(labelFirstname.implicitWidth, labelLastname.implicitWidth)+_spacing
											: CosStyle.baseHeight,
							maximumWidth)
	implicitHeight: Math.max(CosStyle.baseHeight, labelFirstname.implicitHeight+labelLastname.implicitHeight)

	acceptedButtons: Qt.LeftButton


	QProfileImage {
		id: image
		source: cosClient.userPicture
		rankId: cosClient.userRank
		rankImage: cosClient.userRankImage
		width: control.height
		height: control.width*0.85
		anchors.left: parent.left
		anchors.verticalCenter: parent.verticalCenter
	}

	Column {
		spacing: 0
		anchors.left: image.right
		anchors.leftMargin: _spacing
		anchors.right: parent.right
		anchors.verticalCenter: parent.verticalCenter

		visible: control.userNameVisible

		QLabel {
			id: labelFirstname
			width: parent.width
			font.capitalization: Font.MixedCase
			font.pixelSize: CosStyle.pixelSize*0.8
			color: CosStyle.colorPrimaryLight
			font.weight: Font.DemiBold
			elide: Text.ElideRight
			horizontalAlignment: Text.AlignLeft
			text: cosClient.userNickName.length ? cosClient.userNickName : cosClient.userFirstName
		}

		QLabel {
			id: labelLastname
			width: parent.width
			font.capitalization: Font.MixedCase
			font.pixelSize: CosStyle.pixelSize*0.8
			elide: Text.ElideRight
			color: CosStyle.colorPrimaryLight
			font.weight: Font.DemiBold
			horizontalAlignment: Text.AlignLeft
			text: cosClient.userNickName.length ? "" : cosClient.userLastName
			visible: text.length
		}
	}


	mouseArea.onClicked: {
		if (userDetails && !userDetails.running) {
			if (userDetails.opened)
				userDetails.close()
			else
				userDetails.open()
		}
	}
}

