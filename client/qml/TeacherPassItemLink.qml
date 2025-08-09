import QtQuick
import QtQuick.Controls
import SortFilterProxyModel
import CallOfSuli
import Qaterial as Qaterial
import "./QaterialHelper" as Qaterial
import "JScript.js" as JS

Row {
	id: control

	property int groupid: -1
	property PassItem passItem: null
	property string linkTitle: ""
	property bool canAdd: groupid >= 0
	property int passitemid: passItem ? passItem.itemid : -1

	signal reloaded()
	signal selected(int itemid)
	signal unlinked()

	property var unlink: function(_id) {
		if (_id <= 0) {
			console.error("Invalid id")
			return
		}

		Client.send(HttpConnection.ApiTeacher, "passItem/%1/unlink".arg(_id))
		.done(control, function(r){
			control.unlinked()
		})
		.fail(control, function(err) {
			Client.messageError(err, qsTr("Link megszüntetése sikertelen"))
		})
	}


	spacing: 10

	Qaterial.LabelBody1 {
		anchors.verticalCenter: parent.verticalCenter
		text: qsTr("Link:")
	}


	Qaterial.LabelSubtitle1 {
		anchors.verticalCenter: parent.verticalCenter

		color: Qaterial.Style.iconColor()

		width: Math.min (parent.width
						 - (_btnAdd.visible ? parent.spacing + _btnAdd.width : 0)
						 - (_btnClear.visible ? parent.spacing + _btnClear.width : 0),
						 Math.max(implicitWidth, 350))

		elide: implicitWidth > width ? Text.ElideRight : Text.ElideNone

		text: passItem ?
				  ((passItem.linkTitle != "" ? passItem.linkTitle : qsTr("#%1").arg(passItem.linkId))
				   + (passItem.linkType == PassItem.LinkCampaign
					  ? qsTr(" [kihívás]") :
						passItem.linkType == PassItem.LinkExam
						? qsTr(" [dolgozat]") :
						  " [???]")
				   ) : linkTitle
	}


	Qaterial.ToolButton {
		id: _btnAdd
		visible: canAdd
		anchors.verticalCenter: parent.verticalCenter
		icon.source: Qaterial.Icons.linkVariantPlus
		display: AbstractButton.IconOnly
		outlined: true
		ToolTip.text: qsTr("Link létrehozása")
		onClicked: download()
	}


	Qaterial.ToolButton {
		id: _btnClear
		visible: passitemid > 0
		anchors.verticalCenter: parent.verticalCenter
		icon.source: Qaterial.Icons.linkVariantRemove
		display: AbstractButton.IconOnly
		outlined: true
		ToolTip.text: qsTr("Link megszüntetése")
		onClicked: {
			JS.questionDialog(
						{
							onAccepted: function()
							{
								control.unlink(passitemid)
							},
							text: qsTr("Biztosan megszünteted a linket?"),
							title: qsTr("Link megszüntetése"),
							iconSource: Qaterial.Icons.linkVariantRemove
						})
		}
	}

	SortFilterProxyModel {
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


	function download() {
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

	Component {
		id: _delegate

		Qaterial.ItemDelegate
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

}
