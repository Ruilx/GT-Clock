{
	"title": "现在天气",
	"desc": "从中国气象数据网获取天气",
	"author": "Ruilx",
	"mode": 11,
	"arg": {
		"staId": {
			"type": "i",
			"require": true,
		}
	},
	"var": {
		"temp": {
			"type": "f"
		}
	}
	"req": {
		"url": "http://data.cma.cn/weatherGis/web/weather/weatherFcst/getCurrentCondition",
		"method": "POST",
		"data": "staId={staId}",
		"ua": "{CHROME}",
		"headers": {
			"Content-Type": "application/x-www-form-urlencoded; charset=UTF-8",
			"Accept": "application/json",
			"Origin": "http://data.cma.cn",
			"Referer": "http://data.cma.cn/forecast/index.html",
			"X-Requested-With": "XMLHttpRequest"
		},
		"check": {
			"sc": 200,
			"ctype": "json",
			"maxsize": "255"
		},
		"dstr": "v12001=temp",
	},
	"res": {
		"mode": 1,
		"str": "{temp}℃"
	},
	"expire": 14400
}