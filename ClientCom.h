#ifndef __CLIENT_COM_H__
#define __CLIENT_COM_H__

#include <string>
#include <functional>
#include <map>
#include "thread.h"
#include "cpprest/http_listener.h"
#include "cpprest/http_msg.h"

#include "CliCmd.h"



class ClientCom
{
public:
	~ClientCom();

	static ClientCom& GetInstance()
	{
		static ClientCom ins;
		return ins;
	}
private:
	ClientCom();
	ClientCom(const ClientCom &other);
	ClientCom &operator=(const ClientCom &other);

public:
	int start();
	void stop();


private:
	void handle_http_request(web::http::http_request message);
	void handle_http_options(web::http::http_request message);

private:
	web::http::experimental::listener::http_listener *m_listener;

	std::string m_url;


};


#endif // __CLIENT_COM_H__
