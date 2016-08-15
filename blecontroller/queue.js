.pragma library // Shared game state
.import QtQuick 2.5 as QQ

function Queue() {
    this.dataStore=[]
    this.enqueue = enqueue;
    this.degueue = dequeue;
    this.empty = empty;
    this.front = front;
}
function enqueue(element){
    this.dataStore.push(element);
}
function dequeue(){
    this.dataStore.shift();
}
function empty(){
    if(this.dataStore.length==0){
        return true;
    }
    return false;
}
function front(){
    return this.dataStore[0];
}
