;(function(global, factory){
	typeof exports === "object" && typeof module !== "undefined" ? module.exports = factory() : typeof define === "function" && define.amd ? define(factory):
		global.gtsetup = factory()
}(this, function(){
	"use strict";
	var defaults = {
		_lang: "en_US",
		ste: [
			"time.windows.com",
			"time.nist.gov",
			"time-nw.nist.gov",
			"time-a.nist.gov",
			"time-b.nist.gov",
			"ntp1.aliyun.com",
			"cn.ntp.org.cn",
			"us.ntp.org.cn",
		],
		stz: {
			"-12": "-12:00",
			"-11": "-11:00",
			"-10": "-10:00",
			"-9": "-09:00",
			"-8": "-08:00",
			"-7": "-07:00",
			"-6": "-06:00",
			"-5": "-05:00",
			"-4.5": "-04:30",
			"-4": "-04:00",
			"-3.5": "-03:30",
			"-3": "-03:00",
			"-2": "-02:00",
			"-1": "-01:00",
			"0": "00:00",
			"1": "+01:00",
			"2": "+02:00",
			"3": "+03:00",
			"3.5": "+03:30",
			"4": "+04:00",
			"4.5": "+04:30",
			"5": "+05:00",
			"5.5": "+05:30",
			"5.75": "+05:45",
			"6": "+06:00",
			"6.5": "+06:30",
			"7": "+07:00",
			"8": "+08:00",
			"9": "+09:00",
			"9.5": "+09:30",
			"10": "+10:00",
			"11": "+11:00",
			"12": "+12:00",
			"13": "+13:00"
		},
		sdst: {
			"0": "None",
			"-0.5": "0.5 hours before",
			"-1": "1 hour before",
			"-1.5": "1.5 hours before",
			"-2": "2 hours before",
			"+0.5": "0.5 hours later",
			"+1": "1 hours later",
			"+1.5": "1.5 hours later",
			"+2": "2 hours later"
		},
		ssf: {
			"1": "Every morning",
			"2": "Every Sunday morning",
			"3": "1st of every month",
			"4": "First day of year",
			"5": "Sync manually"
		},
		sts: {
			0: {
				"1": "Hours, Minutes and small Seconds(24H)",
				"2": "Hours, Minutes and small Seconds(12H)(without AM/PM sign)",
				"3": "Hours and Minutes(12H)(with AM/PM sign)",
				"4": "Hours, Minutes and big Seconds(24H)",
				"5": "Hours, Minutes and middle Seconds(24H)",
				"6": "Hours and Minutes(24H)",
			},
			34: {
				"7": "Hours and Minutes(NesPixel)(24H)",
				"8": "Hours and Minutes(NesPixel)(12H)",
			},
			33: {
				"9": "Hours and Minutes(Commodore64)(24H)",
				"10": "Hours and Minutes(Commodore64)(12H)",
				"11": "Hours, Minutes and small Seconds(Commodore64)(24H)",
			},
		},
		sta: {
			"1": "Align top",
			"2": "Align bottom"
		},
		sdf: {
			"1": "2019-12-31",
			"2": "19-12-31",
			"3": "2019/12/31",
			"4": "19/12/31",
			"5": "31/12/2019",
			"6": "31/12/19",
			"7": "12/31/2019",
			"8": "12/31/19",
			"9": "Dec. 31, 2019",
			"10": "31 Dec, 2019",
			"11": "2019年12月31日",
			"12": "二〇一九年十二月三十一日",
			"13": "令和元年12月31日",
			"14": "れいわ1年12月31日",
			"15": "R1.12.31"
		},
		swf: {
			"1": "Sun. Mon. Tue. Wed. Thu. Fri. Sat.",
			"2": "Sunday Monday Tuesday Wednesday Thursday Friday Saturday",
			"3": "周日 周一 周二 周三 周四 周五 周六",
			"4": "日 月 火 水 木 金 土"
		},
		swl: {
			"1": "Before date",
			"2": "After date"
		},
		std: {
			"1": "Using DS3231 device",
			"2": "Using DS18B20 device"
		},
		stu: {
			"1": "Celsius temperature scale (℃)",
			"2": "Fahrenheit temperature scale (℉)",
			"3": "Kelvin temperature scale (K)",
			"4": "Column temperature scale (Re)"
		},
		stad: {
			"1": "Do not automatically shown",
			"2": "Every hour",
			"3": "8 o'clock every day",
			"4": "Every day at 8 and 16 o'clock"
		},
		shr: {
			"1": "Mute",
			"2": "Beep~"
		},
		_updateFunc: {
			ste: function(){
				let data = defaults.ste,
					jdom = [$("#sts1"), $("#sts2")];
				for(let i in jdom){
					for(let j in data){
						selInsOpt(jdom[i], data[j], data[j]);
					}
				}
			},
			stz: function(){
				kvselIns($("#stz"), defaults.stz);
			},
			sdst: function(){
				kvselIns($("#sdst"), defaults.sdst);
			},
			ssf: function(){
				kvselIns($("#ssf"), defaults.ssf);
			},
			sts: function(){
				let sts = $("#sts"),
					tf = $("#tf"),
					prets = 0,
					getPrets = function(v){
						return typeof defaults.sts[v] === "undefined" ? 0 : v;
					};
				kvselIns(sts, defaults.sts[0]);
				tf.change(function(e){
					timeFontTrigger($("#tfp"), gtfonts.tfDtno(e.target.value));
					if (getPrets(e.target.value) !== prets) {
						clr(sts);
						kvselIns(sts, defaults.sts[getPrets(e.target.value)]);
						sts.val(sts.find("option")[0].value).change();
						prets = getPrets(e.target.value);
					}
				});
				sts.change(function(e){
					console.log(e.target.value);
					timeFontTrigger($("#tsp"), e.target.value -1);
				});
			},
			sta: function(){
				kvselIns($("#sta"), defaults.sta);
			},
			sdf: function(){
				kvselIns($("#sdf"), defaults.sdf);
			},
			swf: function(){
				kvselIns($("#swf"), defaults.swf);
			},
			swl: function(){
				kvselIns($("#swl"), defaults.swl);
			},
			std: function(){
				kvselIns($("#std"), defaults.std);
			},
			stu: function(){
				kvselIns($("#stu"), defaults.stu);
			},
			stad: function(){
				kvselIns($("#stad"), defaults.stad);
			},
			shr: function(){
				kvselIns($("#shr"), defaults.shr);
			},
		},
		_updateOptions: function(name, values){
			if(values !== null){
				values._lang = name;
				defaults = $.extend(defaults, values);
				return "OK";
			}
			return "fail";
		},
		_updateUI: function(){
			console.log("Selected language: " + defaults._lang);
			for(let i in defaults){
				//if(i.startsWith("_")){
				if(i[0] === "_"){
					continue;
				}
				if(typeof defaults._updateFunc[i] !== "function"){
					console.warn("Option: \"" + i + "\" have no handle function");
					continue;
				}
				clr($("#" + i));
				defaults._updateFunc[i]();
			}
			if(typeof defaults._updateI18n === "function"){
				defaults._updateI18n();
			}
			return "OK";
		}
	};

	function kvselIns(jdom, obj){
		for(let i in obj){
			selInsOpt(jdom, i, obj[i]);
		}
	}

	return defaults;
}));