#include <string.h>
#include <stdlib.h>
#include <Windows.h>
#include <Shlwapi.h>

#include "db_interface.h"
#include "sqlite3.h"
#include "c_util\v6string.c"

#define CHECK_RC(rc,szErrMsg,db) \
	if(rc!=SQLITE_OK) \
{ \
sqlite3_free(szErrMsg); \
sqlite3_close(db); \
return false; \
}\

bool open_db(sqlite3 **db, const char *db_path)
{
	int len = ::MultiByteToWideChar(CP_ACP, 0, db_path, -1, NULL, 0);
    if (len == 0) return false;
	wchar_t *wdb_path = (wchar_t *)malloc(sizeof(wchar_t)*len);
	::MultiByteToWideChar(CP_ACP, 0, db_path, -1, wdb_path, len);

	char *errmsg=0;//记录返回的错误信息。

	wchar_t wpath[MAX_PATH];
	GetModuleFileName(NULL,wpath,MAX_PATH);
	PathAppend(wpath, wdb_path);

	free(wdb_path);

	len = ::WideCharToMultiByte(CP_UTF8, 0, wpath, -1, NULL, 0, NULL, NULL);
	if (len == 0) return false;
	char *upath = (char *)malloc(len);
	::WideCharToMultiByte(CP_UTF8, 0, wpath, -1, upath, len, NULL, NULL);

	int rc=sqlite3_open(upath,db);//打开数据库

	free(upath);
	CHECK_RC(rc,errmsg,*db);

	return true;
}


int insert_myProfile(const char *name,const char *sex,const char *age,const char *introduction,const char *head_image_path, const char *v4_port, const char *v6_port)
{
	char *errmsg=0;//记录返回的错误信息。
	sqlite3 *db=0; //记录返回的数据库句柄

	if(!open_db(&db, MYINFO_DBFILE_NAME))
		return false;

	//下述函数进行数据库添加操作
	char *szBaseSql = "insert into my_profile(nick_name,sex,age,introduction,head_image_path,v4_port,v6_port) values('";
	char *szSql = v6strcat(15, szBaseSql, name, "','", sex, "','", age, "','",introduction,"','",head_image_path,"','",v4_port,"','",v6_port,"')");
	int rc=sqlite3_exec(db,szSql,0,0,&errmsg);
	release_db_data((void **)&szSql);
	CHECK_RC(rc,errmsg,db);

	//下述函数获取数据库插入的数据的ID
	int id = (int) sqlite3_last_insert_rowid(db);

	sqlite3_close(db);
	return id;
}


int update_myProfile(const char *id,const char *name,const char *sex,const char *age,const char *introduction,const char *head_image_path, const char *v4_port, const char *v6_port)
{
	char *errmsg=0;//记录返回的错误信息。
	sqlite3 *db=0; //记录返回的数据库句柄

	if(!open_db(&db, MYINFO_DBFILE_NAME))
		return false;

	int iid = atoi(id);
	char *szSql = NULL;
	//下述函数进行数据库添加操作
	if(iid == 0)
	{
	    char *szBaseSql = "insert into my_profile(nick_name,sex,age,introduction,head_image_path,v4_port,v6_port) values('";
	    szSql = v6strcat(15, szBaseSql, name, "','", sex, "','", age, "','",introduction,"','",head_image_path,"','",v4_port,"','",v6_port,"')");
	}
	else
	{
		char *szBaseSql = "insert or replace into my_profile(id,nick_name,sex,age,introduction,head_image_path,v4_port,v6_port) values('";
	    szSql = v6strcat(17, szBaseSql, id, "','", name, "','", sex, "','", age, "','",introduction,"','",head_image_path,"','",v4_port,"','",v6_port,"')");
	}
	int rc=sqlite3_exec(db,szSql,0,0,&errmsg);
	release_db_data((void **)&szSql);
	CHECK_RC(rc,errmsg,db);

	//下述函数获取数据库插入的数据的ID
	if(iid == 0)
	    iid = (int) sqlite3_last_insert_rowid(db);

	sqlite3_close(db);

	return iid;
}


