﻿#SerialTCP(Server): 实现TCP服务端与串口的交互程序


##概述

Stcps是一个传统的串口转网络（tcp服务端模式）的应用程序实现。

运行本程序，模块会在指定端口运行tcp服务器。如果有客户端接入，便将串口数据和TCP目标进行数据交换。

本示例演示了基于tcp服务端框架的程序设计方法，复杂json文件的解析过程 以及 串口详细的操作方法。

##如何使用

1. 安装stcps.add插件到模块（如果从云端部署，请确定以stcps名称实施，这将保证程序被安装在“/app/stcps”目录下）；
2. 上传index.html、stcps.js 和 cfg.json文件到"/app/stcps"目录；
3. 打开 http://192.168.1.xxx/app/stcps/index.htm 到配置页面设置相应参数（例如，本地端口）。
4. 运行模块上stcps应用（通过云端或者本地）；
5. 在PC机建立TCP客户端连接模块服务器，尝试通过网络或者串口侧向连接输入数据，观察对方输出。

*更详细的配置内容，请查看应用主页中“使用说明”的选项卡。*

##备注
cfg.json作为本应用程序的配置文件存在，其由同目录下的index.html和stcps.js 生成和管理。

stcps.add运行初需要读取“/app/stcps/cfg.json”（看代码main函数），如果该文件不存在或者格式不正确，stcps都无法正常工作。


##依赖
无


****
更多细节请参考源代码。

20131006
问题和建议请email: yizuoshe@gmail.com 


