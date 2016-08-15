import QtQuick 2.0

Item {
    Image{
        anchors.centerIn: parent
        id:footprint
        source:"images/footprint.png"
    }
    function img_width(){
        return footprint.width;
    }
    function img_height(){
        return footprint.height;
    }
}
