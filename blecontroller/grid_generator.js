.pragma library // Shared game state
.import QtQuick 2.5 as QQ

var grid_stack=[];

function generateGridStack(window) {
    var comp = Qt.createComponent("MapGrid.qml");
    var grid;
    for(var i=0;i<500;i++){
        grid = createObj(comp,window);
        grid.visible = false;
        grid_stack[i] = grid;
        grid.setMarked(false);
    }
}


var footprint_stack=[];
var footprint_index=0;
var footprint_max=25;
function generateFootprintStack(window){
    var comp = Qt.createComponent("Footprint.qml");
    var foot;
    for(var i=0;i<footprint_max;i++){
        foot = createObj(comp,window);
        foot.visible = false;
        footprint_stack[i] = foot;
    }
    footprint_index=0;
}
function get_footprint(){
   var ret = footprint_stack[footprint_index];
   footprint_index = (footprint_index+1)%footprint_max;
   return ret;
}


function createObj(comp,window){
    if(comp.status===QQ.Component.Ready){
         var dynamicObject = comp.createObject(window);
         if (dynamicObject === null) {
            console.log("error creating obj");
            console.log(component.errorString());
            return false;
         }
         return dynamicObject;
    }
    return null;
}

