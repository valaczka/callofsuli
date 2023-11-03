import QtQuick
import QtQuick.Controls
import Qaterial as Qaterial
import "./QaterialHelper" as Qaterial
import CallOfSuli
import SortFilterProxyModel
import "JScript.js" as JS

Column {
	id: root

	property var userData: null

	spacing: 15

	Qaterial.LabelHeadline3 {
		anchors.horizontalCenter: parent.horizontalCenter
		width: Math.min(parent.width-100, Qaterial.Style.maxContainerSize)
		topPadding: root.paddingTop
		horizontalAlignment: Qt.AlignHCenter
		text: userData ? userData.fullNickName : ""
		wrapMode: Text.Wrap
		maximumLineCount: 2
		elide: Text.ElideRight
	}

	Row {
		spacing: 10
		anchors.horizontalCenter: parent.horizontalCenter
		UserImage {
			userData: root.userData
			iconColor: Qaterial.Style.colorTheme.primaryText
			width: Qaterial.Style.pixelSize*2.5
			height: Qaterial.Style.pixelSize*2.5
			pictureEnabled: false
			anchors.verticalCenter: parent.verticalCenter
		}

		Column {
			anchors.verticalCenter: parent.verticalCenter
			Qaterial.LabelHeadline6 {
				anchors.left: parent.left
				text: userData ? userData.rank.name : ""
			}
			Qaterial.LabelBody2 {
				anchors.left: parent.left
				text: userData && userData.rank.sublevel > 0 ? qsTr("level %1").arg(userData.rank.sublevel) : ""
			}
		}
	}
}


