qt5-TCP/HTTP转发工具

项目介绍：
基于qt5做的TCP/HTTP转发服务器，支持一个端口同时转发TCP和HTTP。

应用场景：
服务器只开放一个端口，但是同时需要相应TCP和HTTP协议，比如后端web服务及ssh服务。

功能：
1、TCP端口转发，将一个TCP端口的信息转发至多个TCP端口，返回的信息将全部有这个TCP端口接收；
2、一个TCP端口链接一个TCP和HTTP端口，通过分析入口TCP端口接收到的信息判断此连接是TCP还是HTTP协议，从而选择对应的端口转发消息。

配置说明（把tcpTransferConfig.json文件复制到程序运行目录）：
一个json对象中有tasks字段；
tasks字段中是一个数组，数组中每个json定义一个映射关系；
localPort：监听端口
type:group或者forward，gruop为群体转发，forward为判断TCP或HTTP转发
targets:转发地址对象，里面是一个数组，每个数组为一个地址
		targetIp：转发ip
		targetPort：转发端口
		type：转发类型，TCP或者HTTP，在上述2功能时用来选择对应的TCP或者HTTP连接，有多个TCP或HTTP连接时，默认选择第一个