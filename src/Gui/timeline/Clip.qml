import QtQuick 2.0

Rectangle {
    id: clip
    // NEVER SET X DIRECTLY. BINDING WILL BE REMOVED.
    x: ftop( position )
    width: ftop( end - begin + 1 )
    gradient: Gradient {
        GradientStop {
            id: gStop1
            position: 0.00;
            color: "#4276a6";
        }
        GradientStop {
            id: gStop2
            position: 1.00;
            color: "#1f546f";
        }
    }
    radius: 2
    border.color: "#1f546f"
    border.width: 1

    property alias name: text.text
    property int trackId
    // Usualy it is set -1. If not, the clip will be moved to the new track immediately.
    property int newTrackId
    property int position
    property int begin
    property int end
    property string uuid
    property string linkedClip // Uuid
    property bool linked: false
    property string type
    property bool selected: false

    property var clipInfo

    onClipInfoChanged: {
        if ( !clipInfo )
            return;

        position = clipInfo["position"];
        begin = clipInfo["begin"];
        end = clipInfo["end"];
        linkedClip = clipInfo["linkedClip"];
    }

    onPositionChanged: {
        clipInfo["position"] = position;
    }

    onBeginChanged: {
        clipInfo["begin"] = begin;
    }

    onEndChanged: {
        clipInfo["end"] = end;
    }

    onLinkedClipChanged: {
        clipInfo["linkedClip"] = linkedClip;
        if ( linkedClip ) {
            linked = true;
            var linkedClipItem = findClipItem( linkedClip );
            if ( linkedClipItem )
                linkedClipItem.linked = true;
        }
        else
            linked = false;
    }

    onLinkedChanged: {
        selectLinkedClip();

        if ( !linkedClip )
            return;

        if ( linked === true )
            findClipItem( linkedClip ).linked = true;
        else
            findClipItem( linkedClip ).linked = false;
    }

    function setPixelPosition( pixels )
    {
        if ( pixels >= 0 )
            position = ptof( pixels );
        // FIXME: Qt bug. Sometimes binding is lost.
        x = Qt.binding( function() { return ftop( position ); } );
    }

    function pixelPosition()
    {
        return ftop( position );
    }

    function move() {
        // This function updates Backend
        if ( newTrackId !== trackId )
            moveClipTo( track.type, uuid, newTrackId );
        else
            workflow.moveClip( trackId, uuid, position )
    }

    function resize() {
        // This function updates Backend
        workflow.resizeClip( uuid, begin, end, position )
    }

    function selectLinkedClip() {
        if ( selected === true && linked === true && linkedClip )
            findClipItem( linkedClip ).selected = true;
    }

    Component.onCompleted: {
        selected = true;
        newTrackId = trackId;
        allClips.push( clip );
    }

    Component.onDestruction: {
        Drag.drop();
        selected = false;

        for ( var i = 0; i < allClips.length; ++i ) {
            if ( allClips[i] === clip ) {
                allClips.splice( i, 1 );
                return;
            }
        }
    }

    Text {
        id: text
        color: "white"
        width: parent.width - 4
        height: trackHeight
        x: 4
        y: 4 - font.pointSize / 2
        font.pointSize: trackHeight / 4
        elide: Text.ElideRight
        wrapMode: Text.Wrap
    }

    Drag.keys: ["Clip"]
    Drag.active: dragArea.drag.active

    MouseArea {
        id: dragArea
        anchors.fill: parent
        drag.target: resizing ? null : parent
        drag.minimumX: 0
        acceptedButtons: Qt.LeftButton | Qt.RightButton
        hoverEnabled: true
        cursorShape: Qt.OpenHandCursor

        property bool resizing: false

        onPositionChanged: {
            // If it's too short, don't resize.
            if ( width < 6 ) {
                resizing = false;
                return;
            }

            if ( dragArea.pressed === true ) {
                // Handle resizing
                if ( resizing === true ) {
                    if ( mouseX < width / 2 ) {
                        var newPos = ptof( clip.x + mouseX );
                        var newBegin = begin + ( newPos - position );
                        if ( newBegin < 0 || newPos < 0 || newBegin >= end )
                            return;
                        begin = newBegin;
                        position = newPos;
                    }
                    else {
                        var newEnd = ptof( mouseX + ftop( begin ) );
                        if ( newEnd <= begin || newEnd + 1 > clipInfo["length"] )
                            return;
                        end = newEnd;
                    }
                }
            }
            else {
                if ( mouseX < 3 || ( clip.width - mouseX ) < 3 )
                    resizing = true;
                else
                    resizing = false;
            }
        }

        onPressed: {
            clip.Drag.hotSpot = Qt.point( mouseX, clip.height / 2 );

            if ( selected === true )
                return;

            if ( !( mouse.modifiers & Qt.ControlModifier ) )
                clearSelectedClips();

            if ( mouse.button & Qt.LeftButton )
                selected = true;
        }

        onClicked: {
            if ( mouse.button & Qt.RightButton ) {
                clipContextMenu.popup();
            }
        }

        onReleased: {
            if ( resizing === true )
                resize();
            else
                dragFinished();
        }

        states: [
            State {
                name: "Normal"
                when: !dragArea.pressed && !dragArea.resizing
                PropertyChanges { target: dragArea; cursorShape: Qt.OpenHandCursor }
            },
            State {
                name: "Resizing"
                when: dragArea.resizing
                PropertyChanges { target: dragArea; cursorShape: Qt.SizeHorCursor }
            },
            State {
                name: "Dragging"
                when: dragArea.pressed && !dragArea.resizing
                PropertyChanges { target: dragArea; cursorShape: Qt.ClosedHandCursor }
            }
        ]
    }

    ClipContextMenu {
        id: clipContextMenu
        clip: clip
    }

    onYChanged: {
        // Don't move outside its TrackContainer
        // For Top
        var yToMoveUp = track.mapToItem( container, 0, 0 ).y + y;
        if ( yToMoveUp < 0 )
            y -= yToMoveUp;
        // For Bottom
        if ( yToMoveUp + height > container.height )
            y -= yToMoveUp - container.height + height;
    }

    onSelectedChanged: {
        if ( selected === true ) {
            selectedClips.push( clip );

            var group = findGroup( uuid );
            for ( var i = 0; i < ( group ? group.length : 0 ); ++i ) {
                var clipItem = findClipItem( group[i] );
                if ( clipItem )
                    clipItem.selected = true;
            }
            selectLinkedClip();
        }
        else {
            for ( i = 0; i < selectedClips.length; ++i )
                if ( !selectedClips[i] || selectedClips[i] === clip ) {
                    selectedClips.splice( i, 1 );
                    --i;
                }
        }
    }

    states: [
        State {
            name: "Selected"
            when: selected
            PropertyChanges { target: gStop1; color: "#6498c8" }
            PropertyChanges { target: gStop2; color: "#427080" }
        },
        State {
            name: "Unselected"
            when: !selected
            PropertyChanges { target: gStop1; color: "#4276a6" }
            PropertyChanges { target: gStop2; color: "#1f546f" }
        }
    ]

    transitions: Transition {
        PropertyAnimation {
            properties: "color"
            easing.type: Easing.InOutQuad
            duration: 100
        }
    }
}

