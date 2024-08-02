import QtQuick 2.15
import QtQuick.Controls 2.15
import Qaterial 1.0 as Qaterial
import "./QaterialHelper" as Qaterial
import CallOfSuli 1.0
import "JScript.js" as JS

QFormColumn {
	id: _form

	//spacing: 3

	property string username: ""
	property var _active: []

	ListModel {
		id: _model

		ListElement {
			text: qsTr("Új kihívás indításakor")
			value: NotificationType.NotificationStarted
		}

		ListElement {
			text: qsTr("Kihívás vége előtt 1 héttel")
			value: NotificationType.NotificationWeek1
		}

		ListElement {
			text: qsTr("Kihívás vége előtt 48 órával")
			value: NotificationType.NotificationHour48
		}

		ListElement {
			text: qsTr("Kihívás vége előtt 24 órával")
			value: NotificationType.NotificationHour24
		}
	}


	Repeater {
		id: _rptr
		model: _model

		delegate: QFormSwitchButton {
			text: model.text
			checked: _active.includes(model.value)
			enabled: username != ""
		}
	}


	QButton
	{
		anchors.horizontalCenter: parent.horizontalCenter
		text: qsTr("Mentés")
		icon.source: Qaterial.Icons.contentSave
		enabled: _form.modified && username != ""

		onClicked:
		{
			let lEnable = []
			let lDisable = []

			_form.enabled = false

			for (let i=0; i<_model.count; ++i) {
				let m = _model.get(i)
				let b = _rptr.itemAt(i)

				if (b.checked)
					lEnable.push(m.value)
				else
					lDisable.push(m.value)
			}

			Client.send(HttpConnection.ApiUser, "notification/update", {
							"enable": lEnable,
							"disable": lDisable
						})
			.done(_form, function(r){
				_form.enabled = true
				Client.snack(qsTr("Módosítás sikeres"))
				loadData()
			})
			.fail(_form, function(err) {
				Client.messageWarning(err, qsTr("Módosítás sikertelen"))
				_form.enabled = true
			})
			.error(_form, function(err) {
				_form.enabled = true
			})

		}
	}

	function loadData() {
		if (username == "") {
			_form.modified = false
			return
		}

		Client.send(HttpConnection.ApiUser, "notification")
		.done(_form, function(r){
			_active = r.list
			_form.modified = false
		})
	}

	//Component.onCompleted: loadData()
	onUsernameChanged: loadData()
}


