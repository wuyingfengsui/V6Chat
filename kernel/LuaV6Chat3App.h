#pragma once

#include "V6Chat3App.h"

#define XUNLEI_IDEAPP_CLASS		"V6Chat3App.Class"
#define XUNLEI_IDEAPP_OBJ		"V6Chat3App"

#include <list>

#include "Listener\ServerMsgListener.h"

class LuaV6Chat3App
{
public:
	LuaV6Chat3App(void);
	~LuaV6Chat3App(void);

    static V6Chat3App* __stdcall Instance(void*);

	static int AddTray(lua_State *luaState);                              //添加托盘图标
	static int GetMyHead(lua_State *luaState);                            //获取自己的头像
	static int GetMyInfo(lua_State *luaState);                            //获取自己的资料
	static int GetMyIpAndPort(lua_State *luaState);                       //获得本机IP及监听端口号
	static int SetMyIntroduction(lua_State *luaState);                    //设置自己的介绍
	static int SelectMyHead(lua_State *luaState);                         //弹出选择图片对话框选择头像
	static int Start(lua_State *luaState);                                //点击开始按钮事件
	static int StartServer(lua_State *luaState);                          // 开启连接服务器，需在用户列表窗口已完成后调用（确保在线状态能及时更新）
	static int Quit(lua_State *luaState);                                 //退出
	static int DeleteTray(lua_State *luaState);                           //删除托盘

	static int GetUserList(lua_State *luaState);                          //获取用户列表
	static int GetUserInfoByID(lua_State *luaState);                      //通过uid获得用户资料
	static int GetUnreadMessage(lua_State *luaState);                     //获取最新的未读消息
	static int DeleteUnreadMessage(lua_State *luaState);                  //删除最新一条未读消息

	static int AddServer(lua_State *luaState);                            // 添加服务器
	static int GetServerList(lua_State *luaState);                        // 获得服务器列表
	static int GetServerInfoByID(lua_State *luaState);                    // 通过sid获得服务器资料
	static int ChangeServerInfo(lua_State *luaState);                     // 修改某个服务器资料
	static int SearchFriendOnServer(lua_State *luaState);                 // 在服务器上搜索好友
	static int GetSearchFriendResult(lua_State *luaState);                // 获得搜索结果

	static int AddP2PFriend(lua_State *luaState);                         //添加P2P好友
	static int AddServerFriend(lua_State *luaState);                      //添加服务端好友
	static int AddFriendAck(lua_State *luaState);                         //关于添加好友请求的用户选择（同意/拒绝）
	static int ChangeFriendGroup(lua_State *luaState);                    //更改好友所在分组
	static int DeleteFriend(lua_State *luaState);                         //删除好友

	static int ChatWithText(lua_State *luaState);                         //发送文字聊天
	static int GetChatContent(lua_State *luaState);                       //获得收到的聊天内容
	static int SelectChatFile(lua_State *luaState);                       //选择待发送的文件
	static int GetSendFile(lua_State *luaState);                          //获得发送列表中的文件
	static int GetRecvFile(lua_State *luaState);                          //获得接收列表中的文件
	static int GetChatHistory(lua_State *luaState);                       //获得聊天记录

	static int GetCursorPos(lua_State *luaState);                         //获得当前鼠标指针的坐标
	static int GetWorkareaPos(lua_State *luaState);                       //获得屏幕坐标
	static int GetImageHandle(lua_State *luaState);                       //获得图片的句柄
	static int OpenShellExecute(lua_State *luaState);                     //外部打开一个文件/文件夹
	static int PostWindowMessage(lua_State *luaState);                    //发送窗口消息
	static int FileExist(lua_State *luaState);                            // 文件是否存在
	static int CopyToClipboard(lua_State *luaState);                      // 复制一段字符串到剪贴板

	static long RegisterObj(XL_LRT_ENV_HANDLE hEnv);

protected:
	static V6Chat3App* GetV6Chat3App( lua_State* luaState );
};