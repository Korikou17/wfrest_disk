#include "CloudDiskServer.h"
#include "Config.h"
#include "CryptoUtil.h"
#include "common.h"
#include "OssManager.h"
#include <iostream>
#include <nlohmann/json.hpp>
#include <wfrest/PathUtil.h>
#include <workflow/HttpUtil.h>
#include <workflow/MySQLResult.h>
#include <workflow/Workflow.h>
#include <workflow/mysql_types.h>

using namespace std;
using namespace std::placeholders;
using namespace wfrest;
using namespace protocol;
using json = nlohmann::json;

// 数据库的URL（从配置文件读取）
static const string DatabaseURL = "mysql://root:123@localhost/disk";
static const int RetryMax = 3;


void CloudDiskServer::register_routes()
{
    // 设置静态资源的路由
    register_www_module();
    register_auth_module();
    register_user_module();
    register_file_module();
    // ...
}


void CloudDiskServer::register_www_module()
{
    server_.Static("/", "./www/index.html");
    server_.Static("/static", "./www/static");
}

void CloudDiskServer::register_auth_module()
{   
    server_.POST("/api/v1/auth/register",[](const HttpReq *req,HttpResp *resp){
        json data = json::parse(req->body());
        if(req->content_type()!=APPLICATION_JSON){
            json result;
            result["status"]="error";
            result["message"]="请求格式有误";
            resp->set_status(400);
            resp->Json(result.dump());
            return;
        }

        string username = data["username"];
        string password=data["password"];
        string confirm=data["confirm"];

        if(username.empty()&&password.empty()&&confirm.empty())
        {
            json result;
            result["status"]="error";
            result["message"]="用户名和密码不能为空";
            resp->set_status(400);
            resp->Json(result.dump());
            return;
        }

        if(password!=confirm)
        {
            json result;
            result["status"]="error";
            result["message"]="两次输入的密码不一致";
            resp->set_status(400);
            resp->Json(result.dump());
            return;
        }
        
        // hash
        string salt = CryptoUtil::generate_salt();
        string hashcode = CryptoUtil::hash_password(password, salt);

        string sql = "INSERT INTO tbl_user (username, password, salt) VALUES ('" + username + "', '" + hashcode + "', '" + salt + "');";

        resp->MySQL(DatabaseURL, sql, [username,resp](MySQLResultCursor* cursor) {
            if (cursor->get_cursor_status() == MYSQL_STATUS_OK && cursor->get_affected_rows() == 1) {
                int userid=cursor->get_insert_id();
                json result;
                result["status"]="success";
                result["message"]="注册成功";
                result["data"]["userId"]=userid;
                result["data"]["username"]=username;
                resp->set_header_pair("Content-Type","application/json");
                resp->set_status(201);
                resp->Json(result.dump());
            }
            else{
                json result;
                result["status"]="error";
                result["message"]="用户名已存在";
                resp->set_status(409);
                resp->Json(result.dump());
            }
        });
    });
    
    server_.POST("/api/v1/auth/login",[](const HttpReq *req,HttpResp *resp){
        json data = json::parse(req->body());
        if(req->content_type()!=APPLICATION_JSON){
            json result;
            result["status"]="error";
            result["message"]="请求格式有误";
            resp->set_status(400);
            resp->Json(result.dump());
            return;
        }

        string username = data["username"];
        string password=data["password"];

        if(username.empty()&&password.empty())
        {
            json result;
            result["status"]="error";
            result["message"]="用户名和密码不能为空";
            resp->set_status(400);
            resp->Json(result.dump());
            return;
        }


        string sql = "SELECT * from tbl_user WHERE username='" + username + "';";

        resp->MySQL(DatabaseURL, sql, [password,resp](MySQLResultCursor* cursor) {
            if (cursor->get_cursor_status() != MYSQL_STATUS_GET_RESULT) {
                json result;
                result["status"]="error";
                result["message"]="内部服务器错误";
                resp->set_status(500);
                resp->Json(result.dump());
                return;
            }

            if (cursor->get_rows_count() != 1) {
                json result;
                result["status"]="error";
                result["message"]="用户名或密码错误";
                resp->Json(result.dump());
                resp->set_status(401);
                return;
            }

            vector<MySQLCell> record;
            cursor->fetch_row(record);
            User user;
            user.id = record[0].as_int();
            user.username = record[1].as_string();
            string query_password = record[2].as_string();
            string salt = record[3].as_string();
            user.createdAt = record[4].as_datetime();

            string hashcode = CryptoUtil::hash_password(password, salt);

            if(hashcode!=query_password)
            {
                json result;
                result["status"]="error";
                result["message"]="用户名或密码错误";
                resp->Json(result.dump());
                resp->set_status(401);
                return;
            }
            
            string token=CryptoUtil::generate_token(user);

            json result;
            result["status"]="success";
            result["message"]="登录成功";
            result["data"]["accessToken"]=token;
            result["data"]["tokenType"]="Bearer";
            result["data"]["user"]["userId"]=user.id;
            result["data"]["user"]["username"]=user.username;
            resp->set_header_pair("Content-Type","application/json");
            resp->set_status(200);
            resp->Json(result.dump());
        });
    });
}