bool get_myProfileById(const char *id, char **name, unsigned short *sex, unsigned int *age, char **introduction,char **head_image_path, int *v4_port, int *v6_port)
{
	int nrows,ncols; //nrows:记录返回数据库表的行数，ncols:记录返回表的列数
	char *errmsg=0;//记录返回的错误信息。
	char **results;//记录返回的查询结果
	sqlite3 *db=0; //记录返回的数据库句柄

	if(!open_db(&db, MYINFO_DBFILE_NAME))
		return false;

	//下述函数进行查询数据库操作
	char *szBaseSql = "select * from my_profile where id='";
	char *szSql = v6strcat(3, szBaseSql, id, "';");
	int rc=sqlite3_get_table(db,szSql,&results,&nrows,&ncols,&errmsg);
	release_db_data((void **)&szSql);
	CHECK_RC(rc,errmsg,db);

	bool success = true;
	if(nrows > 0)
	{
		int buf_size = strlen(results[MYINFO_DB_ROWS+1])+1;
		*name  = (char*)malloc(buf_size);
		memcpy(*name, results[MYINFO_DB_ROWS+1], buf_size);
	    *sex = atoi(results[MYINFO_DB_ROWS+2]);
		*age = atoi(results[MYINFO_DB_ROWS+3]);
		buf_size = strlen(results[MYINFO_DB_ROWS+4])+1;
		*introduction = (char*)malloc(buf_size);
		memcpy(*introduction, results[MYINFO_DB_ROWS+4], buf_size);
		buf_size = strlen(results[MYINFO_DB_ROWS+5])+1;
		*head_image_path = (char*)malloc(buf_size);
		memcpy(*head_image_path, results[MYINFO_DB_ROWS+5], buf_size);
		*v4_port = atoi(results[MYINFO_DB_ROWS+6]);
		*v6_port = atoi(results[MYINFO_DB_ROWS+7]);
	}
	else
		success = false;

	sqlite3_free_table(results);
	sqlite3_close(db);
	return success;
}


bool get_myProfileByName(const char *name, unsigned int *id, unsigned short *sex, unsigned int *age, char **introduction,char **head_image_path, int *v4_port, int *v6_port)
{
	int nrows,ncols; //nrows:记录返回数据库表的行数，ncols:记录返回表的列数
	char *errmsg=0;//记录返回的错误信息。
	char **results;//记录返回的查询结果
	sqlite3 *db=0; //记录返回的数据库句柄

	if(!open_db(&db, MYINFO_DBFILE_NAME))
		return false;

	//下述函数进行查询数据库操作
	char *szBaseSql = "select * from my_profile where nick_name='";
	char *szSql = v6strcat(3, szBaseSql, name, "';");
	int rc=sqlite3_get_table(db,szSql,&results,&nrows,&ncols,&errmsg);
	release_db_data((void **)&szSql);
	CHECK_RC(rc,errmsg,db);

	bool success = true;
	if(nrows > 0)
	{
		*id  = atoi(results[MYINFO_DB_ROWS]);
	    *sex = atoi(results[MYINFO_DB_ROWS+2]);
		*age = atoi(results[MYINFO_DB_ROWS+3]);
		int buf_size = strlen(results[MYINFO_DB_ROWS+4])+1;
		*introduction = (char*)malloc(buf_size);
		memcpy(*introduction, results[MYINFO_DB_ROWS+4], buf_size);
		buf_size = strlen(results[MYINFO_DB_ROWS+5])+1;
		*head_image_path = (char*)malloc(buf_size);
		memcpy(*head_image_path, results[MYINFO_DB_ROWS+5], buf_size);
		*v4_port = atoi(results[MYINFO_DB_ROWS+6]);
		*v6_port = atoi(results[MYINFO_DB_ROWS+7]);
	}
	else
		success = false;

	sqlite3_free_table(results);
	sqlite3_close(db);
	return success;
}


bool get_peersProfile(char ***peers_table, int *rows, int *cols)
{
	char *errmsg=0;//记录返回的错误信息。
	sqlite3 *db=0; //记录返回的数据库句柄

	if(!open_db(&db, PEERSINFO_DBFILE_NAME))
		return false;

	//下述函数进行查询数据库操作
	char *szSql = "select * from peers_profile";
	int rc=sqlite3_get_table(db,szSql,peers_table,rows,cols,&errmsg);
	CHECK_RC(rc,errmsg,db);

	bool success = true;
	if(SQLITE_OK != errmsg)
		success = false;

	sqlite3_close(db);
	return success;
}


