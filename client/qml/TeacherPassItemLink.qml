import QtQuick
import SortFilterProxyModel
import CallOfSuli
import Qaterial as Qaterial
import "./QaterialHelper" as Qaterial
import "JScript.js" as JS

QtObject {
	id: control

	property int groupid: -1

	signal reloaded()
	signal selected(int itemid)

	// SELECT passItem.id, passItem.description, passid, pass.title FROM passItem "

	readonly property SortFilterProxyModel model: SortFilterProxyModel {
		id: _sorted

		sourceModel: ListModel {
			id: _model
		}

		sorters: [
			RoleSorter {
				roleName: "passid"
				priority: 2
				sortOrder: Qt.AscendingOrder
			},
			RoleSorter {
				roleName: "id"
				priority: 1
				sortOrder: Qt.AscendingOrder
			}
		]
	}


	readonly property var download: function () {
		if (groupid <= 0) {
			console.error("Invalid groupid")
			return
		}

		Client.send(HttpConnection.ApiTeacher, "group/%1/passItems".arg(groupid))
		.done(control, function(r){
			_model.clear()

			for (let i=0; i<r.list.length; ++i)
				_model.append(r.list[i])

			control.reloaded()
		})
		.fail(control, function(err) {
			Client.messageError(err, qsTr("Letöltés sikertelen"))
		})
	}

	onReloaded: {
		Qaterial.DialogManager.openListView(
					{
						onAccepted: function(index)
						{
							if (index < 0)
								return

							console.debug("index", index, _sorted.get(index).id)

							control.selected(_sorted.get(index).id)
						},
						title: qsTr("Call Pass link"),
						model: _sorted,
						delegate: _delegate
					})
	}

	readonly property Component _delegate: Qaterial.ItemDelegate
	{
		required property string description
		required property string title
		required property int id
		required property int passid
		required property int index

		text: description != "" ? description : qsTr("Elem #%1").arg(id)
		secondaryText: title != "" ? title : qsTr("Call Pass #%1").arg(passid)
		width: ListView.view.width
		onClicked: ListView.view.select(index)
	}

}
