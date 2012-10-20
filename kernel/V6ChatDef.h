#pragma once

#define XAR_RELATIVE_PATH            TEXT("..\\..\\XAR")
#define USERDATA_RELATIVE_PATH       TEXT("..\\..\\UserData")
#define MYHEADIMAGE_PELATIVE_PATH    TEXT("My\\head.png")
#define USERHEAD_RELATIVE_PATH       TEXT("Head")
#define BEFOER_TEMPUSERHEAD          TEXT("search")
#define USERHEAD_RECVFILE_PATH       TEXT("Recv")
#define USERHEAD_EXT                 TEXT(".png")
#define DEFAULT_GROUP_NAME           TEXT("����")

#define MAX_TRY_TIMES                3
#define ETERNAL_TRY_TIMES            -1

#define HEAD_IMAGE_SIZE              1024*100
#ifndef SMALL_FILE_MAXSIZE
#define SMALL_FILE_MAXSIZE 10*1024*1024
#endif

#define LIVE_TIME                    35*1000
#define SEND_WAITACKMSG_LOOP_TIME     5*1000

#define EACH_CHATHISTORYITEM_NUM     20

//��������
#define MAIN_WND                     "MainFrame"
#define FRIENDLIST_WND               "MainTab1"
#define BEFORE_CHAT_WND              "ChatFrame"
#define ADDFRIEND_WND                "AddFriendFrame"

//�������Զ�����Ϣ
#define  WM_TRAYMSG                  0x0401                                        //������Ϣ
#define  WM_SHOWUNREADMSG            0x0402
#define  WM_SHOWSENDEDMSG            0x0403
//�����б��Զ�����Ϣ
#define  WM_UPDATEFRIENDLIST         0x0401                              //���º����б���Ϣ��wParam��1-��Ӻ��ѣ�δʵ�֣���2-ɾ�����ѣ�δʵ�֣���3-���º�����Ϣ�������ڻ��Զ���Ӵ˺��ѣ�Ҳ���������촰�ڣ���
#define  WM_UPDATECHATPANEL          0x0402                              //�������촰��
#define  WM_UPDATECHATDOWNLOADFILE   0x0403                              //�������촰���е��ļ����������ʱ��

// ��Ӻ��Ѵ����Զ�����Ϣ
#define  WM_UPDATESERVERLIST         0x0401                              //���·�������Ϣ��wParam��1-��ӷ�������δʵ�֣���2-ɾ����������δʵ�֣���3-���·�������Ϣ�������ڻ��Զ���Ӵ˷���������
#define  WM_UPDATESEARCHUSERRESULT   0x0402                              // ���������û�����������ڻ��Զ���Ӵ��û���

#define DEFAULT_XMPP_PORT 5222

// P2P���緢��/���յ��û���Ϣ��
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

// Server������Ϣ��
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