void CloudDiskServer::register_user_module()
{
    server_.GET("/api/v1/user/me",[](const HttpReq *req,HttpResp *resp){
        string auth_str = req->header("Authorization");
        if(auth_str.empty() || auth_str.find("Bearer ") != 0)
        {
            json result;
            result["status"]="error";
            result["message"]="无效的访问令牌";
            resp->Json(result.dump());
            resp->set_status(401);
            return;
        }

        User user;
        string token = auth_str.substr(7);

        if (!CryptoUtil::verify_token(token, user)) {
            json result;
            result["status"]="error";
            result["message"]="无效的访问令牌";
            resp->Json(result.dump());
            resp->set_status(401);
            return;
        }

        json result;
        result["status"]="success";
        result["message"]="获取个人信息成功";
        result["data"]["userId"]=user.id;
        result["data"]["username"]=user.username;
        result["data"]["createdAt"]=user.createdAt;
        resp->set_header_pair("Content-Type","application/json");
        resp->set_status(200);
        resp->Json(result.dump());
    });
}

void CloudDiskServer::register_file_module()
{
    server_.GET("/api/v1/files",[](const HttpReq *req,HttpResp *resp){
        string auth_str = req->header("Authorization");
        if(auth_str.empty() || auth_str.find("Bearer ") != 0)
        {
            json result;
            result["status"]="error";
            result["message"]="无效的访问令牌";
            resp->Json(result.dump());
            resp->set_status(401);
            return;
        }

        User user;
        string token = auth_str.substr(7);

        if (!CryptoUtil::verify_token(token, user)) {
            json result;
            result["status"]="error";
            result["message"]="无效的访问令牌";
            resp->Json(result.dump());
            resp->set_status(401);
            return;
        }

        string sql="SELECT * FROM tbl_file WHERE uid = "+to_string(user.id)+"";
        
        resp->MySQL(DatabaseURL,sql,[resp](MySQLResultCursor *cursor){
            if(cursor->get_cursor_status()!=MYSQL_STATUS_GET_RESULT)
            {
                json result;
                result["status"]="error";
                result["message"]="内部服务器错误";
                resp->Json(result.dump());
                resp->set_status(500);
                return;
            }

            vector<MySQLCell> record;
            json result;
            result["status"]="success";
            result["message"]="获取文件列表成功";
            json filesArray=json::array();
            while(cursor->fetch_row(record))
            {
                json fileJson;
                fileJson["fileId"]=record[0].as_int();
                fileJson["filename"]=record[2].as_string();
                fileJson["size"]=record[4].as_int();
                fileJson["createdAt"]=record[5].as_date();
                fileJson["updatedAt"]=record[6].as_date();
                filesArray.push_back(fileJson);
            }
            result["data"]["files"]=filesArray;
            resp->Json(result.dump());
            resp->set_status(200);
            resp->add_header_pair("Content-Type"," application/json");

        });
    });

    server_.POST("/api/v1/files",[](const HttpReq *req,HttpResp *resp){
        if(req->content_type()!=MULTIPART_FORM_DATA)
        {
            json result;
            result["status"]="error";
            result["message"]="请求格式有误";
            resp->Json(result.dump());
            resp->set_status(400);
            return;
        }
        string auth_str = req->header("Authorization");
        if(auth_str.empty() || auth_str.find("Bearer ") != 0)
        {
            json result;
            result["status"]="error";
            result["message"]="无效的访问令牌";
            resp->Json(result.dump());
            resp->set_status(401);
            return;
        }

        User user;
        string token = auth_str.substr(7);

        if (!CryptoUtil::verify_token(token, user)) {
            json result;
            result["status"]="error";
            result["message"]="无效的访问令牌";
            resp->Json(result.dump());
            resp->set_status(401);
            return;
        }
        
        const Form& formData = req->form();
        for (const auto& [key, file] : formData) {
            string filename=file.first;
            string content=file.second;
            string basename = PathUtil::base(filename);
            int size=content.size();
            string hashcode = CryptoUtil::generate_hashcode(content.c_str(),size);
            string sql = "INSERT INTO tbl_file (uid, filename, hashcode, size) VALUES (" 
            + to_string(user.id) 
            + ",'" + basename + "'"
            + ",'" + hashcode + "'" 
            + "," + to_string(size) + ");";
            string username = user.username;
            resp->MySQL(DatabaseURL,sql,[username,basename,content,resp](MySQLResultCursor *cursor){
                if(cursor->get_cursor_status()!=MYSQL_STATUS_OK||cursor->get_affected_rows()!=1)
                {
                    json result;
                    result["status"]="error";
                    result["message"]="内部服务器错误";
                    resp->Json(result.dump());
                    resp->set_status(500);
                    return;
                }
                string dir_path="file/"+username+"";
                string file_path=dir_path+"/"+basename;
                mkdir(dir_path.c_str(),0755);

                resp->Save(file_path, move(content));
                OssManager::getInstance().upload(file_path,content);
                
                json result;
                int fileid=cursor->get_insert_id();
                result["status"]="success";
                result["message"]="上传成功";
                result["data"]["fileId"]=fileid;
                result["data"]["filename"]=basename;
                resp->Json(result.dump());
                resp->set_status(200);
                resp->add_header_pair("Content-Type","application/json");
                return;
            });
        }
    });

    server_.GET("/api/v1/file/{id}",[](const HttpReq *req,HttpResp *resp){
        string auth_str = req->header("Authorization");
        if(auth_str.empty() || auth_str.find("Bearer ") != 0)
        {
            json result;
            result["status"]="error";
            result["message"]="无效的访问令牌";
            resp->Json(result.dump());
            resp->set_status(401);
            return;
        }

        User user;
        string token = auth_str.substr(7);

        if (!CryptoUtil::verify_token(token, user)) {
            json result;
            result["status"]="error";
            result["message"]="无效的访问令牌";
            resp->Json(result.dump());
            resp->set_status(401);
            return;
        }
        
        string req_id_str = req->current_path().substr(13);
        cout<<req_id_str<<endl;
        string sql="SELECT * FROM tbl_file WHERE id = "+req_id_str+";";
        cout<<"[SQL]"<<sql<<endl;
        string username=user.username;
        resp->MySQL(DatabaseURL,sql,[username,resp](MySQLResultCursor *cursor){
            if(cursor->get_cursor_status()!=MYSQL_STATUS_GET_RESULT)
            {
                json result;
                result["status"]="error";
                result["message"]="内部服务器错误";
                resp->Json(result.dump());
                resp->set_status(500);
                return;
            }

            if(cursor->get_rows_count()!=1)
            {
                json result;
                result["status"]="error";
                result["message"]="文件不存在";
                resp->Json(result.dump());
                resp->set_status(404);
                return;
            }
            vector<MySQLCell> record;
            cursor->fetch_row(record);
            string filename=record[2].as_string();
            resp->set_status(200);
            resp->add_header_pair("Content-Disposition","attachment; filename="+filename+"");

            string file_path="file/"+username+"/"+filename;

            resp->File(file_path);

        });
    });
}

