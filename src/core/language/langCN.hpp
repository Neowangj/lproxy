﻿// 注意此文件的编码格式

struct LangCN : public Lang {
	LangCN(void) {
		m["hello"] = "你好！";
		m["hehe"] = "呵呵";
	}
};
