"use strict";
const
	cl = console.log,
	langList = {"zh_CN": "assets/i18n/setup.zh_CN.js"},
	l = typeof(jQuery) != "undefined" || function(){throw Error("jQuery not found.")}(),
	showError = function(msg){
		$("#main").html("<div class=\"jumbotron\"><div class=\"jumbotron-contents\"><h1 class=\"text-center\" style=\"margin-top: 10px\">System Error</h1><p>" + msg + "</p></div></div>");
		$("#bs").parent().empty();
	},
	optTmp = "<option {ATTRS}>{TEXT}</option>",
	clr = function(jdl){
		if(typeof jdl.html === "function") {
			jdl.html("");
		}else if(typeof jdl === "object"){
			for (let i in jdl) {
				jdl[i].html("");
			}
		}
	},
	selInsOpt = function(jdom, val, text, attrObj){
		attrObj = typeof attrObj === "undefined" ? {} : attrObj;
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
	timeFontTrigger = function(jd, targetValue){
		jd.css("background-position-y", -targetValue * 24);
	},
	n = null,
	u = undefined;