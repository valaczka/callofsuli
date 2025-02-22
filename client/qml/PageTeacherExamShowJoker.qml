import QtQuick
import QtQuick.Controls
import Qaterial as Qaterial
import "./QaterialHelper" as Qaterial
import CallOfSuli
import "JScript.js" as JS

QPage {
	id: root

	property var userList: null				// QList<ExamUser*>
	readonly property int count: userList ? userList.length : 0

	title: qsTr("Joker lehetőség")

	appBar.backButtonVisible: true

	Grid {
		id: _grid
		anchors.fill: parent

		flow: Grid.TopToBottom

		columns: {
			if (root.height > root.width)
				return root.count > 20 ? 2 : 1
			else if (root.count > 10)
				return 3
			else
				return 2
		}

		rows: Math.ceil(root.count/columns)

		Repeater {
			model: userList

			delegate: Rectangle {
				property ExamUser user: modelData

				width: _grid.width/_grid.columns
				height: _grid.height/_grid.rows

				color: "transparent"

				Qaterial.Label {
					anchors.fill: parent

					readonly property bool _canAdd: user.jokerOptions & ExamUser.JokerCanAdd
					readonly property bool _canDeny: user.jokerOptions & ExamUser.JokerCanDeny

					text: user.fullName + " ("
						  + (_canAdd ? "+" : "")
						  + (_canDeny ? "−" : "")
						  + ")"


					font.family: Qaterial.Style.textTheme.headline1.family
					font.pixelSize: Math.min(85*Qaterial.Style.pixelSizeRatio, parent.height*0.8/2)
					font.weight: Font.Bold
					lineHeight: 0.8

					horizontalAlignment: Text.AlignHCenter
					verticalAlignment: Text.AlignVCenter

					wrapMode: Text.Wrap

					color: Qaterial.Style.iconColor()

					leftPadding: 10
					rightPadding: 10
					topPadding: 5
					bottomPadding: 5
				}
			}
		}
	}
}
