const
	cl = console.log,
	l = typeof(jQuery) != "undefined" || function(){throw Error("jQuery not found.")}(),
	showError = function(msg){
		$("#main").html("<div class=\"jumbotron\"><div class=\"jumbotron-contents\"><h1 class=\"text-center\" style=\"margin-top: 10px\">System Error</h1><p>" + msg + "</p></div></div>");
		$("#bs").parent().empty();
	},
	optTmp = "<option {ATTRS}>{TEXT}</option>",
	selInsOpt = function(jdom, val, text, attrObj = {}){
		if(typeof(jdom) !== "object" || typeof(jdom.append) !== "function"){
			throw Error("Insert target not an object(jQuery)");
		}
		if(typeof(attrObj) !== "object"){
			throw Error("attrs is not an object.");
		}
		let attrs = "";
		if(val !== ""){
			attrObj['value'] = val;
		}
		for(let i in attrObj){
			attrs += i + "=\"" + attrObj[i] + "\" ";
		}
		jdom.append(optTmp.replace("{VAL}", val).replace("{TEXT}", text).replace("{ATTRS}", attrs));
	},
	u = undefined;