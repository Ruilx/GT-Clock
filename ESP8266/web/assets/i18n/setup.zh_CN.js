;(function(){
	"use strict";
	if(typeof gtsetup === "undefined"){
		throw Error("Import gtsetup first.");
	}
	gtsetup._updateOptions("zh_CN", {
		sdst: {
			"0": "无",
			"-0.5": "快半小时",
			"-1": "快1小时",
			"-1.5": "快1.5小时",
			"-2": "快2小时",
			"+0.5": "慢半小时",
			"+1": "慢1小时",
			"+1.5": "慢1.5小时",
			"+2": "慢2小时",
		},
		ssf: {
			"1": "每天凌晨",
			"2": "每周天凌晨",
			"3": "每月1日",
			"4": "每年初",
			"5": "手动同步",
		},
		sts: {
			0: {
				"1": "时, 分, 小字体秒 (24时)",
				"2": "时, 分, 小字体秒 (12时)(没有AM/PM符号)",
				"3": "时, 分 (12时)(AM/PM符号)",
				"4": "时, 分, 秒 (24时)",
				"5": "时, 分, 中字体秒 (24时)",
				"6": "时, 分 (24时)",
			},
			34: {
				"7": "时, 分 (NesPixel字体)(24时)",
				"8": "时, 分 (NesPixel字体)(12时)",
			},
			33: {
				"9": "时, 分 (Commodore64字体)(24时)",
				"10": "时, 分 (Commodore64字体)(12时)",
				"11": "时, 分, 小字体秒 (Commodore64字体)(24时)",
			},
		},
		sta: {
			"1": "靠顶",
			"2": "靠底",
		},
		swl: {
			"1": "在日期之前",
			"2": "在日期之后",
		},
		std: {
			"1": "使用STM32内部传感器",
			"2": "使用DS3231温度计量",
			"3": "使用DS18B20温度计量",
		},
		stu: {
			"1": "摄氏温标(℃)",
			"2": "华氏温标(℉)",
			"3": "开氏温标(K)",
			"4": "列氏温标(Re)",
		},
		stad: {
			"1": "不自动显示",
			"2": "每整点",
			"3": "每天8时整",
			"4": "每天8时, 16时整"
		},
		shr: {
			"1": "不提醒",
			"2": "滴滴声"
		}
	});

	gtsetup._updateI18n = function(){
		const si18n = {
			"index": {
				"#lm>li:eq(0)>a": "主页",
				"#lm>li:eq(1)>a": "功能",
				"#lm>li:eq(2)>a": "关于",
				"#ll": "语言",
				"#lns": "本页面包含使用JavaScript来进行动态信息的交互的内容, 请您允许浏览器执行JavaScript, 否则一些功能可能无法正常使用",
				"#lt>h1": "欢迎使用",
				"#lt>p:eq(0)": "感谢您使用GT-Clock网络时钟! 在开始使用之前, 我们需要在页面上应用一部分常用的设置, 点击\"保存并应用\"确认您的设置.",
				'#lt>p:eq(1)': "当您需要脱机使用GT-Clock时, 您只需本地设置好时间, 时钟将会按照设置的时间运行. 若需使用扩展功能, 请连接Wifi, 确保设备接入互联网.",
				'#lpts .panel-title': "时间设定",
				'#llt>label': "本地时间",
				'#bsldt': "同步本机时间",
				'#llt>p:eq(0)': "轻触输入框调出时间选择框, 或单击\"同步本机时间\"将本机时间同步给页面. 完毕后页面会自动更新时间, 并在保存应用时将最新的时间发送给设备.",
				'#cundt~span': "通过网络时间服务器进行时间同步",
				'#lnit': "您的设备尚未接入互联网, 时间同步服务可能无法正常运行.",
				'#lsi>p:eq(0)': "通过网络NTP服务器获取最新的网络时间, 并同步至设备中.",
				'#lsi>.input-group:eq(0)>span': "首选时间服务器",
				'#lsi>.input-group:eq(1)>span': "备选时间服务器",
				'#lsi>.input-group:eq(2)>span:eq(0)': "当前时区 UTC",
				'#bsltz': "同步本机时区",
				'#lsi>.input-group:eq(3)>span': "夏令时",
				'#lsi>.input-group:eq(4)>span': "同步频率",
				'#lrd>label': "RTC设备",
				'#lrd>p:eq(0)': "选择一个RTC设备, 将时间交由该设备存储.",
				'#lrd>p:eq(1)': "确保RTC设备后备电池工作正常, 否则断电后可能遗失正常时间.",
				'#lrd>.input-group:eq(0)>span': "RTC设备",
				'#lpds .panel-title': "显示设定",
				'#ldm>label:eq(0)': "设置时间显示方式, 字体和点阵微调",
				'#ldm .input-group:eq(0)>span:eq(0)': "预览",
				'#ldm .input-group:eq(1)>span:eq(0)': "时间字体",
				'#ldm .input-group:eq(2)>span:eq(0)': "预览",
				'#ldm .input-group:eq(3)>span:eq(0)': "时间风格",
				'#ldm .input-group:eq(4)>span:eq(0)': "对齐微调",
				'#ldm p:eq(0)': "如果选择的是高度为7的字体该可以配置显示靠顶或者靠底, 在相对位置预留一像素的宽度",
				'#c0h~span': "在不足两位的小时前补'0'",
				'#ldm p:eq(1)': "当小时不足两位时前显示'0'",
				'#lnf>label:eq(0)': "普通字体",
				'#lnf .input-group:eq(0)>span:eq(0)': "预览",
				'#lnf .input-group:eq(1)>span:eq(0)': "字体",
				'#ldf>label:eq(0)': "日期",
				'#ldf .input-group:eq(0)>span:eq(0)': "日期格式",
				'#c0d~span': "在不足两位的月和日前补'0'",
				'#ldf>p:eq(0)': "当月或日不足两位时前显示'0'",
				'#lwf>label:eq(0)': "星期",
				'#lwf .input-group:eq(0)>span:eq(0)': "星期格式",
				'#lwf .input-group:eq(1)>span:eq(0)': "星期位置",
				'#lpt .panel-title': "温度",
				'#ltm>label:eq(0)': "设置温度模式, 显示方式",
				'#ltm .input-group:eq(0)>span:eq(0)': "温度来源",
				'#ltm .input-group:eq(1)>span:eq(0)': "温度单位",
				'#ltm .input-group:eq(2)>span:eq(0)': "温度自动显示",
				'#lpo .panel-title': "其他设定",
				'#lhr>label:eq(0)': "整点提醒",
				'#lhr .input-group:eq(0)>span:eq(0)': "整点提醒",
				'#csan~span': "夜间不发出声音",
				'#lsn>p:eq(0)': "在夜间停止蜂鸣器工作以便不影响夜间休息",
				'#cab~span': "自动调整亮度",
				'#lab>p:eq(0)': "使用光感元件来自动调整屏幕亮度",
				'#caso~span': "自动屏幕方向",
				'#laso>p:eq(0)': "使用重力开关来自动调整屏幕方向",
				'#lpn .panel-title': "网络设置",
				'#lw>label:eq(0)': "设置WiFi连接点",
				'#lw>p:eq(0)': "设备载有ESP8266无线收发器, 可以连接外部的2.4GHz无线发射点使设备接入无线网. 您可以手动输入网络接入点的信息或在下面的可用网络接入点中选择您的接入点输入密码并保存, 设备将在重新启动后尝试连接指定接入点",
				'#lw .input-group:eq(0)>span:eq(0)': "SSID",
				'#lw .input-group:eq(1)>span:eq(0)': "密码",
				'#lw>p:eq(1)': "若接入点未设置密码或未保存更改的密码, 请将此处留空",
				'#laa>label:eq(0)': "可用网络",
				'#bwc': "选择此网络",
				'#bws': "再次扫描可用网络",
				'#ldr>label:eq(0)': "DNS解析点",
				'#ldr .input-group:eq(0)>span:eq(0)': "DNS",
				'#ldr>p:eq(0)': "DNS解析点留空则使用默认的DNS解析地址",
				'#lpib .panel-title': "互动按钮",
				'#lib>p:eq(0)': "按钮的功能可以在下方列表中进行循环播放, 短按一次切换一个功能, 长按可以执行插件的功能(若支持). 可以从\"可用插件\"列表中拖动到\"使用中的插件\"增加插件功能, 反之则停用插件功能, 拖动亦能进行排序.",
				'#lup': "使用中的插件",
				'#lup~.ph': "无",
				'#lap': "可用的插件",
				'#lap~.ph': "无",
				'#bie': "编辑插件",
				'#bs': "保存并应用",
				'#lcpr': "GT-Soft Studio, 2010 - 2020 保留所有权利.",
			},
		};
		if(typeof gtsetup._page !== "undefined") {
			for (let i in si18n[gtsetup._page]) {
				try {
					$(i).html(si18n[gtsetup._page][i]);
				} catch (e) {
					console.error(e);
				}
			}
		}
	};
	gtsetup._updateUI();

	if(typeof moment == "object" && typeof moment.defineLocale === "function") {
		var zhCn = moment.defineLocale('zh-cn', {
			months: '一月_二月_三月_四月_五月_六月_七月_八月_九月_十月_十一月_十二月'.split('_'),
			monthsShort: '1月_2月_3月_4月_5月_6月_7月_8月_9月_10月_11月_12月'.split('_'),
			weekdays: '星期日_星期一_星期二_星期三_星期四_星期五_星期六'.split('_'),
			weekdaysShort: '周日_周一_周二_周三_周四_周五_周六'.split('_'),
			weekdaysMin: '日_一_二_三_四_五_六'.split('_'),
			longDateFormat: {
				LT: 'HH:mm',
				LTS: 'HH:mm:ss',
				L: 'YYYY/MM/DD',
				LL: 'YYYY年M月D日',
				LLL: 'YYYY年M月D日Ah点mm分',
				LLLL: 'YYYY年M月D日ddddAh点mm分',
				l: 'YYYY/M/D',
				ll: 'YYYY年M月D日',
				lll: 'YYYY年M月D日 HH:mm',
				llll: 'YYYY年M月D日dddd HH:mm'
			},
			meridiemParse: /凌晨|早上|上午|中午|下午|晚上/,
			meridiemHour: function (hour, meridiem) {
				if (hour === 12) {
					hour = 0;
				}
				if (meridiem === '凌晨' || meridiem === '早上' ||
					meridiem === '上午') {
					return hour;
				} else if (meridiem === '下午' || meridiem === '晚上') {
					return hour + 12;
				} else {
					// '中午'
					return hour >= 11 ? hour : hour + 12;
				}
			},
			meridiem: function (hour, minute, isLower) {
				var hm = hour * 100 + minute;
				if (hm < 600) {
					return '凌晨';
				} else if (hm < 900) {
					return '早上';
				} else if (hm < 1130) {
					return '上午';
				} else if (hm < 1230) {
					return '中午';
				} else if (hm < 1800) {
					return '下午';
				} else {
					return '晚上';
				}
			},
			calendar: {
				sameDay: '[今天]LT',
				nextDay: '[明天]LT',
				nextWeek: '[下]ddddLT',
				lastDay: '[昨天]LT',
				lastWeek: '[上]ddddLT',
				sameElse: 'L'
			},
			dayOfMonthOrdinalParse: /\d{1,2}(日|月|周)/,
			ordinal: function (number, period) {
				switch (period) {
					case 'd':
					case 'D':
					case 'DDD':
						return number + '日';
					case 'M':
						return number + '月';
					case 'w':
					case 'W':
						return number + '周';
					default:
						return number;
				}
			},
			relativeTime: {
				future: '%s内',
				past: '%s前',
				s: '几秒',
				ss: '%d 秒',
				m: '1 分钟',
				mm: '%d 分钟',
				h: '1 小时',
				hh: '%d 小时',
				d: '1 天',
				dd: '%d 天',
				M: '1 个月',
				MM: '%d 个月',
				y: '1 年',
				yy: '%d 年'
			},
			week: {
				// GB/T 7408-1994《数据元和交换格式·信息交换·日期和时间表示法》与ISO 8601:1988等效
				dow: 1, // Monday is the first day of the week.
				doy: 4  // The week that contains Jan 4th is the first week of the year.
			}
		});
		moment.locale("zh-cn");
	}
})();