import QtQuick 2.15
import QtQuick.Controls 2.15
import Qaterial 1.0 as Qaterial
import "./QaterialHelper" as Qaterial
import CallOfSuli 1.0
import "JScript.js" as JS

Item {
	id: control

	property Campaign campaign: null

	signal reloadRequest()

	QScrollable {
		anchors.fill: parent
		contentCentered: true

		spacing: 10 * Qaterial.Style.pixelSizeRatio

		QButton {
			icon.source: Qaterial.Icons.play
			text: qsTr("Start")
			anchors.horizontalCenter: parent.horizontalCenter

			enabled: campaign
			visible: campaign && campaign.state < Campaign.Running
			onClicked: {
				Client.send(WebSocket.ApiTeacher, "campaign/%1/run".arg(campaign.campaignid))
				.done(function(r){
					reloadRequest()
				})
				.fail(JS.failMessage(qsTr("Hadjárat indítása sikertelen")))
			}
		}

		Qaterial.LabelBody1 {
			id: _labelStart
			color: Qaterial.Colors.lightGreen400
			width: parent.width
			horizontalAlignment: Qt.AlignHCenter
			leftPadding: 20
			rightPadding: 20
			wrapMode: Text.Wrap
			visible: _startTimer.running

			Timer {
				id: _startTimer
				interval: 500
				triggeredOnStart: true
				running: campaign && campaign.startTime.getTime() && campaign.state < Campaign.Running
				repeat: true
				onTriggered: {
					var diff = campaign.startTime.getTime() - new Date().getTime()
					if (diff <= 0) {
						stop()
						return
					}

					diff = Math.floor(diff/1000)

					var s = diff%60
					diff = Math.floor(diff/60)
					var m = diff%60
					diff = Math.floor(diff/60)
					var h = diff%24
					diff = Math.floor(diff/24)

					var list = []

					if (diff > 0)
						list.push(qsTr("%1 nap").arg(diff))
					if (h > 0 || diff > 0)
						list.push(qsTr("%1 óra").arg(h))
					if (m > 0 || h > 0)
						list.push(qsTr("%1 perc").arg(m))
					if (s > 0 || m > 0)
						list.push(qsTr("%1 másodperc").arg(s))

					_labelStart.text = qsTr("Indításig hátralévő idő:\n")+list.join(" ")
				}
			}
		}

		Row {
			id: _row
			anchors.horizontalCenter: parent.horizontalCenter
			spacing: 5 * Qaterial.Style.pixelSizeRatio
			visible: campaign && campaign.state >= Campaign.Running

			Qaterial.Icon
			{
				id: _icon
				anchors.verticalCenter: parent.verticalCenter
				icon: campaign && campaign.state == Campaign.Finished ? Qaterial.Icons.checkBold : Qaterial.Icons.play
				color: Qaterial.Colors.lightGreen400
				size: 48 * Qaterial.Style.pixelSizeRatio
				width: size
				height: size

			}

			Qaterial.LabelHeadline4
			{
				id: _label
				text: campaign && campaign.state == Campaign.Finished ? qsTr("A hadjárat véget ért") : qsTr("A hadjárat folyamatban van")
				color: Qaterial.Colors.lightGreen400
				elide: Text.ElideNone
				width: Math.min(implicitWidth, _row.parent.width-_icon.width-_row.spacing)
				wrapMode: implicitWidth > width ? Text.Wrap : Text.NoWrap
				horizontalAlignment: Text.AlignHCenter
			}
		}



		Qaterial.LabelBody1 {
			id: _labelEnd
			color: Qaterial.Style.accentColor
			width: parent.width
			horizontalAlignment: Qt.AlignHCenter
			leftPadding: 20
			rightPadding: 20
			wrapMode: Text.Wrap
			visible: _endTimer.running

			Timer {
				id: _endTimer
				interval: 500
				triggeredOnStart: true
				running: campaign && campaign.endTime.getTime() && campaign.state == Campaign.Running
				repeat: true
				onTriggered: {
					var diff = campaign.endTime.getTime() - new Date().getTime()
					if (diff <= 0) {
						stop()
						return
					}

					diff = Math.floor(diff/1000)

					var s = diff%60
					diff = Math.floor(diff/60)
					var m = diff%60
					diff = Math.floor(diff/60)
					var h = diff%24
					diff = Math.floor(diff/24)

					var list = []

					if (diff > 0)
						list.push(qsTr("%1 nap").arg(diff))
					if (h > 0 || diff > 0)
						list.push(qsTr("%1 óra").arg(h))
					if (m > 0 || h > 0)
						list.push(qsTr("%1 perc").arg(m))
					if (s > 0 || m > 0)
						list.push(qsTr("%1 másodperc").arg(s))

					_labelEnd.text = qsTr("Befejezésig hátralévő idő:\n")+list.join(" ")
				}
			}
		}

		QButton {
			anchors.horizontalCenter: parent.horizontalCenter
			icon.source: Qaterial.Icons.stop
			text: qsTr("Leállítás")
			enabled: campaign && campaign.state == Campaign.Running
			visible: campaign && campaign.state < Campaign.Finished
			onClicked: {
				JS.questionDialog({
									  onAccepted: function()
									  {
										  Client.send(WebSocket.ApiTeacher, "campaign/%1/finish".arg(campaign.campaignid))
										  .done(function(r){
											  reloadRequest()
										  })
										  .fail(JS.failMessage(qsTr("Hadjárat befejezése sikertelen")))
									  },
									  title: qsTr("Hadjárat befejezése"),
									  text: qsTr("Biztosan befejezed a hadjáratot?")
								  })

			}
		}

	}
}
