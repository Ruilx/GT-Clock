const
	cl = console.log,
	l = typeof(jQuery) != "undefined" || function(){throw Error("jQuery not found.")}(),
	showError = function(msg){
		$("#main").html("<div class=\"jumbotron\"><div class=\"jumbotron-contents\"><h1 class=\"text-center\" style=\"margin-top: 10px\">System Error</h1><p>" + msg + "</p></div></div>");
		$("#bs").parent().empty();
	},
	optTmp = "<option value=\"{VAL}\">{TEXT}</option>",
	selInsOpt = function(jdom, val, text){
		if(typeof(jdom) !== "object" || typeof(jdom.append) !== "function"){
			throw Error("Insert target not an object(jQuery)");
		}
		jdom.append(optTmp.replace("{VAL}", val).replace("{TEXT}", text));
	},
	u = undefined;