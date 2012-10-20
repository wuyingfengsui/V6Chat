目录结构：
|-db_interface(数据库接口，基于SQLite)
|
|-socket_interface(网络通信接口，基于WinSock2)
|
|-xmpp_interface(xmpp协议通信接口，基于gloox-0.9.9.12)
|
|-kernel-|-Bean-|-ChatHistoryItem(聊天记录条目)
|        |
|        |-FileList-|-SendFileList(发送文件列表，通过编号查找文件路径)
|        |          |
|        |          |-RecvFileList(接收文件列表，通过编号查找文件名)
|        |
|        |
|        |-IPSearcher(IP地址查询数据库DLL头文件)
|      	 |
|        |-json(JsonCpp头文件)
|        |
|        |-Lib(相关库文件)
|        |
|        |-libjpeg(jpeg库相关头文件)
|        |
|        |-libpng(libpng库相关头文件)
|        |
|        |-Lock(互斥对象)
|        |
|        |-Listener-|-ClientMsgListener(接收其他客户端发送的数据)
|        |          |
|        |          |-ServerMsgListener(接收服务端发送的数据)
|        |
|        |-Sender-|-MsgSender(发送消息基类)
|        |        |
|        |        |-ClientMsgSender(给其他客户端发送数据)
|        |        |
|        |        |-ServerMsgSender(给服务端发送数据)
|        |        |
|        |        |-WaitACKMsgSender(等待回复的消息池，池中的消息会被自动重发直至超时或被删除)
|        |
|        |-Server-|-ServerInfo(服务端资料)
|        |        |
|        |        |-ServerList(服务端列表)
|        |
|        |-UnreadMsg(未读消息栈，用户还未看到或还没做出选择的消息)
|        |
|        |-User-|-MyInfo(用户自己的个人资料)
|        |      |
|        |      |-UserInfo(用户资料基类)
|        |      |
|        |      |-P2PUserInfo(通过P2P方式保持联系的用户资料)
|        |      |
|        |      |-ServerUserInfo(通过服务端保持联系的用户资料)
|        |      |
|        |      |-UserList(用户列表)
|        |      |
|        |      |-TempUserList(临时用户列表，存储欲添加的新用户)
|        |
|        |-Handle(句柄类模板)
|        |
|        |-V6Util(一些常用的静态方法)
|        |
|        |-V6ChatDef(宏定义)
|        |
|        |-LuaV6Chat3App(用户界面接口)
|        |
|        |-V6Chat3App(程序接口)
|        |
|        |-kernel(DLL入口点)
|
|-V6Chat3(WIN32应用程序入口)



-------------------------------------提示------------------------------------------
1、Dependencies目录中有程序的调用关系图。

2、由于软件用到迅雷的Bolt界面库（版本号:1.2.0.386），所以编译前需包含相应的头文件目录及库目录，Bolt界面库可以从下面的链接获取：
http://bolt.xunlei.com/index.html

3、程序的正常运行可能还需要下列这些动态链接库文件：
sqlite3.dll（SQLite数据库DLL）
IPSearcher.dll（IP地址查询数据库DLL）
--------------以下均为迅雷Bolt界面库所需的DLL--------------
XLFSIO.dll
XLGraphic.dll
XLGraphicPlus.dll
XLLuaRuntime.dll
XLTS.dll
XLUE.dll
XLUEIPC.dll
XLUEOPC.dll
libexpat.dll
libpng13.dll
zlib1.dll
MSVCP71.DLL
msvcr71.dll
atl71.dll

4、IP地址查询还需要两个数据库文件（来自纯真IP数据库）：
ipv6wry.db
qqwry.dat

5、遇到任何问题都欢迎联系wuyingfengsui@gmail.com