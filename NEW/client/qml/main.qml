import QtQuick 2.12
import QtQuick.Controls 2.12
import Qaterial 1.0 as Qaterial
import Box2D 2.0

Qaterial.ApplicationWindow
{
	id: window
	width: 480
	height: 750
	visible: true
	title: "Qaterial Gallery"

	menuBar: Qaterial.MenuBar
	{
		visible: true

		Qaterial.Menu
		{
			title: qsTr("File")
			width: 300
			Qaterial.MenuItem
			{
				text: qsTr("New...");onTriggered: console.log(
													  "New");
				action: Action { shortcut: "Ctrl+N" }
			} // MenuItem
			Qaterial.MenuItem { text: qsTr("Open...");onTriggered: console.log("Open") } // MenuItem
			Qaterial.MenuItem { text: qsTr("Save");onTriggered: console.log("Save") } // MenuItem
			Qaterial.MenuItem { text: qsTr("Save As...");onTriggered: console.log("Save As") } // MenuItem
			Qaterial.MenuSeparator { width: parent.width } // MenuItem
			Qaterial.MenuItem { text: qsTr("Quit");onTriggered: console.log("Quit") } // MenuItem
		} // Menu

		Qaterial.Menu
		{
			width: 300
			title: qsTr("Edit")
			Qaterial.MenuItem
			{
				text: qsTr("Copy");
				//icon.source: "qrc:/QaterialGallery/images/icons/content-copy.svg";
				action: Action
				{
					shortcut: "Ctrl+C"
					onTriggered: console.log("Copy")
				}
			} // MenuItem
			Qaterial.MenuItem { text: qsTr("Cut");onTriggered: console.log("Cut") } // MenuItem
			Qaterial.MenuItem { text: qsTr("Paste");onTriggered: console.log("Paster") } // MenuItem

			Qaterial.Menu
			{
				title: "Find/Replace"
				Qaterial.MenuItem { text: "Find Next" } // MenuItem
				Qaterial.MenuItem { text: "Find Previous" } // MenuItem
				Qaterial.MenuItem { text: "Replace" } // MenuItem
			} // Menu

			Qaterial.MenuSeparator { width: parent.width } // MenuSeperator

			Qaterial.MenuItem
			{
				text: qsTr("Dummy");
				//icon.source: "qrc:/QaterialGallery/images/icons/airplane.svg";
				action: Action
				{
					shortcut: "Ctrl+Shift+F5"
					onTriggered: console.log("Dummy")
				}
			} // MenuItem
			Qaterial.MenuItem
			{
				text: qsTr("Colored Icon");
				//icon.source: "qrc:/QaterialGallery/images/icons/album.svg";
				icon.color: "#8BC34A"
				action: Action
				{
					shortcut: "Ctrl+K,Ctrl+L"
					onTriggered: console.log("Colored")
				}
			} // MenuItem
		} // Menu

		Qaterial.Menu
		{
			title: qsTr("Test")
			Qaterial.MenuItem { text: qsTr("Checked 1 very loing afznuaefb");checked: true } // MenuItem
			Qaterial.MenuItem { text: qsTr("Checked 2");checkable: true } // MenuItem
			Qaterial.MenuItem { text: qsTr("Checked 3");checkable: true } // MenuItem
			Qaterial.MenuItem { text: qsTr("Checked 4");checkable: true } // MenuItem
			Qaterial.MenuItem { text: qsTr("Checked 5");checkable: true } // MenuItem
		} // Menu

		Qaterial.Menu
		{
			title: qsTr("Help")
			Qaterial.MenuItem { text: qsTr("About");onTriggered: console.log("About") } // MenuItem
		} // Menu
	} // MenuBar

	Component.onCompleted:
	{
		//Style.theme = Material.Dark
	}

	Shortcut
	{
		sequences: ["Esc", "Back"]
		enabled: stackView.depth > 1
		onActivated:
		{
			stackView.pop()
		}
	} // Shortcut

	Qaterial.StackView
	{
		id: stackView
		anchors.fill: parent

		initialItem: Rectangle {
			id: screen

			width: 800
			height: 600

			readonly property int wallMeasure: 40
			readonly property int ballDiameter: 20
			readonly property int minBallPos: Math.ceil(wallMeasure)
			readonly property int maxBallPos: Math.floor(screen.width - (wallMeasure + ballDiameter))

			Slider {
				id: lengthSlider
				x: 180
				y: 50
				width: 100
				height: 50
				from: 20
				to: 50
				value: 30
			}

			Component {
				id: linkComponent
				PhysicsItem {
					id: ball

					width: 20
					height: 20
					bodyType: Body.Dynamic

					property color color: "#EFEFEF"

					fixtures: Circle {
						radius: ball.width / 2
						density: 0.5
					}

					Rectangle {
						radius: parent.width / 2
						border.color: "blue"
						color: parent.color
						width: parent.width
						height: parent.height
						smooth: true
					}
				}
			}

			Component {
				id: jointComponent
				RopeJoint {
					localAnchorA: Qt.point(10,10)
					localAnchorB: Qt.point(10,10)
					maxLength: lengthSlider.value
					collideConnected: true
				}
			}

			World { id: physicsWorld }

			Component.onCompleted: {
				/*var prev = leftWall;
				for (var i = 60;i < 740;i += 20) {
					var newLink = linkComponent.createObject(screen);
					newLink.color = "orange";
					newLink.x = i;
					newLink.y = 100;
					var newJoint = jointComponent.createObject(screen);
					if (i === 60)
						newJoint.localAnchorA = Qt.point(40, 100);
					newJoint.bodyA = prev.body;
					newJoint.bodyB = newLink.body;
					prev = newLink;
				}
				newJoint = jointComponent.createObject(screen);
				newJoint.localAnchorB = Qt.point(0,100);
				newJoint.bodyA = prev.body;
				newJoint.bodyB = rightWall.body;*/
			}

			PhysicsItem {
				id: ground
				height: 40
				anchors {
					left: parent.left
					right: parent.right
					bottom: parent.bottom
				}
				fixtures: Box {
					width: ground.width
					height: ground.height
					friction: 1
					density: 1
				}
				Rectangle {
					anchors.fill: parent
					color: "#DEDEDE"
				}
			}



			Rectangle {
				id: debugButton
				x: 50
				y: 50
				width: 120
				height: 30
				Text {
					text: "Debug view: " + (debugDraw.visible ? "on" : "off")
					anchors.centerIn: parent
				}
				color: "#DEDEDE"
				border.color: "#999"
				radius: 5
				MouseArea {
					anchors.fill: parent
					onClicked: debugDraw.visible = !debugDraw.visible;
				}
			}

			DebugDraw {
				id: debugDraw
				world: physicsWorld
				opacity: 1
				visible: false
			}

			function xPos() {
				return (Math.floor(Math.random() * (maxBallPos - minBallPos)) + minBallPos)
			}

			Timer {
				id: ballsTimer
				interval: 500
				running: true
				repeat: true
				onTriggered: {
					var newBox = linkComponent.createObject(screen);
					//newBox.x = xPos()
					newBox.y = 50;
				}
			}
		}
	} // StackView

}

