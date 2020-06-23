# GT-Clock 字体预览

字体都作为变宽字体使用, 外观上commodore64, nespixel, ruilxfixedsys, ocr a 为等宽字体, commodore和nespixel宽度为8, 其他宽度为6.
其他的是变宽字体, 宽度为可见宽度+1空列进行裁剪, 可取0x00-0x7F的字摸, 其中0x00-0x1F的模型存在, 依照实际情况看是否保留
NesPixel到时候小写字符会更新,
假名一栏到时候会遵循unicode顺序而非shiftJIS, 可不做

其中时间字体宽度有三种
固定宽度6x8字符显示时选择宽度5,
Minecraftia字体选择宽度5,
R pro bold, RC bold选择宽度5,
commodore64选择宽度6(数字4的第6列内容保留, 即便会与其他字符挨上),
nespixel选择宽度7

另外还有一些固定字符(汉字, 图标等)下来单独制作

其他的没有了