import QtQuick 2.0

Item {
    Image{
        id:grid_empty;
        source:"images/grid.png"
    }
    Image{
        id:grid_fill;
        source:"images/grid_fill.png"
    }
    function setMarked(b){
        grid_fill.visible=b;
        grid_empty.visible=!b;
    }
    function width(){
        return grid_fill.width;
    }
    function height(){
        return grid_fill.height;
    }

}
