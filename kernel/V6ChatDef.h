#pragma once

#define XAR_RELATIVE_PATH            TEXT("..\\..\\XAR")
#define USERDATA_RELATIVE_PATH       TEXT("..\\..\\UserData")
#define MYHEADIMAGE_PELATIVE_PATH    TEXT("My\\head.png")
#define USERHEAD_RELATIVE_PATH       TEXT("Head")
#define BEFOER_TEMPUSERHEAD          TEXT("search")
#define USERHEAD_RECVFILE_PATH       TEXT("Recv")
#define USERHEAD_EXT                 TEXT(".png")
#define DEFAULT_GROUP_NAME           TEXT("好友")

#define MAX_TRY_TIMES                3
#define ETERNAL_TRY_TIMES            -1

#define HEAD_IMAGE_SIZE              1024*100
#ifndef SMALL_FILE_MAXSIZE
#define SMALL_FILE_MAXSIZE 10*1024*1024
#endif

#define LIVE_TIME                    35*1000
#define SEND_WAITACKMSG_LOOP_TIME     5*1000

#define EACH_CHATHISTORYITEM_NUM     20

//窗口类名
#define MAIN_WND                     "MainFrame"
#define FRIENDLIST_WND               "MainTab1"
#define BEFORE_CHAT_WND              "ChatFrame"
#define ADDFRIEND_WND                "AddFriendFrame"

//主窗口自定义消息
#define  WM_TRAYMSG                  0x0401                                        //托盘消息
#define  WM_SHOWUNREADMSG            0x0402
#define  WM_SHOWSENDEDMSG            0x0403
//好友列表自定义消息
#define  WM_UPDATEFRIENDLIST         0x0401                              //更新好友列表消息（wParam：1-添加好友（未实现），2-删除好友（未实现），3-更新好友信息（不存在会自动添加此好友，也可用于聊天窗口））
#define  WM_UPDATECHATPANEL          0x0402                              //更新聊天窗口
#define  WM_UPDATECHATDOWNLOADFILE   0x0403                              //更新聊天窗口中的文件（下载完成时）

// 添加好友窗口自定义消息
#define  WM_UPDATESERVERLIST         0x0401                              //更新服务器消息（wParam：1-添加服务器（未实现），2-删除服务器（未实现），3-更新服务器信息（不存在会自动添加此服务器））
#define  WM_UPDATESEARCHUSERRESULT   0x0402                              // 更新搜索用户结果（不存在会自动添加此用户）

#define DEFAULT_XMPP_PORT 5222

// P2P网络发送/接收的用户信息包
// UDP
#define NORMAL_ACK                   50
#define USER_ONLINE                  1
#define USER_ONLINE_ACK              2
#define USER_KEEPONLINE              3
#define USER_KEEPONLINE_ACK          4
#define USER_OFFLINE                 5
#define ADD_P2PUSER_ASK              6
#define REPLY_ADD_USER               8
#define TEXT_CHAT                    10
// TCP
#define USER_INFO_UPDATE             51
#define GET_FILE                     52
#define SEND_FILE                    53

// Server网络信息包
#define ADD_SERVERUSER_ASK            7

enum SelectFileRetCode
{
	success,
	user_cancel,
	invalid_file,
	size_too_large
};

enum ServerState
{
	server_disconnect,
	server_connected
};

enum UserState
{
	user_offline,
	user_online
};

enum ContentType
{
	system_content,
	user_content
};