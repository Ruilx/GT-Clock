;(function(global, factory){
	typeof exports === "object" && typeof module !== "undefined" ? module.exports = factory() : typeof define === "function" && define.amd ? define(factory) : (
		global = global || self, global.gtfonts = factory()
	)
}(this, function(){
	"use strict";
	let tf = $("#tf"),
		sf = $("#sf"),
		fonts = {
			1: ["R Pro Bold",    0, 0, 571],
			2: ["R Bold",        1, n, 571],
			3: ["R Bold Serif",  2, n, 571],
			4: ["RC Bold",       3, 1, 571],
			5: ["R Std",         4, n, 570],
			6: ["RuilxFixedSys", 5, 2, 570],
			7: ["RH Sans",       6, n, 570],
			8: ["Casio LCD",     7, n, 570],
			9: ["Minecraftia",   8, 3, 570],
			10:["R Sans",        9, n, 570],
			11:["R 4511",       10, n, 570],
			12:["R Sans Pro",   11, n, 570],
			13:["R Pro",        12, n, 570],
			14:["OCR A",        13, 4, 570],
			15:["R Hand",       14, n, 570],
			16:["R SMG",        15, 5, 570],
			17:["R SMG Bold",   16, n, 570],
			18:["R Kanji",      17, n, 570],
			19:["R Kanji Pro",  18, n, 570],
			20:["R Half Bold",  19, n, 570],
			21:["R LSD",        20, n, 570],
			22:["R Block",      21, n, 570],
			23:["R Small",      22, n, 570],
			24:["R Sidy",       23, n, 570],
			25:["R Shift Sym",  24, n, 570],
			26:["R Sq",         25, n, 570],
			27:["R Sq Pro",     26, n, 570],
			28:["R Sq Std",     27, n, 570],
			29:["R Sq Serif",   28, n, 570],
			30:["Georgia",      29, n, 570],
			31:["R 6H",         30, n, 570],
			32:["Simsun",       31, 6, 680],
			33:["Commodore64",  32, 7, 670],
			34:["NesPixel",     33, 8, 770],
			35:["Devanagari",   34, n, 570],
			36:["Arabic",       35, n, 570],
			37:["Minecraft",     n, 9, 570],
		};

	clr([tf, sf]);
	for(let item in fonts){
		if(fonts[item][1] != null){
			selInsOpt(tf, item, fonts[item][0]);
		}
		if(fonts[item][2] != null){
			selInsOpt(sf, item, fonts[item][0]);
		}
	}
	sf.change(function(e){
		timeFontTrigger($("#fp"), fonts[e.target.value][2]);
	});

	return {
		fonts: fonts,
		tfDtno: function(v){
			return fonts[v][1];
		},
		sfDtno: function(v){
			return fonts[v][2];
		},
		prop: function(v){
			return fonts[v][3];
		}
	}
}))