bool clear_peersProfile()
{
	char *errmsg=0;//记录返回的错误信息。
	sqlite3 *db=0; //记录返回的数据库句柄

	if(!open_db(&db, PEERSINFO_DBFILE_NAME))
		return false;

	//下述函数进行数据库添加操作
	char *szSql = "delete from peers_profile";
	int rc=sqlite3_exec(db,szSql,0,0,&errmsg);
	CHECK_RC(rc,errmsg,db);

	sqlite3_close(db);
	return true;
}


bool get_peerProfileById(const char *uid, char **nick_name, char **group_name, int *sex, int *age, char **introduction, char **head_image_path, int *server_id, char **server_uid, char **last_ip, int *tcp_port, int *udp_port)
{
	int nrows,ncols; //nrows:记录返回数据库表的行数，ncols:记录返回表的列数
	char *errmsg=0;//记录返回的错误信息。
	char **results;//记录返回的查询结果
	sqlite3 *db=0; //记录返回的数据库句柄

	if(!open_db(&db, PEERSINFO_DBFILE_NAME))
		return false;

	//下述函数进行查询数据库操作
	char *szBaseSql = "select * from peers_profile where uid='";
	char *szSql = v6strcat(3, szBaseSql, uid, "';");
	int rc=sqlite3_get_table(db,szSql,&results,&nrows,&ncols,&errmsg);
	release_db_data((void **)&szSql);
	CHECK_RC(rc,errmsg,db);

	bool success = true;
	if(nrows > 0)
	{
		int buf_size = strlen(results[PEERSINFO_DB_ROWS+1])+1;
		*nick_name  = (char*)malloc(buf_size);
		memcpy(*nick_name, results[PEERSINFO_DB_ROWS+1], buf_size);
		buf_size = strlen(results[PEERSINFO_DB_ROWS+2])+1;
		*group_name  = (char*)malloc(buf_size);
		memcpy(*group_name, results[PEERSINFO_DB_ROWS+2], buf_size);
	    *sex = atoi(results[PEERSINFO_DB_ROWS+3]);
		*age = atoi(results[PEERSINFO_DB_ROWS+4]);
		buf_size = strlen(results[PEERSINFO_DB_ROWS+5])+1;
		*introduction = (char*)malloc(buf_size);
		memcpy(*introduction, results[PEERSINFO_DB_ROWS+5], buf_size);
		buf_size = strlen(results[PEERSINFO_DB_ROWS+6])+1;
		*head_image_path = (char*)malloc(buf_size);
		memcpy(*head_image_path, results[PEERSINFO_DB_ROWS+6], buf_size);
		*server_id = atoi(results[PEERSINFO_DB_ROWS+7]);
		buf_size = strlen(results[PEERSINFO_DB_ROWS+8])+1;
		*server_uid = (char*)malloc(buf_size);
		memcpy(*server_uid, results[PEERSINFO_DB_ROWS+8], buf_size);
		buf_size = strlen(results[PEERSINFO_DB_ROWS+9])+1;
		*last_ip = (char*)malloc(buf_size);
		memcpy(*last_ip, results[PEERSINFO_DB_ROWS+9], buf_size);
		*tcp_port = atoi(results[PEERSINFO_DB_ROWS+10]);
		*udp_port = atoi(results[PEERSINFO_DB_ROWS+11]);
	}
	else
		success = false;

	sqlite3_free_table(results);
	sqlite3_close(db);
	return success;
}


int insert_peerProfile(const char *nick_name, const char *group_name, const char *sex, const char *age, const char *introduction, const char *head_image_path, const char *server_id, const char *server_uid, const char *last_ip, const char *tcp_port, const char *udp_port)
{
	char *errmsg=0;//记录返回的错误信息。
	sqlite3 *db=0; //记录返回的数据库句柄

	if(!open_db(&db, PEERSINFO_DBFILE_NAME))
		return false;

	// 下述函数进行数据库添加操作
	char *szBaseSql = "insert into peers_profile(nick_name,group_name,sex,age,introduction,head_image_path,server_id,server_uid,last_ip,last_tcp_port,last_udp_port) values('";
	char *szSql = v6strcat(23, szBaseSql, nick_name, "','", group_name, "','", sex, "','",age, "','",introduction, "','",head_image_path, "','",server_id, "','",server_uid, "','",last_ip, "','",tcp_port, "','",udp_port,"')");
	int rc=sqlite3_exec(db,szSql,0,0,&errmsg);
	release_db_data((void **)&szSql);
	CHECK_RC(rc,errmsg,db);

	// 下述函数获取数据库插入的数据的ID
	int id = (int) sqlite3_last_insert_rowid(db);

	sqlite3_close(db);
	return id;
}


