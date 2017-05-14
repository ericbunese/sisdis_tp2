var html = {
 "rep":"relatorio.html",
 "cod":"fonte.html",
 "log":"log.html"
};

function init()
{
 var frm = document.getElementById("frame");
 var rep = document.getElementById("rep");
 var cod = document.getElementById("cod");
 var log = document.getElementById("log");

 rep.addEventListener("click", function(){
   frm.src = html["rep"];
 });

 cod.addEventListener("click", function(){
  frm.src = html["cod"];
 });

 log.addEventListener("click", function(){
  frm.src = html["log"];
 });
}
