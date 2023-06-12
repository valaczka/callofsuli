import QtQuick 2.15
import QtQuick.Controls 2.15
import Qaterial 1.0 as Qaterial
import "./QaterialHelper" as Qaterial
import CallOfSuli 1.0
import "JScript.js" as JS

Qaterial.Expandable {
	id: root

	property alias username: _userLog.username

	property bool _firstRun: true

	expanded: true

	header: QExpandableHeader {
		text: qsTr("Előléptetések")
		icon: Qaterial.Icons.medalOutline
		expandable: root
	}

	delegate: Column {
		width: root.width

		Repeater {
			model: _firstRun ? 5 : _userLog.model

			delegate: _firstRun ? _cmpPlacholder : _cmpDelegate
		}
	}


	Component {
		id: _cmpDelegate

		QLoaderItemFullDelegate {
			id: _delegate

			spacing: 10
			leftPadding: 0
			rightPadding: 0

			width: root.width

			height: Qaterial.Style.textTheme.body2.pixelSize*2 + topPadding+bottomPadding+topInset+bottomInset

			highlighted: modelData && modelData.rank === undefined

			readonly property color _color: modelData && modelData.rank !== undefined ? Qaterial.Style.primaryTextColor() : Qaterial.Style.accentColor

			contentSourceComponent: Label {
				font: Qaterial.Style.textTheme.body2
				verticalAlignment: Label.AlignVCenter
				/*text: game && game.user ? (root.positionType == MapPlayMissionLevelInfoList.Position.Duration ? game.posDuration : game.posSolved)+". "
									  +game.user.fullNickName : ""
			color: root.mainColor*/

				color: _delegate._color

				text: modelData.rank !== undefined
					  ? (modelData.rank.sublevel > 0 ? qsTr("%1 (level %2)").arg(modelData.rank.name).arg(modelData.rank.sublevel) : modelData.rank.name)
					  : "Streak: "+modelData.streak
			}

			leftSourceComponent: UserImage {
				visible: modelData && modelData.rank !== undefined
				userData: modelData && modelData.rank !== undefined ? modelData : null
				pictureEnabled: false
				sublevelEnabled: true
				height: _delegate.height-6
				width: _delegate.height-6
			}

			rightSourceComponent: Qaterial.LabelCaption {
				text: JS.readableDate(modelData.timestamp)
				color: _delegate._color
			}
		}
	}

	Component {
		id: _cmpPlacholder
		QLoaderItemFullDelegate {
			id: _delegatePlaceholder

			width: root.width

			height: Qaterial.Style.textTheme.body2.pixelSize*2 + topPadding+bottomPadding+topInset+bottomInset

			spacing: 10
			leftPadding: 0
			rightPadding: 0

			contentSourceComponent: QPlaceholderItem {
				horizontalAlignment: Qt.AlignLeft
				heightRatio: 0.5
			}

			leftSourceComponent: QPlaceholderItem {
				contentComponent: ellipseComponent
				fixedHeight: _delegatePlaceholder.height-6
				fixedWidth: fixedHeight
			}

			rightSourceComponent: QPlaceholderItem {
				width: 90
				height: Qaterial.Style.textTheme.caption.pixelSize
				heightRatio: 1.0
				widthRatio: 1.0
			}
		}
	}



	UserLogListImpl {
		id: _userLog

		onModelReloaded: _firstRun = false
	}


	function reload() {
		_userLog.reload()
	}
}
