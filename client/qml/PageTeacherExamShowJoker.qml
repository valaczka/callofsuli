import QtQuick 2.15
import QtQuick.Controls 2.15
import Qaterial 1.0 as Qaterial
import "./QaterialHelper" as Qaterial
import CallOfSuli 1.0
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
					text: user.fullName

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
