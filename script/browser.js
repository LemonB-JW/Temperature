var curr = 0;
var highest = 0;
var lowest = 0;
var average = 0;
var scale = "c";
var ht = 0;
var lt = 0;

$(".scale").click(selectScale);
$("#setThrehold").click(setT);
$("#update").click(updateCurr);

function updateCurr(){
  //update example.jason to actually request "http://localhost:3001/temp"
  $.getJSON('http://localhost:3001/T', function(data){
    curr = data.curr;
    console.log(curr);
    // highest = data.highest;
    // lowest = data.lowest;
    // average = data.average;
    updateDisplay()
  })
}

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
  // $("#highest").html(highest);
  // $("#lowest").html(lowest);
  // $("#average").html(average);
}

function setT(){
  if ($("#ht").val() != null){
    ht = $("#ht").val();
  }
  if ($("#lt").val() != null){
    lt = $("#lt").val();
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
