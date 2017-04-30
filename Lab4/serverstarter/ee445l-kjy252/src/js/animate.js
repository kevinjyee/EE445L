$(document).ready(function () {
// Create two variables with names of months and days of the week in the array
var monthNames = [ "January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December" ]; 
var dayNames= ["Sunday","Monday","Tuesday","Wednesday","Thursday","Friday","Saturday"]

function updateClock(id,newValue)
{
    $(id).toggleClass('halfRotation').text(newValue);
}





setInterval( function() {
    // Create an object newDate () and extract the clock from the current time
    var hours = new Date().getHours();
    // Add a leading zero to the value of hours
    hours = (hours < 10 ? "0" : "")  + hours;
    console.log(hours);
        var minutes = new Date().getMinutes();
    // Add a leading zero to the minutes
    minutes =( minutes < 10 ? "0" : "" ) + minutes;
    console.log(minutes);
    var finaltime = hours +":" +minutes;
    updateClock('#firstpan .TIME .t',finaltime)
    }, 1000);


// Create an object newDate()
var newDate = new Date();
// Retrieve the current date from the Date object
newDate.setDate(newDate.getDate());
// At the output of the day, date, month and year
var newVal = dayNames[newDate.getDay()] + " " + newDate.getDate() + ' ' + monthNames[newDate.getMonth()] + ' ' + newDate.getFullYear();

updateClock('#firstpan .DATE .digit',newVal)
//$('#DATE .digit:last-child').text(dayNames[newDate.getDay()] + " " + newDate.getDate() + ' ' + monthNames[newDate.getMonth()] + ' ' + newDate.getFullYear());
//$('#Date').html(dayNames[newDate.getDay()] + " " + newDate.getDate() + ' ' + monthNames[newDate.getMonth()] + ' ' + newDate.getFullYear());

/*
setInterval( function() {
    // Create an object newDate () and extract the second of the current time
    var seconds = new Date().getSeconds();
    // Add a leading zero to the value of seconds
    $("#sec").html(( seconds < 10 ? "0" : "" ) + seconds);
    },1000);
    
setInterval( function() {
    // Create an object newDate () and extract the minutes of the current time
    var minutes = new Date().getMinutes();
    // Add a leading zero to the minutes
    $("#min").html(( minutes < 10 ? "0" : "" ) + minutes);
    },1000);
    
setInterval( function() {
    // Create an object newDate () and extract the clock from the current time
    var hours = new Date().getHours();
    // Add a leading zero to the value of hours
    $("#hours").html(( hours < 10 ? "0" : "" ) + hours);
    }, 1000);
    */
});