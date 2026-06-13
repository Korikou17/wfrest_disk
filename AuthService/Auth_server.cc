#include "auth_service.srpc.h"
#include "workflow/WFFacilities.h"
#include <workflow/MySQLResult.h>
#include <workflow/WFTaskFactory.h>
#include <workflow/MySQLUtil.h>
#include <workflow/Workflow.h>
#include <workflow/MySQLMessage.h>
#include <workflow/WFTask.h>
#include "CryptoUtil.h"
#include <signal.h>

using namespace protocol;
using namespace srpc;
using namespace std;

static WFFacilities::WaitGroup wait_group(1);
static const string DatabaseURL = "mysql://root:123@localhost/disk";

void sig_handler(int signo)
{
	wait_group.done();
}

class AuthServiceServiceImpl : public AuthService::Service
{
public:

	void Register(RegisterReq *request, AuthResp *response, srpc::RPCContext *ctx) override
	{
		string username=request->username(); 
		string password=request->password(); 
		string confirm=request->confirm();
        if(password!=confirm)
        {
            response->set_code(400);
            response->set_message("两次输入的密码不一致");
            return;
        }

        string salt = CryptoUtil::generate_salt();
        string hashcode = CryptoUtil::hash_password(password, salt);

        string sql="INSERT INTO tbl_user (username, password, salt) VALUES ('" + username + "', '" + hashcode + "', '" + salt + "');";
        WFMySQLTask *task=WFTaskFactory::create_mysql_task(DatabaseURL,3,[response,username](WFMySQLTask *task){
            int state = task->get_state();
            if(state!=WFT_STATE_SUCCESS)
            {
                cerr<<WFGlobal::get_error_string(state,task->get_error())<<endl;
                return;
            }
            MySQLResponse *resp=task->get_resp();
            if(resp->get_packet_type()==MYSQL_PACKET_ERROR)
            {
                cerr<<"error code:"<<resp->get_error_code()<<" "
                <<"error msg:"<<resp->get_error_msg()<<endl;
                return;
            }

            MySQLResultCursor cursor(resp);
            if(cursor.get_affected_rows()!=1)
            {
                response->set_message("用户名已存在");
                response->set_code(409);
                return;
            }
            int userid=cursor.get_insert_id();
            response->set_message("注册成功");
            response->set_user_id(userid);
            response->set_username(username);
            response->set_code(201);
        });
        task->get_req()->set_query(sql);
        ctx->get_series()->push_back(task);
	}

	void Login(LoginReq *request, AuthResp *response, srpc::RPCContext *ctx) override
	{
		// TODO: fill server logic here
		string username=request->username(); 
		string password=request->password(); 

        string sql = "SELECT * from tbl_user WHERE username='" + username + "';";
        WFMySQLTask *task=WFTaskFactory::create_mysql_task(DatabaseURL,3,[response,password](WFMySQLTask *task){
            int state = task->get_state();
            if(state!=WFT_STATE_SUCCESS)
            {
                response->set_message("内部服务器错误");
                response->set_code(500);
                
                cerr<<WFGlobal::get_error_string(state,task->get_error())<<endl;
                return;
            }
            MySQLResponse *resp=task->get_resp();
            if(resp->get_packet_type()==MYSQL_PACKET_ERROR)
            {
                response->set_message("内部服务器错误");
                response->set_code(500);
                
                cerr<<"error code:"<<resp->get_error_code()<<" "
                <<"error msg:"<<resp->get_error_msg()<<endl;
                return;
            }

            MySQLResultCursor cursor(resp);

            if (cursor.get_rows_count() != 1) {
                response->set_message("用户名或密码错误");
                response->set_code(401);
                return;
            }

            vector<MySQLCell> record;
            cursor.fetch_row(record);
            User user;
            user.id = record[0].as_int();
            user.username = record[1].as_string();
            string query_password = record[2].as_string();
            string salt = record[3].as_string();
            user.createdAt = record[4].as_datetime();

            string hashcode = CryptoUtil::hash_password(password, salt);

            if(hashcode!=query_password)
            {
                response->set_message("用户名或密码错误");
                response->set_code(401);
                return;
            }

            string token=CryptoUtil::generate_token(user);
            
            response->set_message("登录成功");
            response->set_user_id(user.id);
            response->set_username(user.username);
            response->set_token(token);
            response->set_token_type("Bearer");
            response->set_code(200);
        });
        task->get_req()->set_query(sql);
        ctx->get_series()->push_back(task);
	}


	void VerifyToken(VerifyTokenReq *request, VerifyTokenResp *response, srpc::RPCContext *ctx) override
	{
		// TODO: fill server logic here
	}
};

int main()
{
	GOOGLE_PROTOBUF_VERIFY_VERSION;
    signal(SIGINT,sig_handler);

	unsigned short port = 1412;
	SRPCServer server;

	AuthServiceServiceImpl authservice_impl;
	server.add_service(&authservice_impl);

	server.start(port);
	wait_group.wait();
	server.stop();
	google::protobuf::ShutdownProtobufLibrary();
	return 0;
}
