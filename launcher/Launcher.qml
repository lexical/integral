import Qt 4.7
import UnityApplications 1.0

Item {
    width: 58
    height: 1024

    Image {
        id: background

        anchors.fill: parent
        fillMode: Image.TileVertically
        source: "/usr/share/unity/themes/launcher_background_middle.png"
    }

    ListView {
        id: list

        anchors.fill: parent
        focus: true

        model: ListAggregatorModel {
            id: items
        }

        delegate: LauncherItem {
            id: wrapper

            width: 58; height: 54
            icon: "image://icons/"+item.icon
            running: item.running
            active: item.active
            urgent: item.urgent
            launching: item.launching
            onClicked: {
                if (mouse.button == Qt.LeftButton) {
                    item.hideMenu()
                    item.activate()
                }
                else if (mouse.button == Qt.RightButton) {
                    item.showMenu()
                }
            }

            /* Display the tooltip when hovering the item only when the list
               is not moving */
            onEntered: if (!list.moving) item.showTooltip(y + height / 2)
            onExited: item.hideMenu()
            Connections {
                target: list
                onMovementStarted: item.hideMenu()
            }

            ListView.onAdd: SequentialAnimation {
                PropertyAction { target: wrapper; property: "scale"; value: 0 }
                NumberAnimation { target: wrapper; property: "height"; from: 0; to: 54; duration: 250; easing.type: Easing.InOutQuad }
                NumberAnimation { target: wrapper; property: "scale"; to: 1; duration: 250; easing.type: Easing.InOutQuad }
            }

            ListView.onRemove: SequentialAnimation {
                PropertyAction { target: wrapper; property: "ListView.delayRemove"; value: true }
                NumberAnimation { target: wrapper; property: "scale"; to: 0; duration: 250; easing.type: Easing.InOutQuad }
                NumberAnimation { target: wrapper; property: "height"; to: 0; duration: 250; easing.type: Easing.InOutQuad }
                PropertyAction { target: wrapper; property: "ListView.delayRemove"; value: false }
            }
        }
    }

   LauncherApplicationsList {
        id: applications
    }

    LauncherPlacesList {
        id: places
    }

    LauncherDevicesList {
        id: devices
    }

    Trashes {
        id: trashes
    }

    Component.onCompleted: {
        items.appendModel(applications);
        items.appendModel(places);
        items.appendModel(devices);
        items.appendModel(trashes);
    }

    Connections {
        target: launcherView
        onDesktopFileDropped: applications.insertFavoriteApplication(path)
    }
}
