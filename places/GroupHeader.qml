import Qt 4.7

Button {
    property alias icon: icon.source
    property alias label: label.text
    property bool folded: true
    property bool moreAvailable


    Image {
        id: icon

        width: 22
        height: 22
        anchors.verticalCenter: parent.verticalCenter
        anchors.left: parent.left
        anchors.leftMargin: 6
        fillMode: Image.PreserveAspectFit
        sourceSize.width: width
        sourceSize.height: height
    }

    TextCustom {
        id: label

        color: parent.state == "pressed" ? "#444444" : "#ffffff"
        font.pixelSize: 16
        anchors.top: parent.top
        anchors.topMargin: 5
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 7
        anchors.left: icon.right
        anchors.leftMargin: 5
        width: paintedWidth
    }

    Image {
        id: arrow

        visible: moreAvailable
        rotation: folded ? 0 : 90
        source: "artwork/arrow.png"
        width: 7
        height: 7
        anchors.verticalCenter: parent.verticalCenter
        anchors.left: label.right
        anchors.leftMargin: 6
        fillMode: Image.PreserveAspectFit
        sourceSize.width: width
        sourceSize.height: height
    }

    Rectangle {
        id: underline

        opacity: parent.state == "selected" ? 0.0 : 1.0
        Behavior on opacity {NumberAnimation {duration: 100}}

        height: 1
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.leftMargin: 2
        anchors.right: parent.right
        anchors.rightMargin: 2

        color: "#4cffffff"
    }
}
