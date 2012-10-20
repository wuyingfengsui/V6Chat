#ifndef _DB_INTERFACE_H_
#define _DB_INTERFACE_H_

#ifdef __cplusplus
extern "C" {
#endif

#define MYINFO_DBFILE_NAME "..\\..\\UserData\\My\\my_info.db"
#define MYINFO_DB_ROWS     8

#define PEERSINFO_DBFILE_NAME "..\\..\\UserData\\peers_info.db"
#define PEERSINFO_DB_ROWS     12

#define SERVERSINFO_DBFILE_NAME "..\\..\\UserData\\servers_info.db"
#define SERVERSINFO_DB_ROWS     7

#define CHATHISTORY_DBFILE_NAME "..\\..\\UserData\\chat_history.db"
#define CHATHISTORY_DB_ROWS     4

typedef struct New_Peer_Profile
{
	char* nick_name;
	char* group_name;
	char* sex;
	char* age;
	char* introduction;
	char* head_image_path;
	char* server_id;
	char* server_uid;
	char* last_ip;
	char* tcp_port;
	char* udp_port;

	struct New_Peer_Profile* next;
}*insert_peer_profile_list, new_peer_profile;

	//添加自己的用户信息，返回此条信息的id（失败返回0）
	int insert_myProfile(const char *name,const char *sex,const char *age,const char *introduction,const char *head_image_path, const char *v4_port, const char *v6_port);
	//更新自己的用户信息，如不存在则添加（也可传入id为0来添加），返回此条信息的id（失败返回0）
	int update_myProfile(const char *id,const char *name,const char *sex,const char *age,const char *introduction,const char *head_image_path, const char *v4_port, const char *v6_port);
	//根据id获取自己的用户信息，需调用release_db_data释放name及introduction,head_image_path
    bool get_myProfileById(const char *id, char **name, unsigned short *sex, unsigned int *age, char **introduction, char **head_image_path, int *v4_port, int *v6_port);
	//根据nick_name获取自己的用户信息，需调用release_db_data释放introduction,head_image_path
    bool get_myProfileByName(const char *name, unsigned int *id, unsigned short *sex, unsigned int *age, char **introduction, char **head_image_path, int *v4_port, int *v6_port);

	//获取所有好友信息，需调用release_indb_table_data释放peers_table
	bool get_peersProfile(char ***peers_table, int *rows, int *cols);
	//清除所有好友信息
	bool clear_peersProfile();
	//根据uid获取好友的用户信息，需调用release_db_data释放nick_name，group_name及introduction,head_image_path,server_uid,last_ip
	bool get_peerProfileById(const char *uid, char **nick_name, char **group_name, int *sex, int *age, char **introduction, char **head_image_path, int *server_id, char **server_uid, char **last_ip, int *tcp_port, int *udp_port);
	//添加一条好友信息，返回此条信息的id（失败返回0）
	int insert_peerProfile(const char *nick_name, const char *group_name, const char *sex, const char *age, const char *introduction, const char *head_image_path, const char *server_id, const char *server_uid, const char *last_ip, const char *tcp_port, const char *udp_port);
	// 添加多条好友信息
	bool insert_peersProfile(const insert_peer_profile_list peers_list);
	// 返回添加的最后一条好友信息的uid
	int last_insert_peerProfileUid();
	//更新一条好友信息，如不存在则添加（也可传入uid为0来添加），返回此条信息的id（失败返回0）
	int update_peerProfile(const char *uid, const char *nick_name, const char *group_name, const char *sex, const char *age, const char *introduction, const char *head_image_path, const char *server_id, const char *server_uid, const char *last_ip, const char *tcp_port, const char *udp_port);
	// 删除一条好友信息
	bool delete_peerProfile(const char *uid);

	//获取所有服务端信息，需调用release_indb_table_data释放servers_table
	bool get_serversInformation(char ***servers_table, int *rows, int *cols);
	//清除所有服务端信息
	bool clear_serversInformation();
	//添加一条服务端信息，返回此条信息的id（失败返回0）
	int insert_serverInformation(const char *name, const char *ip, const char *port, const char *description, const char *uid, const char *upassword);
	// 更新一条服务端信息，如不存在则添加（也可传入sid为0来添加），返回此条信息的id（失败返回0）
	int update_serverInformation(const char *sid, const char *name, const char *ip, const char *port, const char *description, const char *uid, const char *upassword);

	// 添加一条聊天记录，需调用release_db_data释放insert_time，返回此条记录的id（失败返回0）
	int insert_chatItem(const char *uid, const char *name, const char *item, char **insert_time);
	// 返回指定区间的聊天记录，需调用release_indb_table_data释放chat_table，当*id为-1时返回逆序的最近的数据
	bool get_chatItems(const char *uid, const char *id, const char *num, char ***chat_table, int *rows, int *cols);

	void release_db_data(void **p);
	void release_indb_table_data(char **p);

#ifdef __cplusplus
}
#endif

#endif