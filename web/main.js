
var statusTab = document.getElementById("status-tab");
var batteryTab = document.getElementById("battery-tab");
var driveTab = document.getElementById("drive-tab");
var chargeTab = document.getElementById("charge-tab");
var accessoriesTab = document.getElementById("accessories-tab");

var ui = {
    onLoad: function() {
        batteryTab.classList.add("hidden");
        driveTab.classList.add("hidden");
        chargeTab.classList.add("hidden");
        accessoriesTab.classList.add("hidden");
    }
}

function showTab(tabId, buttonId) {
    var contentContainers = document.querySelectorAll('.tab');
    contentContainers.forEach(function(item) {
        item.classList.add('hidden');
    });
    var tab = document.getElementById(tabId);
    tab.classList.remove('hidden');
    document.querySelectorAll('.nav-button').forEach(function(item) {
        item.classList.remove('nav-active');
    });
    document.getElementById(buttonId).classList.add('nav-active');
}

function showBatteryTab(tabId, buttonId) {
    // Switch the tab
    var batteryTabs = document.querySelectorAll('.battery-subtab');
    batteryTabs.forEach(function(item) {
        console.log("Hiding " + item);
        item.classList.add('hidden');
    });
    var tab = document.getElementById(tabId);
    tab.classList.remove('hidden');
    // Update the active button
    document.querySelectorAll('.battery-nav-button').forEach(function(item) {
        item.classList.remove('battery-nav-active');
    });
    document.getElementById(buttonId).classList.add('battery-nav-active');
}

function showChargeTab(tabId, buttonId) {
    var chargeTabs = document.querySelectorAll('.charge-tab');
    chargeTabs.forEach(function(item) {
        item.classList.add('hidden');
    });
    var tab = document.getElementById(tabId);
    tab.classList.remove('hidden');
    document.querySelectorAll('.charge-nav-button').forEach(function(item) {
        item.classList.remove('charge-nav-active');
    });
    document.getElementById(buttonId).classList.add('charge-nav-active');
}

function showVoltageTemperatureTab(){

}