bool insert_peersProfile(const insert_peer_profile_list peers_list)
{
	char *errmsg=0;//记录返回的错误信息。
	sqlite3 *db=0; //记录返回的数据库句柄

	if(!open_db(&db, PEERSINFO_DBFILE_NAME))
		return false;

	int rc = sqlite3_exec(db, "begin transaction", 0, 0, &errmsg);       // 开启事务处理
	CHECK_RC(rc,errmsg,db);

	const new_peer_profile* p = peers_list->next;
	while(p)
	{
		char *szBaseSql = "insert into peers_profile(nick_name,group_name,sex,age,introduction,head_image_path,server_id,server_uid,last_ip,last_tcp_port,last_udp_port) values('";
		char *szSql = v6strcat(23, szBaseSql, p->nick_name, "','", p->group_name, "','", p->sex, "','",p->age, "','",p->introduction, "','",p->head_image_path, "','",p->server_id, "','",p->server_uid, "','",p->last_ip, "','",p->tcp_port, "','",p->udp_port,"')");
		int rc=sqlite3_exec(db,szSql,0,0,&errmsg);
		release_db_data((void **)&szSql);
		if(rc!=SQLITE_OK)
		{
			sqlite3_free(errmsg);
		}

		p = p->next;
	}

	rc = sqlite3_exec(db, "commit transaction", 0, 0, &errmsg);          // 提交事务
	CHECK_RC(rc,errmsg,db);

	sqlite3_close(db);
	return true;
}


int last_insert_peerProfileUid()
{
	char *errmsg=0;//记录返回的错误信息。
	sqlite3 *db=0; //记录返回的数据库句柄

	if(!open_db(&db, PEERSINFO_DBFILE_NAME))
		return 0;

	// 下述函数获取数据库最后一条数据的ID
	sqlite3_stmt *stmt;

	char *szBaseSql = "select seq from sqlite_sequence where name='peers_profile'";
	sqlite3_prepare(db,
		szBaseSql,
		-1,&stmt,0);
	int rc = sqlite3_step(stmt);

	sqlite3_int64 lid = 0;
	if(rc == SQLITE_ROW)
	{
		lid = sqlite3_column_int64(stmt,0);
	}

	sqlite3_finalize(stmt);

	sqlite3_close(db);
	return lid;
}


int update_peerProfile(const char *uid, const char *nick_name, const char *group_name, const char *sex, const char *age, const char *introduction, const char *head_image_path, const char *server_id, const char *server_uid, const char *last_ip, const char *tcp_port, const char *udp_port)
{
	char *errmsg=0;//记录返回的错误信息。
	sqlite3 *db=0; //记录返回的数据库句柄

	if(!open_db(&db, PEERSINFO_DBFILE_NAME))
		return false;

	// 替换introduction字段中'为''
	char* newintro = replacestr(introduction, "'", "''");

	//下述函数进行数据库添加操作
	int id = atoi(uid);
	char *szSql = NULL;
	if(id == 0)
	{
	    char *szBaseSql = "insert into peers_profile(nick_name,group_name,sex,age,introduction,head_image_path,server_id,server_uid,last_ip,last_tcp_port,last_udp_port) values('";
	    szSql = v6strcat(23, szBaseSql, nick_name, "','", group_name, "','", sex, "','",age, "','",newintro, "','",head_image_path, "','",server_id, "','",server_uid, "','",last_ip, "','",tcp_port, "','",udp_port,"')");
	}
	else
	{
		char *szBaseSql = "insert or replace into peers_profile(uid,nick_name,group_name,sex,age,introduction,head_image_path,server_id,server_uid,last_ip,last_tcp_port,last_udp_port) values('";
	    szSql = v6strcat(25, szBaseSql, uid, "','", nick_name, "','", group_name, "','", sex, "','",age, "','",newintro, "','",head_image_path, "','",server_id, "','",server_uid, "','",last_ip, "','",tcp_port, "','",udp_port,"')");
	}
	int rc=sqlite3_exec(db,szSql,0,0,&errmsg);
	release_db_data((void **)&szSql);
	free(newintro);

	CHECK_RC(rc,errmsg,db);

	//下述函数获取数据库插入的数据的ID
	if(id == 0)
	    id = (int) sqlite3_last_insert_rowid(db);

	sqlite3_close(db);
	return id;
}


