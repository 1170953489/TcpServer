# 私人在线网盘
下载 TcpClient 和 TcpServer 并修改config.txt中的IP地址为 127.0.0.1 进行本地测试。如果没有Qt开发环境也可以直接[下载](https://github.com/1170953489/TcpServer/releases/tag/1.0)打包好的运行程序，先启动TcpServer后再启动TcpClient。
- 本项目使用 C/S 架构，Qt界面开发，服务器数据库使用sqlite3，通信使用TCP套接字，自己封装了PDU协议来实现各种功能。
- 使用了多线程提高并发访问效率，可同时多人聊天与上传下载多个文件。
- 客户端与服务器实现多对多通信，服务器主线程进行套接字监听，子线程处理业务逻辑。
### 服务器TcpServer
- MyTcpServer负责监听8888端口处理用户普通请求逻辑，一个客户端对应MyTcpServer下的一个子线程。
- FileTcpServer监听9999端口处理用户文件操作逻辑，一个文件操作请求对应FileTcpServer下的一个子线程。
### 客户端TcpClient
- 一个客户端对应一个主线程，文件操作在独立的子线程里处理。
  ![1](https://github.com/1170953489/TcpServer/assets/91270044/2488d126-bddb-4ce7-a858-5be7597e37d7)
  
  ### 
