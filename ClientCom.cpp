#include "ClientCom.h"
#include <exception>
#include "cpprest/json.h"
#include "cpprest/http_listener.h"
#include "cpprest/uri.h"
#include "cpprest/asyncrt_utils.h"
#include "cpprest/http_client.h"


#include "log.h"
#include "../config.h"

#define SEND_HTTP_NOTIFY_TIMEOUT_S 10



	using namespace web;
	using namespace http;
	using namespace utility;
	using namespace http::experimental::listener;

#ifdef _UTF16_STRINGS
#define string_t_to_std_string(str) utility::conversions::utf16_to_utf8(str)
#define std_string_to_string_t(str) utility::conversions::utf8_to_utf16(str)
#else
#define string_t_to_std_string(str) str
#define std_string_to_string_t(str) str
#endif

// cors£º"Cross-origin resource sharing""¿çÓò×ÊÔ´¹²Ïí"
static pplx::task<void> cors_reply(http_request &message, http::status_code status, const std::string &body, bool is_json=false)
{
	http_response response(status);
	response.headers().add(U("Access-Control-Allow-Origin"), U("*"));
	if (is_json)
		response.headers().add(U("Content-Type"), U("application/json"));
	response.set_body(body);
	return message.reply(response);
}

ClientCom::ClientCom()
{
	m_listener = NULL;
}

ClientCom::~ClientCom()
{
	stop();

}

int ClientCom::start()
{
	m_url = Config::GetInstance().clicom_param.listen_url;

	try
	{
		m_listener = new http_listener(conversions::to_string_t(m_url));
		if (m_listener==NULL) return -1;

		m_listener->support(methods::POST, std::bind(&ClientCom::handle_http_request, this, std::placeholders::_1));
		m_listener->support(methods::OPTIONS, std::bind(&ClientCom::handle_http_options, this, std::placeholders::_1));
		m_listener->open().wait();
	}
	catch (const std::exception &e)
	{
		log_fatal("listen at: %s fail, %s", m_url.c_str(), e.what());
		return -1;
	}

	log_message("ClientCom start OK, listen at: %s", m_url.c_str());

	return 0;
}

void ClientCom::stop()
{
	if (m_listener!=NULL)
	{
		try
		{
			m_listener->close().wait();
		}
		catch (const std::exception &e)
		{
			log_fatal("stop listen fail, %s", e.what());
		}

		delete m_listener;
		m_listener = NULL;
	}
}


void ClientCom::handle_http_request(http_request message)
{
	//ucout << message.to_string() << std::endl;
	log_message("recv %s request, path:%s", string_t_to_std_string(message.method()).c_str(),
		string_t_to_std_string(http::uri::decode(message.relative_uri().path())).c_str());

	if (message.method() != methods::POST)
	{
		cors_reply(message, status_codes::MethodNotAllowed, "MethodNotAllowed");
		return;
	}

        std::string path;
        std::string body;
        std::string app_key;
        std::string user_id;
//        cmd_t cmd_type = cmd_empty;
        try
        {
                utility::string_t tpath = http::uri::decode(message.relative_uri().path());
                path = string_t_to_std_string(tpath);
              //  cmd_type = path_to_cmd_type(path);
                utility::string_t content = message.extract_string().get();
                body = string_t_to_std_string(content);
                app_key = string_t_to_std_string(message.headers()[U("app_key")]);
                user_id = string_t_to_std_string(message.headers()[U("user_id")]);
        }
        catch (const std::exception &e)
        {
                log_error("%s", e.what());
                cors_reply(message, status_codes::BadRequest, "BadRequest, server explain content exception");
                return;
        }
        cors_reply(message, status_codes::OK, "0");
}

void ClientCom::handle_http_options(web::http::http_request message)
{
	//ucout << message.to_string() << std::endl;
	log_message("recv %s request, path:%s", string_t_to_std_string(message.method()).c_str(), 
			string_t_to_std_string(http::uri::decode(message.relative_uri().path())).c_str());

	http_response response(status_codes::OK);
	response.headers().add(U("Allow"), U("POST, OPTIONS"));
	response.headers().add(U("Access-Control-Allow-Origin"), U("*"));
	response.headers().add(U("Access-Control-Allow-Methods"), U("POST, OPTIONS"));
	response.headers().add(U("Access-Control-Allow-Headers"), U("Content-Type, app_key"));
	message.reply(response);
}