bool delete_peerProfile(const char *uid)
{
	char *errmsg=0;//记录返回的错误信息。
	sqlite3 *db=0; //记录返回的数据库句柄

	if(!open_db(&db, PEERSINFO_DBFILE_NAME))
		return false;

	//下述函数进行数据库添加操作
	char *szSql = v6strcat(2, "delete from peers_profile where uid = ", uid);
	int rc=sqlite3_exec(db,szSql,0,0,&errmsg);
	CHECK_RC(rc,errmsg,db);

	sqlite3_close(db);
	return true;
}


bool get_serversInformation(char ***servers_table, int *rows, int *cols)
{
	char *errmsg=0;//记录返回的错误信息。
	sqlite3 *db=0; //记录返回的数据库句柄

	if(!open_db(&db, SERVERSINFO_DBFILE_NAME))
		return false;

	//下述函数进行查询数据库操作
	char *szSql = "select * from server_information";
	int rc=sqlite3_get_table(db,szSql,servers_table,rows,cols,&errmsg);
	CHECK_RC(rc,errmsg,db);

	bool success = true;
	if(SQLITE_OK != errmsg)
		success = false;

	sqlite3_close(db);
	return success;
}


bool clear_serversInformation()
{
	char *errmsg=0;//记录返回的错误信息。
	sqlite3 *db=0; //记录返回的数据库句柄

	if(!open_db(&db, SERVERSINFO_DBFILE_NAME))
		return false;

	//下述函数进行数据库添加操作
	char *szSql = "delete * from server_information";
	int rc=sqlite3_exec(db,szSql,0,0,&errmsg);
	CHECK_RC(rc,errmsg,db);

	sqlite3_close(db);
	return true;
}


int insert_serverInformation(const char *name, const char *ip, const char *port, const char *description, const char *uid, const char *upassword)
{
	char *errmsg=0;//记录返回的错误信息。
	sqlite3 *db=0; //记录返回的数据库句柄

	if(!open_db(&db, SERVERSINFO_DBFILE_NAME))
		return false;

	//下述函数进行数据库添加操作
	char *szBaseSql = "insert into server_information(name,ip,port,description,uid,upassword) values('";
	char *szSql = v6strcat(13, szBaseSql, name, "','", ip, "','", port, "','",description, "','",uid, "','",upassword,"')");
	int rc=sqlite3_exec(db,szSql,0,0,&errmsg);
	release_db_data((void **)&szSql);
	CHECK_RC(rc,errmsg,db);

	//下述函数获取数据库插入的数据的ID
	int id = (int) sqlite3_last_insert_rowid(db);

	sqlite3_close(db);
	return id;
}


int update_serverInformation(const char *sid, const char *name, const char *ip, const char *port, const char *description, const char *uid, const char *upassword)
{
	char *errmsg=0;//记录返回的错误信息。
	sqlite3 *db=0; //记录返回的数据库句柄

	if(!open_db(&db, SERVERSINFO_DBFILE_NAME))
		return false;

	//下述函数进行数据库添加操作
	int id = atoi(sid);
	char *szSql = NULL;
	if(id == 0)
	{
	    char *szBaseSql = "insert into server_information(name,ip,port,description,uid,upassword) values('";
		char *szSql = v6strcat(13, szBaseSql, name, "','", ip, "','", port, "','",description, "','",uid, "','",upassword,"')");
	}
	else
	{
		char *szBaseSql = "insert or replace into server_information(id,name,ip,port,description,uid,upassword) values('";
	    szSql = v6strcat(15, szBaseSql, sid, "','", name, "','", ip, "','", port, "','",description, "','",uid, "','",upassword,"')");
	}
	int rc=sqlite3_exec(db,szSql,0,0,&errmsg);
	release_db_data((void **)&szSql);

	CHECK_RC(rc,errmsg,db);

	//下述函数获取数据库插入的数据的ID
	if(id == 0)
	    id = (int) sqlite3_last_insert_rowid(db);

	sqlite3_close(db);
	return id;
}


