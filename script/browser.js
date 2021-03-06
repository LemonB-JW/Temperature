// var curr = 0;
// var highest = 0;
// var lowest = 0;
// var average = 0;
var curr = "";
var highest = "";
var lowest = "";
var average = "";
var scale = "c";
var ht = 0;
var lt = 0;

$(".scale").click(selectScale);
$("#setThrehold").click(setT);
$("#standby").click(standby);
$("#cis").click(showCIS);

// update temperature display every 5 seconds
setInterval(function(){
  $.getJSON('http://localhost:3001/T', function(data){
    // TODO: can't receive cur temp
    curr = data.curr;
    highest = data.highest;
    lowest = data.lowest;
    average = data.average;
    status = data.status;
    console.log(data.curr);

    if (status == 1){
      $("#message").html("Warning - disconnected from Arduino");
    }
    else {
      $("#message").html("Arduino Connected");
      updateDisplay();
    }
  })
}, 5000);

// function updateCurr(){
//
// }

function selectScale(){
    //update display according to the scale
    scale = $(this).val();
    console.log(scale);
    sendScale();
}

//helper function to send scale to server
function sendScale(){
  console.log('F');
  //update return.jason to actually request "http://localhost:3001/temp"
  $.get('http://localhost:3001/F', function(){
    $("#message").html("the scale is " + scale);
  })
}

//helper function to update temperature display
function updateDisplay(){
  console.log(curr);
  $("#curr").html(curr);
  $("#highest").html(highest);
  $("#lowest").html(lowest);
  $("#average").html(average);
}

function setT(){
  if ($("#ht").val() != null){
    ht = parseFloat($("#ht").val());
  }
  if ($("#lt").val() != null){
    lt = parseFloat($("#lt").val());
  }

  if (ht < lt){
    $("#message").html("Error:the lower threhold must be smaller than higher threhold");
  }
  else {
    var t = [ht, lt];
    console.log(t);
    //update return.jason to actually request "http://localhost:3001/temp"
    $.get('http://localhost:3001/'+t,function(){
      $("#message").html("the threshold is " + t);
    })
  }
}

function standby(){
  $.get('http://localhost:3001/S', function(){
    $("#message").html("Standby mode - no update from Arduino");
  })
}

function showCIS(){
  $.get('http://localhost:3001/X', function(){
    $("#message").html("show CIS on Arduino screen ");
  })
}
