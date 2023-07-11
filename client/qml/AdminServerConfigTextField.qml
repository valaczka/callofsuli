import QtQuick 2.15
import QtQuick.Controls 2.15
import CallOfSuli 1.0
import Qaterial 1.0 as Qaterial
import "./QaterialHelper" as Qaterial


Qaterial.TextField {
	id: root

	width: parent.width

	property string field: ""

	trailingContent: Qaterial.TextFieldButtonContainer
	{
		QTextFieldInPlaceButtons {
			id: _ip

			property string field: root.field
			readonly property string getData: text

			onSaveRequest: {
				var d = {}
				d[field] = getData
				Client.send(WebSocket.ApiAdmin, "config", d)
				.done(function(r){
					saved()
				})
				.fail(function(err) {
					Client.messageWarning(err, qsTr("Módosítás sikertelen"))
					revert()
				})
			}

			onActiveChanged: checkModified()

			Component.onCompleted: {
				_items.push(_ip)
				if (field === "serverName")
					set(Client.server.serverName)
				if (field !== "" && _data[field] !== undefined) {
					set(_data[field])
				}
			}
		}
	}
}