int insert_chatItem(const char *uid, const char *name, const char *item, char **insert_time)
{
	char *errmsg=0;//记录返回的错误信息。
	sqlite3 *db=0; //记录返回的数据库句柄

	if(!open_db(&db, CHATHISTORY_DBFILE_NAME))
		return false;

	//下述函数进行判断表是否存在，如不存在则建立表
	char *szBaseSql = "select max(id) from chat_with_";
	char *szSql = v6strcat(2, szBaseSql, uid);
	int rc=sqlite3_exec(db,szSql,0,0,&errmsg);
	release_db_data((void **)&szSql);

	if(rc != SQLITE_OK)
	{
		sqlite3_free(errmsg);

		szBaseSql = "create table chat_with_";
		char *szBackSql = "(id integer primary key autoincrement, name text, content text, chat_time TimeStamp NOT NULL DEFAULT (datetime('now','localtime')))";
		szSql = v6strcat(3, szBaseSql, uid, szBackSql);
		rc=sqlite3_exec(db,szSql,0,0,&errmsg);
		release_db_data((void **)&szSql);
		CHECK_RC(rc,errmsg,db);
	}

	//下述函数进行数据库添加操作
	szBaseSql = "insert into chat_with_";
	char *szBackSql = "(name,content) values('";
	szSql = v6strcat(7, szBaseSql, uid, szBackSql, name, "','", item, "')");
	rc=sqlite3_exec(db,szSql,0,0,&errmsg);
	release_db_data((void **)&szSql);
	CHECK_RC(rc,errmsg,db);

	//下述函数获取数据库插入的数据的ID
	int id = (int) sqlite3_last_insert_rowid(db);

	if (id != 0)
	{
		sqlite3_stmt *stmt;
		char   tmp[16];

		szBaseSql = "select chat_time from chat_with_";
		szBackSql = " where id=";
		sprintf(tmp, "%d", id);

		szSql = v6strcat(4, szBaseSql, uid, szBackSql, tmp);
		sqlite3_prepare(db,
                szSql,
                -1,&stmt,0);
		rc = sqlite3_step(stmt);

		if(rc == SQLITE_ROW)
		{
			const char *t = (const char *)sqlite3_column_text(stmt,0);
			if(t && insert_time)
			{
				int buf_size = strlen(t)+1;
				*insert_time = (char *)malloc(buf_size);
				memcpy(*insert_time, t, buf_size);
			}
		}
		
		sqlite3_finalize(stmt);
		release_db_data((void **)&szSql);
	}

	sqlite3_close(db);
	return id;
}


bool get_chatItems(const char *uid, const char *id, const char *num, char ***chat_table, int *rows, int *cols)
{
	char *errmsg=0;//记录返回的错误信息。
	sqlite3 *db=0; //记录返回的数据库句柄

	if(!open_db(&db, CHATHISTORY_DBFILE_NAME))
		return false;

	//下述函数进行查询数据库操作
	char *szBaseSql = "select * from chat_with_";
	char *szSql = NULL;
	int iid = atoi(id);
	if(iid > -1)
	{
		szSql = v6strcat(6, szBaseSql, uid, " limit ", id, ",", num);
	}
	else
	{
		szSql = v6strcat(4, szBaseSql, uid, " order by id desc limit ", num);
	}
	int rc=sqlite3_get_table(db,szSql,chat_table,rows,cols,&errmsg);
	CHECK_RC(rc,errmsg,db);

	bool success = true;
	if(SQLITE_OK != errmsg)
		success = false;

	sqlite3_close(db);
	return success;
}


void release_db_data(void **p)
{
	if(*p)
		free(*p);
	*p=0;
}

void release_indb_table_data(char **p)
{
	sqlite3_free_table(p);
}