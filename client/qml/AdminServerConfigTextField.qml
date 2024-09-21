import QtQuick
import QtQuick.Controls
import CallOfSuli
import Qaterial as Qaterial
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
				Client.send(HttpConnection.ApiAdmin, "config", d)
				.done(root, function(r){
					saved()
				})
				.fail(root, function(err) {
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
